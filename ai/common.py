"""Shared helpers: .env loader and MQTT client factory."""
import os
import ssl
from pathlib import Path

import paho.mqtt.client as mqtt

ROOT = Path(__file__).resolve().parent.parent
TOPIC_PREFIX = "lensguard/"
NODES = ["box_A", "box_B"]


def load_env(path=None):
    path = Path(path) if path else Path(__file__).resolve().parent / ".env"
    if path.exists():
        for line in path.read_text().splitlines():
            line = line.strip()
            if line and not line.startswith("#") and "=" in line:
                k, v = line.split("=", 1)
                os.environ.setdefault(k.strip(), v.strip())


def make_client(client_id):
    load_env()
    try:  # paho-mqtt 2.x
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, client_id=client_id)
    except AttributeError:  # paho-mqtt 1.x
        client = mqtt.Client(client_id=client_id)
    user = os.environ.get("MQTT_USER")
    if user:
        client.username_pw_set(user, os.environ.get("MQTT_PASSWORD", ""))
    if os.environ.get("MQTT_TLS", "1") == "1":
        ca = os.environ.get("MQTT_CA_FILE") or None
        client.tls_set(ca_certs=ca, cert_reqs=ssl.CERT_REQUIRED)
    return client


def connect(client):
    client.connect(os.environ["MQTT_HOST"], int(os.environ.get("MQTT_PORT", "8883")), keepalive=60)
