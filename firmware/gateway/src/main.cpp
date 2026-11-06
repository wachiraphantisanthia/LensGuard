// LensGuard gateway: receives nodes over ESP-NOW, reads its own sensors, controls the fans,
// uploads over MQTT (TLS) and publishes alert events (dashboard/notifier.py turns them into
// Telegram messages). NOT yet compiled or tested on hardware.
//
// Engineering depth: E1 (4 FreeRTOS tasks, queues, mutex), E4 (ACK to nodes, duplicate
// detection, store-and-forward ring buffer), E5 (MQTT over TLS, secrets kept out of Git),
// E6 (ArduinoOTA).
//
// Libraries are declared in platformio.ini (lib_deps).
// Copy include/secrets.example.h to include/secrets.h first.

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <time.h>
#include "secrets.h"
#include "settings.h"

// ---------- Types ----------
struct AckMsg { uint32_t seq; };
struct RxItem { uint8_t mac[6]; char data[200]; };
struct OutMsg { char topic[48]; char payload[256]; };

struct NodeState {
  const char *id;
  uint8_t fanPin;
  float rh, temp;
  int door;
  uint32_t lastSeq, rxCount, lost, duplicates;
  uint32_t lastSeenMs;
  bool online;
  bool fanOn;
  uint32_t fanStartMs, cooldownUntil;
  float rhAtFanStart;
  bool aiFan;
  uint32_t aiMs;
  uint32_t doorOpenSince;
  bool doorAlerted, gelAlerted;
  uint32_t lastAlertMs[4];   // 0 rh_high, 1 offline, 2 door, 3 gel
};

enum { AL_RH = 0, AL_OFFLINE = 1, AL_DOOR = 2, AL_GEL = 3 };

// ---------- Globals ----------
NodeState nodes[2];
SemaphoreHandle_t stateMux;
QueueHandle_t rxQueue, outQueue;

WiFiClientSecure tlsClient;
PubSubClient mqtt(tlsClient);
OneWire oneWire(PIN_DS18B20);
DallasTemperature ds18(&oneWire);

static OutMsg ring[RING_SIZE];
static int ringHead = 0, ringCount = 0;

// ---------- Helpers ----------
static NodeState *findNode(const char *id) {
  for (auto &n : nodes) if (strcmp(n.id, id) == 0) return &n;
  return nullptr;
}

static uint32_t nowEpoch() {
  time_t t = time(nullptr);
  return t > 1600000000 ? (uint32_t)t : 0;   // 0 until NTP has synced
}

static void postOut(const char *subTopic, const JsonDocument &doc) {
  OutMsg m;
  snprintf(m.topic, sizeof(m.topic), TOPIC_PREFIX "%s", subTopic);
  serializeJson(doc, m.payload, sizeof(m.payload));
  xQueueSend(outQueue, &m, 0);
}

static const char *ALERT_NAMES[4] = {"rh_high", "node_offline", "door_open", "gel_saturated"};

// Publishes a structured alert event. The Telegram text is built in dashboard/notifier.py.
// value: RH for rh_high/gel_saturated, seconds for node_offline/door_open.
static void sendAlert(NodeState *n, int type, float value) {
  uint32_t now = millis();
  if (n->lastAlertMs[type] != 0 && now - n->lastAlertMs[type] < ALERT_COOLDOWN_MS) return;
  n->lastAlertMs[type] = now ? now : 1;
  StaticJsonDocument<192> d;
  d["event"] = ALERT_NAMES[type];
  d["node_id"] = n->id;
  d["value"] = value;
  d["ts"] = nowEpoch();
  postOut("alerts", d);
}

// ---------- Store-and-forward ring buffer (E4) ----------
static void ringPush(const OutMsg &m) {
  if (ringCount == RING_SIZE) { ringHead = (ringHead + 1) % RING_SIZE; ringCount--; }  // drop oldest
  ring[(ringHead + ringCount) % RING_SIZE] = m;
  ringCount++;
}
static bool ringPeek(OutMsg &m) { if (!ringCount) return false; m = ring[ringHead]; return true; }
static void ringPop() { if (ringCount) { ringHead = (ringHead + 1) % RING_SIZE; ringCount--; } }

// ---------- ESP-NOW receive: copy and hand off to taskRx ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onEspNowRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  const uint8_t *mac = info->src_addr;
#else
void onEspNowRecv(const uint8_t *mac, const uint8_t *data, int len) {
#endif
  if (len <= 0 || len >= (int)sizeof(RxItem::data)) return;
  RxItem it;
  memcpy(it.mac, mac, 6);
  memcpy(it.data, data, len);
  it.data[len] = 0;
  xQueueSend(rxQueue, &it, 0);
}

