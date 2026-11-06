// LensGuard sensor node: DHT22 + reed switch -> ESP-NOW -> gateway
// Engineering depth: E1 (FreeRTOS tasks + queue + event group), E2 (ISR + deferred work),
// E4 (ACK/retry), E6 (OTA mode, see runOtaMode). NOT yet compiled or tested on hardware.
//
// Libraries are declared in platformio.ini (lib_deps). Build: pio run -e box_a / -e box_b

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "config.h"
#if __has_include("secrets.h")
#include "secrets.h"          // WIFI_SSID, WIFI_PASSWORD, OTA_PASSWORD (git-ignored)
#define HAVE_OTA_SECRETS 1
#endif

#define DHT_PIN   4
#define REED_PIN  33   // reed closed (door shut, magnet near) = LOW with INPUT_PULLUP

struct AckMsg { uint32_t seq; };               // gateway -> node
struct TxItem { char json[192]; uint32_t seq; };

DHT dht(DHT_PIN, DHT22);
QueueHandle_t doorIsrQueue;    // ISR -> taskDoor (deferred processing)
QueueHandle_t txQueue;         // sensor/door tasks -> taskTx
EventGroupHandle_t ackEvents;
const EventBits_t ACK_BIT = BIT0;

volatile uint32_t lastAckSeq = 0;
volatile float lastTemp = NAN, lastRh = NAN;
static uint32_t seqCounter = 0;

// ---------- E2: the ISR only signals; real work happens in taskDoor ----------
void IRAM_ATTR doorIsr() {
  BaseType_t woken = pdFALSE;
  uint8_t tick = 1;
  xQueueSendFromISR(doorIsrQueue, &tick, &woken);
  if (woken) portYIELD_FROM_ISR();
}

// ---------- ESP-NOW receive callback (ACKs from the gateway) ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onRecv(const esp_now_recv_info_t *, const uint8_t *data, int len)
#else
void onRecv(const uint8_t *, const uint8_t *data, int len)
#endif
{
  if (len == (int)sizeof(AckMsg)) {
    AckMsg a;
    memcpy(&a, data, sizeof(a));
    lastAckSeq = a.seq;
    xEventGroupSetBits(ackEvents, ACK_BIT);
  }
}

static void enqueueReading(float t, float h, int door, const char *type) {
  TxItem it;
  it.seq = ++seqCounter;
  StaticJsonDocument<192> doc;
  doc["node_id"] = NODE_ID;
  doc["seq"] = it.seq;
  doc["up"] = (uint32_t)(millis() / 1000);   // uptime; the gateway adds the real timestamp
  doc["type"] = type;                          // "periodic" or "door"
  if (!isnan(t)) doc["temp"] = t;
  if (!isnan(h)) doc["rh"] = h;
  doc["door"] = door;
  serializeJson(doc, it.json, sizeof(it.json));
  xQueueSend(txQueue, &it, 0);
}

static int doorOpen() { return digitalRead(REED_PIN) == HIGH ? 1 : 0; }

// ---------- Task 1 (priority 1): periodic sensor reading ----------
void taskSensor(void *) {
  for (;;) {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) { lastRh = h; lastTemp = t; }
    enqueueReading(t, h, doorOpen(), "periodic");   // NaN fields are omitted from the JSON
    vTaskDelay(pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
  }
}

// ---------- Task 2 (priority 3): door events; highest because it is event-driven and short ----------
void taskDoor(void *) {
  uint8_t tick;
  for (;;) {
    if (xQueueReceive(doorIsrQueue, &tick, portMAX_DELAY) == pdTRUE) {
      vTaskDelay(pdMS_TO_TICKS(50));                 // debounce
      while (xQueueReceive(doorIsrQueue, &tick, 0) == pdTRUE) {}   // drop bounce edges
      enqueueReading(lastTemp, lastRh, doorOpen(), "door");
    }
  }
}

// ---------- Task 3 (priority 2): transmit with ACK + retry (E4) ----------
void taskTx(void *) {
  TxItem it;
  for (;;) {
    if (xQueueReceive(txQueue, &it, portMAX_DELAY) != pdTRUE) continue;
    bool acked = false;
    for (int attempt = 0; attempt < MAX_RETRIES && !acked; attempt++) {
      xEventGroupClearBits(ackEvents, ACK_BIT);
      esp_now_send(GATEWAY_MAC, (const uint8_t *)it.json, strlen(it.json) + 1);
      TickType_t deadline = xTaskGetTickCount() + pdMS_TO_TICKS(ACK_TIMEOUT_MS);
      while (!acked && xTaskGetTickCount() < deadline) {
        xEventGroupWaitBits(ackEvents, ACK_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(20));
        if (lastAckSeq == it.seq) acked = true;
      }
    }
    if (!acked) Serial.printf("[tx] seq %u not acknowledged\n", (unsigned)it.seq);
  }
}

// ---------- E6: OTA mode ----------
// Put a jumper between OTA_PIN (GPIO32) and GND, then reset. The node joins Wi-Fi and waits for
// an OTA upload for OTA_WINDOW_MS, then restarts. Remove the jumper for normal operation.
static void runOtaMode() {
#ifdef HAVE_OTA_SECRETS
  Serial.println("OTA mode: connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 30000) delay(250);
  if (WiFi.status() != WL_CONNECTED) { Serial.println("Wi-Fi failed, restarting"); delay(500); ESP.restart(); }
  ArduinoOTA.setHostname(NODE_HOST);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();
  Serial.printf("OTA ready: %s.local\n", NODE_HOST);
  t0 = millis();
  while (millis() - t0 < OTA_WINDOW_MS) { ArduinoOTA.handle(); delay(10); }
  ESP.restart();
#else
  Serial.println("OTA mode requested but include/secrets.h is missing; continuing normally");
#endif
}

void setup() {
  Serial.begin(115200);
  pinMode(OTA_PIN, INPUT_PULLUP);
  delay(50);
  if (digitalRead(OTA_PIN) == LOW) runOtaMode();
  pinMode(REED_PIN, INPUT_PULLUP);
  dht.begin();

  doorIsrQueue = xQueueCreate(8, sizeof(uint8_t));
  txQueue = xQueueCreate(16, sizeof(TxItem));
  ackEvents = xEventGroupCreate();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.print("Node MAC: "); Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) { Serial.println("ESP-NOW init failed"); ESP.restart(); }
  esp_now_register_recv_cb(onRecv);
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, GATEWAY_MAC, 6);
  peer.channel = ESPNOW_CHANNEL;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) Serial.println("add peer failed");

  attachInterrupt(digitalPinToInterrupt(REED_PIN), doorIsr, CHANGE);

  xTaskCreate(taskSensor, "sensor", 4096, nullptr, 1, nullptr);
  xTaskCreate(taskTx,     "tx",     4096, nullptr, 2, nullptr);
  xTaskCreate(taskDoor,   "door",   4096, nullptr, 3, nullptr);
}

void loop() { vTaskDelay(portMAX_DELAY); }
