"""Telegram notifier (owner: UI/UX & Dashboard developer).

Subscribes to the alert events and the status message published by the gateway and turns them
into Telegram messages. Run it on an always-on machine (the same one as ai/run_controller.py
is fine). If it is not running, alerts are not delivered, but data and fan control still work.

Topics (see docs/message_format.md):
  lensguard/alerts          {"event": "...", "node_id": "box_A", "value": 61.2, "ts": ...}
  lensguard/gateway/status  "online" / "offline" (retained, offline comes from the MQTT last will)
"""
import json
import os
import ssl
import urllib.request
from pathlib import Path

import paho.mqtt.client as mqtt

TOPIC_PREFIX = "lensguard/"
NODE_NAMES = {"box_A": "กล่อง A", "box_B": "กล่อง B"}
TEMPLATES = {
    "rh_high": "{node}: ความชื้นสูง {value:.1f}% ตรวจสอบกล่องและซิลิกาเจล",
    "node_offline": "{node}: ไม่ส่งข้อมูลมา {value:.0f} วินาที ตรวจสอบบอร์ดและไฟเลี้ยง",
    "door_open": "{node}: ประตูเปิดค้างมา {value:.0f} วินาที",
    "gel_saturated": "{node}: เปิดพัดลมแล้วความชื้นไม่ลง (ตอนนี้ {value:.1f}%) "
                     "ซิลิกาเจลน่าจะอิ่มตัว ควรเปลี่ยนหรืออบซ้ำ",
}
STATUS_TEXT = {
    "offline": "gateway หลุดการเชื่อมต่อ ข้อมูลจะไม่เข้า cloud จนกว่าจะกลับมา",
    "online": "gateway กลับมาออนไลน์แล้ว",
}


def load_env():
    path = Path(__file__).resolve().parent / ".env"
    if path.exists():
        for line in path.read_text().splitlines():
            line = line.strip()
            if line and not line.startswith("#") and "=" in line:
                k, v = line.split("=", 1)
                os.environ.setdefault(k.strip(), v.strip())


def send_telegram(text):
    token = os.environ.get("TELEGRAM_BOT_TOKEN", "")
    chat = os.environ.get("TELEGRAM_CHAT_ID", "")
    if os.environ.get("NOTIFIER_DRY_RUN") == "1" or not token or not chat:
        print("[dry-run] " + text)
        return
    req = urllib.request.Request(
        f"https://api.telegram.org/bot{token}/sendMessage",
        data=json.dumps({"chat_id": chat, "text": text}).encode(),
        headers={"Content-Type": "application/json"},
    )
    try:
        urllib.request.urlopen(req, timeout=10).read()   # certificate is verified by default
    except Exception as e:                               # keep running if Telegram is unreachable
        print("telegram send failed:", e)


class Notifier:
    def __init__(self, send=send_telegram):
        self.send = send
        self.last_status = None

    def handle(self, topic, payload):
        if topic == TOPIC_PREFIX + "alerts":
            try:
                d = json.loads(payload)
                text = TEMPLATES[d["event"]].format(
                    node=NODE_NAMES.get(d.get("node_id"), d.get("node_id", "?")),
                    value=float(d.get("value", 0)))
            except (ValueError, KeyError, TypeError):
                return
            self.send(text)
        elif topic == TOPIC_PREFIX + "gateway/status":
            status = payload.decode().strip() if isinstance(payload, bytes) else str(payload).strip()
            if status not in STATUS_TEXT:
                return
            if self.last_status is not None and status != self.last_status:
                self.send(STATUS_TEXT[status])
            self.last_status = status        # the first (retained) value is recorded, not announced


def main():
    load_env()
    notifier = Notifier()
    try:  # paho-mqtt 2.x
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id="lensguard-notifier")
    except AttributeError:  # paho-mqtt 1.x
        client = mqtt.Client(client_id="lensguard-notifier")
    if os.environ.get("MQTT_USER"):
        client.username_pw_set(os.environ["MQTT_USER"], os.environ.get("MQTT_PASSWORD", ""))
    if os.environ.get("MQTT_TLS", "1") == "1":
        client.tls_set(ca_certs=os.environ.get("MQTT_CA_FILE") or None, cert_reqs=ssl.CERT_REQUIRED)

    def on_connect(c, *args):
        c.subscribe([(TOPIC_PREFIX + "alerts", 0), (TOPIC_PREFIX + "gateway/status", 0)])
        print("notifier subscribed")

    client.on_connect = on_connect
    client.on_message = lambda c, u, m: notifier.handle(m.topic, m.payload)
    client.connect(os.environ["MQTT_HOST"], int(os.environ.get("MQTT_PORT", "8883")), keepalive=60)
    client.loop_forever()


if __name__ == "__main__":
    main()