// ---------- MQTT command from the AI script ----------
void onMqtt(char *topic, byte *payload, unsigned int len) {
  StaticJsonDocument<192> d;
  if (deserializeJson(d, payload, len)) return;
  String t(topic);
  for (auto &n : nodes) {
    if (t.endsWith(n.id)) {
      xSemaphoreTake(stateMux, portMAX_DELAY);
      n.aiFan = d["fan"] | false;
      n.aiMs = millis();
      xSemaphoreGive(stateMux);
    }
  }
}

// ---------- Task: own sensors (priority 1) ----------
void taskSensors(void *) {
  ds18.begin();
  for (;;) {
    ds18.requestTemperatures();
    float tRef = ds18.getTempCByIndex(0);
    int ldr = analogRead(PIN_LDR);
    int mq = analogRead(PIN_MQ135);
    StaticJsonDocument<256> d;
    d["node_id"] = "gateway";
    d["ts"] = nowEpoch();
    if (tRef > -100) d["temp_ref"] = tRef;     // DS18B20 returns -127 when unplugged
    d["ldr"] = ldr;
    d["mq135"] = mq;
    d["mq_ready"] = millis() > 120000;         // MQ-135 needs warm-up
    postOut("gateway/telemetry", d);
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PERIOD_MS));
  }
}

// ---------- Task: process node messages (priority 3) ----------
void taskRx(void *) {
  RxItem it;
  for (;;) {
    if (xQueueReceive(rxQueue, &it, portMAX_DELAY) != pdTRUE) continue;
    StaticJsonDocument<256> in;
    if (deserializeJson(in, it.data)) continue;
    const char *id = in["node_id"] | "";
    NodeState *n = findNode(id);
    if (!n) continue;
    uint32_t seq = in["seq"] | 0;

    // ACK first so the node can stop retrying (also for duplicates)
    if (!esp_now_is_peer_exist(it.mac)) {
      esp_now_peer_info_t p = {};
      memcpy(p.peer_addr, it.mac, 6);
      p.channel = 0;
      p.encrypt = false;
      esp_now_add_peer(&p);
    }
    AckMsg ack = {seq};
    esp_now_send(it.mac, (uint8_t *)&ack, sizeof(ack));

    xSemaphoreTake(stateMux, portMAX_DELAY);
    if (seq == n->lastSeq) { n->duplicates++; xSemaphoreGive(stateMux); continue; }
    if (seq > n->lastSeq && n->lastSeq != 0) n->lost += seq - n->lastSeq - 1;   // gap = lost packets
    n->lastSeq = seq;                        // seq < lastSeq means the node rebooted
    n->rxCount++;
    n->lastSeenMs = millis();
    if (!n->online) { n->online = true; }
    if (in.containsKey("rh"))   n->rh = in["rh"];
    if (in.containsKey("temp")) n->temp = in["temp"];
    int door = in["door"] | 0;
    if (door && !n->door) n->doorOpenSince = millis();
    if (!door) { n->doorOpenSince = 0; n->doorAlerted = false; }
    n->door = door;

    StaticJsonDocument<256> out;
    out["node_id"] = n->id;
    out["seq"] = seq;
    out["ts"] = nowEpoch();
    out["type"] = in["type"] | "periodic";
    out["temp"] = n->temp;
    out["rh"] = n->rh;
    out["door"] = n->door;
    out["fan"] = n->fanOn ? 1 : 0;
    out["rx"] = n->rxCount;
    out["lost"] = n->lost;
    xSemaphoreGive(stateMux);
    postOut((String(n->id) + "/telemetry").c_str(), out);
  }
}

// ---------- Task: fan control + alert rules (priority 4, safety) ----------
static bool desiredFan(NodeState &n, uint32_t now) {
  if (!n.online || isnan(n.rh)) return false;               // stale data -> do not run blindly
  if (n.rh >= RH_HARD) return true;                         // failsafe overrides AI
  if (n.rh <= RH_OFF) return false;
  bool aiFresh = n.aiMs != 0 && (now - n.aiMs) < AI_CMD_TTL_MS;
  if (aiFresh) return n.aiFan;
  return n.fanOn ? (n.rh > RH_OFF) : (n.rh >= RH_ON);        // fallback hysteresis
}

