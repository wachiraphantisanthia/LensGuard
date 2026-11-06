# LensGuard: ตู้กันชื้นอัจฉริยะพร้อม AI ทำนายความชื้น

**กลุ่ม TuaKorm** | Embedded System (CEDT) Final Project 2026: Smart Devices
เอกสารต้นทาง: proposal ที่ส่งแล้ว (ไม่ได้ใส่ไฟล์ไว้ในรีโป เพราะมีรหัสนักศึกษา ถ้าทีมไม่ขัดข้องจะเพิ่มเองก็ได้)

ระบบตู้กันชื้นสำหรับกล้องและเลนส์ ใช้ ESP32 3 ตัว (sensor node ×2, gateway ×1), AI ทำนาย RH ล่วงหน้า 15 นาที,
พัดลมหมุนเวียนอากาศในกล่องผ่านซิลิกาเจล และแจ้งเตือนผ่าน Telegram

> **สถานะ:** โครงเริ่มต้น ส่วน Python (`ai/`, `dashboard/`) ทดสอบแล้วด้วยข้อมูลสังเคราะห์ แต่
> **เฟิร์มแวร์ ESP32 ยังไม่เคยถูกคอมไพล์และไม่เคยทดสอบบนบอร์ดจริง** ถือเป็นร่างที่ต้องแก้และทดสอบเอง
> ค่า threshold, ขา GPIO และราคาเป็นค่าประมาณ

## ทีมและโฟลเดอร์ที่รับผิดชอบ

| บทบาท (ตาม proposal) | สมาชิก | โฟลเดอร์/ไฟล์ | เอกสารงานรายคน |
|---|---|---|---|
| System Architect & Integration | Teerapat Sardsud | `docs/` (ยกเว้น test_plan), README, รูปแบบข้อความ, การรวมระบบ | `docs/team/01_teerapat_architect.md` |
| Embedded Developer (Sensor Node) | Wachiraphan Tisanthia | `firmware/node/` | `docs/team/02_wachiraphan_node.md` |
| Embedded Developer (Gateway & Cloud) | Napat Juengthanachote | `firmware/gateway/` | `docs/team/03_napat_gateway_cloud.md` |
| UI/UX & Dashboard Developer | Natthadon Chairuangsirikul | `dashboard/` | `docs/team/04_natthadon_dashboard.md` |
| Project Manager & Test Lead | Methasit Phanawongwat | `ai/`, `docs/test_plan.md`, `tools/` | `docs/team/05_methasit_pm_test_ai.md` |

ภาพรวมการแบ่งงาน ช่องว่างที่ proposal ยังไม่ได้กำหนดเจ้าของ และปฏิทิน: `docs/team/00_overview.md`
(เลขประจำตัวนักศึกษาไม่ได้ใส่ในรีโป เพราะรีโปต้องเป็นสาธารณะตามข้อ M8)

## โครงสร้าง

```
firmware/node/       PlatformIO: sensor node (DHT22 + reed switch -> ESP-NOW), env box_a / box_b (+ _ota)
firmware/gateway/    PlatformIO: gateway (รับจาก node, คุมพัดลม, MQTT/TLS, OTA, ส่ง alert event)
ai/                  เก็บข้อมูล, เทรนโมเดล, controller ที่สั่งพัดลมผ่าน MQTT
dashboard/           notifier.py (alert event -> Telegram) + วิธีตั้ง dashboard
tools/               simulate_data.py (ข้อมูลสังเคราะห์สำหรับทดสอบ pipeline เท่านั้น)
docs/                pinout, รูปแบบข้อความ, BOM, test plan, checklist, proposal, เอกสารทีม
```

## ลำดับข้อมูล (ตรงกับ block diagram ใน proposal)

node A/B --ESP-NOW (JSON + ACK/retry)--> gateway --MQTT/TLS--> cloud --> AI script (ai/run_controller.py)
 และ --> dashboard และ --> notifier (dashboard/notifier.py) --> Telegram
AI ส่งคำสั่งพัดลมกลับทาง cloud ไปที่ gateway ซึ่งยังมีกฎสำรองและข้อจำกัดความปลอดภัยของตัวเองเสมอ

## เริ่มต้นใช้งาน

### 1) เฟิร์มแวร์ (PlatformIO)
1. gateway: คัดลอก `firmware/gateway/include/secrets.example.h` เป็น `secrets.h` แล้วกรอก (ไฟล์นี้ถูก git-ignore)
2. flash gateway แล้วเปิด monitor จด **MAC address** และ **Wi-Fi channel**
   ```
   pio run -d firmware/gateway -t upload
   pio device monitor -d firmware/gateway
   ```
