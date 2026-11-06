#pragma once

// ---------- Pins (see docs/pinout.md) ----------
#define PIN_DS18B20 16
#define PIN_LDR     34   // ADC1 (ADC2 cannot be used while Wi-Fi is on)
#define PIN_MQ135   35   // ADC1, behind a 10k/15k divider
#define PIN_FAN_A   26   // MOSFET gate, box_A
#define PIN_FAN_B   27   // MOSFET gate, box_B

// ---------- Humidity control (tune with your own data) ----------
#define RH_ON            55.0f    // fallback: start fan at or above this RH
#define RH_OFF           45.0f    // stop fan at or below this RH
#define RH_HARD          70.0f    // always run fan (failsafe even if AI says no)
#define RH_ALERT         65.0f    // Telegram alert threshold
#define AI_CMD_TTL_MS    150000UL // AI command older than this is ignored (fallback rule used)

// ---------- Safety (rule M7: moving parts need a safe stop) ----------
#define FAN_MAX_RUN_MS   (30UL * 60UL * 1000UL)  // forced stop after 30 min
#define FAN_COOLDOWN_MS  (5UL * 60UL * 1000UL)   // wait before restarting after a forced stop

// ---------- Alerts ----------
#define NODE_TIMEOUT_MS      (3UL * 60UL * 1000UL)   // no data for 3 min -> node offline
#define DOOR_OPEN_ALERT_MS   (2UL * 60UL * 1000UL)   // door open for 2 min
#define GEL_CHECK_MS         (20UL * 60UL * 1000UL)  // fan on this long ...
#define GEL_MIN_DROP         2.0f                    // ... but RH fell less than this -> gel saturated?
#define ALERT_COOLDOWN_MS    (30UL * 60UL * 1000UL)

// ---------- Cloud ----------
#define SENSOR_PERIOD_MS 10000
#define RING_SIZE        60        // store-and-forward buffer (RAM only; lost on power loss)
#define TOPIC_PREFIX     "lensguard/"
