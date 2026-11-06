#pragma once
// Copy this file to secrets.h and fill in your values. secrets.h is git-ignored.
// NEVER commit real Wi-Fi passwords, MQTT credentials or tokens.

#define WIFI_SSID        "your-hotspot"
#define WIFI_PASSWORD    "your-password"

#define MQTT_HOST        "mqtt.example.com"
#define MQTT_PORT        8883                 // TLS port
#define MQTT_CLIENT_ID   "lensguard-gateway"
#define MQTT_USER        "user-or-token"
#define MQTT_PASSWORD    "password-or-secret"

// Root CA certificate of your MQTT broker (PEM). Needed for verified TLS (E5).
static const char MQTT_CA_CERT[] = R"EOF(
-----BEGIN CERTIFICATE-----
PASTE-THE-BROKER-ROOT-CA-HERE
-----END CERTIFICATE-----
)EOF";

#define OTA_PASSWORD     "change-me"