3. node: แก้ `firmware/node/include/config.h` ใส่ MAC ของ gateway และ `ESPNOW_CHANNEL` ให้ตรงกับ gateway แล้ว flash
   ```
   pio run -d firmware/node -e box_a -t upload
   pio run -d firmware/node -e box_b -t upload
   ```
4. OTA (E6): gateway `pio run -d firmware/gateway -e gateway_ota -t upload` ส่วน node ต้องต่อจัมเปอร์ GPIO32-GND
   แล้วรีเซ็ตก่อน (ดู `docs/pinout.md`) จากนั้น `pio run -d firmware/node -e box_a_ota -t upload`
   ตั้งตัวแปร `OTA_PASSWORD` ใน shell ให้ตรงกับ `secrets.h`

### 2) Python
```
cd ai && pip install -r requirements.txt && cp env.example .env     # กรอก MQTT
python collect.py          # เก็บข้อมูลอย่างน้อย 2-3 วัน
python train.py            # พิมพ์ MAE และบันทึกโมเดล
python run_controller.py   # ทำนายสดและสั่งพัดลม

cd ../dashboard && pip install -r requirements.txt && cp env.example .env   # กรอก MQTT + Telegram
python notifier.py         # ส่ง Telegram จาก alert event
```
ทดสอบโดยไม่มีฮาร์ดแวร์: `python tools/simulate_data.py data/synthetic_telemetry.csv` แล้ว
`python ai/train.py data/synthetic_telemetry.csv` (ข้อมูลปลอม ห้ามใช้ในรายงาน)

## การตัดสินใจออกแบบที่ต้องรู้

- gateway คุมพัดลมผ่านสาย (MOSFET ที่ GPIO26/27) ตามข้อ M2 ใน proposal
- AI ส่งคำสั่งทาง MQTT; gateway ใช้เมื่อคำสั่งยังใหม่ (ไม่เกิน 150 วินาที) ถ้าไม่ได้รับจะถอยไปใช้กฎ threshold
  (เปิดที่ RH >= 55%, ปิดที่ RH <= 45%) ตรงกับ failsafe ใน proposal และ gateway บังคับ safety เสมอ:
  หยุดพัดลมครบ 30 นาที, RH >= 70% เปิดพัดลมเสมอ, ข้อมูล node ค้างเก่าจะไม่เปิดพัดลม
- gateway ไม่ส่ง Telegram เอง แต่ publish alert event ที่ `lensguard/alerts` แล้ว `dashboard/notifier.py` ส่งต่อ
  ตาม block diagram ที่ Telegram ต่อออกจาก Cloud
- ตำแหน่งวางเซ็นเซอร์ของ gateway (DS18B20, LDR, MQ-135) **ยังไม่ได้ตัดสินใจ**

## เรื่องที่ทีมต้องตัดสินใจ (ยังเปิดอยู่)

1. **เลือก cloud:** proposal เขียน "NETPIE / ThingSpeak" แต่โค้ดตอนนี้ใช้หัวข้อ MQTT แบบทั่วไป (`lensguard/...`)
   ซึ่งใช้ได้กับ broker ทั่วไปที่ตั้งหัวข้อเองได้ ส่วน NETPIE และ ThingSpeak กำหนดรูปแบบหัวข้อและข้อมูลของตัวเอง
   ต้องปรับ gateway, ai และ notifier ให้ตรง ดูรายละเอียดใน `docs/team/00_overview.md`
2. ใครดูแลงานที่ proposal ไม่ได้กำหนด (ประกอบฮาร์ดแวร์/กล่อง, ติดต่อ site จริง, ตัดวิดีโอ, รายงาน)

## ข้อจำกัดที่รู้อยู่แล้ว

- buffer ตอนเน็ตหลุดอยู่ใน RAM (60 ข้อความ) ไฟดับแล้วหาย
- ESP-NOW ไม่เข้ารหัส (`encrypt = false`) และ node ไม่เก็บข้อความที่ส่งไม่สำเร็จเกิน 4 ครั้ง
- `platformio.ini` ปักแพลตฟอร์ม `espressif32@^6.9.0` (Arduino-ESP32 core 2.x) โค้ดมี `#if` สำหรับ core 3.x แต่ยังไม่ได้ทดสอบ
- notifier และ AI controller ต้องรันบนเครื่องที่เปิดตลอด ถ้าไม่ทำงาน Telegram จะไม่ส่ง และพัดลมจะใช้กฎสำรอง

## เครื่องมือ AI ที่ใช้ (ต้องระบุในรายงานตามข้อ 4.6)

[ใส่ชื่อเครื่องมือและส่วนที่ใช้ เช่น Claude ช่วยร่างโครงโค้ดเฟิร์มแวร์ สคริปต์ AI และเอกสาร]
ทุกคนต้องอธิบายโค้ดในส่วนของตัวเองได้ในช่วง Q&A
