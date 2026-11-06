#pragma once

// NODE_ID is set per environment in platformio.ini (-DNODE_ID=...), so box_A and box_B
// are built from the same code. This fallback is only used if the flag is missing.
#ifndef NODE_ID
#define NODE_ID "box_A"
#endif

// MAC address of the gateway ESP32 (the gateway prints it on Serial at boot)
static const uint8_t GATEWAY_MAC[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// Must equal the Wi-Fi channel the gateway ends up on (the router/hotspot channel).
// The gateway prints its channel at boot. Fix the hotspot channel if possible.
#define ESPNOW_CHANNEL 1

#define SAMPLE_PERIOD_MS 30000     // normal reading interval
#define ACK_TIMEOUT_MS   150       // wait for gateway ACK per attempt
#define MAX_RETRIES      4         // attempts per message

// ---- OTA mode (E6): jumper GPIO32 -> GND at boot ----
#define OTA_PIN       32
#define OTA_WINDOW_MS (5UL * 60UL * 1000UL)
#ifndef NODE_HOST
#define NODE_HOST "lensguard-box-a"      // mDNS name, set per environment in platformio.ini
#endif