void taskFan(void *) {
  for (;;) {
    uint32_t now = millis();
    for (auto &n : nodes) {
      xSemaphoreTake(stateMux, portMAX_DELAY);
      if (n.online && now - n.lastSeenMs > NODE_TIMEOUT_MS) {
        n.online = false;
        sendAlert(&n, AL_OFFLINE, (now - n.lastSeenMs) / 1000.0f);
      }
      bool want = desiredFan(n, now);
      if (want && !n.fanOn && now >= n.cooldownUntil) {
        n.fanOn = true; n.fanStartMs = now; n.rhAtFanStart = n.rh; n.gelAlerted = false;
        digitalWrite(n.fanPin, HIGH);
      } else if (n.fanOn && (!want || now - n.fanStartMs >= FAN_MAX_RUN_MS)) {
        if (want) n.cooldownUntil = now + FAN_COOLDOWN_MS;   // forced stop by timeout
        n.fanOn = false;
        digitalWrite(n.fanPin, LOW);
      }
      if (n.fanOn && !n.gelAlerted && now - n.fanStartMs >= GEL_CHECK_MS &&
          (n.rhAtFanStart - n.rh) < GEL_MIN_DROP) {
        n.gelAlerted = true;
        sendAlert(&n, AL_GEL, n.rh);
      }
      if (!isnan(n.rh) && n.rh >= RH_ALERT)
        sendAlert(&n, AL_RH, n.rh);
      if (n.door && n.doorOpenSince && !n.doorAlerted && now - n.doorOpenSince >= DOOR_OPEN_ALERT_MS) {
        n.doorAlerted = true;
        sendAlert(&n, AL_DOOR, (now - n.doorOpenSince) / 1000.0f);
      }
      xSemaphoreGive(stateMux);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// ---------- Task: Wi-Fi + MQTT + OTA + buffered publish (priority 2) ----------
void taskCloud(void *) {
  uint32_t lastWifi = 0, lastMqtt = 0;
  bool ntpStarted = false, otaStarted = false;
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqtt);
  mqtt.setBufferSize(512);
  tlsClient.setCACert(MQTT_CA_CERT);

  for (;;) {
    uint32_t now = millis();
    if (WiFi.status() != WL_CONNECTED) {
      if (now - lastWifi > 8000 || lastWifi == 0) {
        lastWifi = now ? now : 1;
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      }
    } else {
      if (!ntpStarted) { configTime(7 * 3600, 0, "pool.ntp.org"); ntpStarted = true; }
      if (!otaStarted) {
        ArduinoOTA.setHostname("lensguard-gateway");
        ArduinoOTA.setPassword(OTA_PASSWORD);
        ArduinoOTA.begin();
        otaStarted = true;
        Serial.printf("Wi-Fi channel: %d  (set ESPNOW_CHANNEL on the nodes to this)\n", WiFi.channel());
      }
      ArduinoOTA.handle();
      if (!mqtt.connected() && (now - lastMqtt > 4000 || lastMqtt == 0)) {
        lastMqtt = now ? now : 1;
        // Last Will: the broker publishes "offline" if the gateway disappears
        if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD,
                         TOPIC_PREFIX "gateway/status", 0, true, "offline")) {
          mqtt.subscribe(TOPIC_PREFIX "cmd/fan/#");
          mqtt.publish(TOPIC_PREFIX "gateway/status", "online", true);
        }
      }
      mqtt.loop();
    }

    OutMsg m;
    while (xQueueReceive(outQueue, &m, 0) == pdTRUE) ringPush(m);   // everything is buffered first
    if (mqtt.connected()) {
      OutMsg p;
      int budget = 10;                                              // do not starve mqtt.loop()
      while (budget-- > 0 && ringPeek(p)) {
        if (mqtt.publish(p.topic, p.payload)) ringPop(); else break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  Serial.begin(115200);
  nodes[0] = NodeState(); nodes[0].id = "box_A"; nodes[0].fanPin = PIN_FAN_A;
  nodes[1] = NodeState(); nodes[1].id = "box_B"; nodes[1].fanPin = PIN_FAN_B;
  for (auto &n : nodes) {
    n.rh = NAN; n.temp = NAN; n.rhAtFanStart = NAN;
    pinMode(n.fanPin, OUTPUT);
    digitalWrite(n.fanPin, LOW);             // fans off at boot
  }

  stateMux = xSemaphoreCreateMutex();
  rxQueue = xQueueCreate(16, sizeof(RxItem));
  outQueue = xQueueCreate(24, sizeof(OutMsg));

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);                      // modem sleep can drop ESP-NOW packets
  Serial.print("Gateway MAC (put this in the nodes' config.h): ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); ESP.restart(); }
  esp_now_register_recv_cb(onEspNowRecv);

  xTaskCreate(taskFan,     "fan",     4096, nullptr, 4, nullptr);
  xTaskCreate(taskRx,      "rx",      6144, nullptr, 3, nullptr);
  xTaskCreate(taskCloud,   "cloud",   10240, nullptr, 2, nullptr);
  xTaskCreate(taskSensors, "sensors", 4096, nullptr, 1, nullptr);
}

void loop() { vTaskDelay(portMAX_DELAY); }
