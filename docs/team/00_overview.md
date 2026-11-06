# ภาพรวมการแบ่งงานทีม TuaKorm

อิงจากบทบาทใน proposal ที่ส่งแล้ว (ตรวจเทียบกับโค้ดในรีโปเมื่อ 5 พ.ย. 2026) โค้ดเฟิร์มแวร์ยังไม่เคยคอมไพล์/ทดสอบบนบอร์ด

## 1) ตรวจงานตามบทบาทใน proposal

| บทบาท | งานที่ proposal ระบุ | อยู่ที่ไหนในรีโป | สถานะเทียบกับโค้ด |
|---|---|---|---|
| Teerapat: Architect & Integration | ออกแบบระบบ รูปแบบข้อความ JSON รวมทุกส่วน | `docs/`, README | ร่างครบ ต้องยืนยันและ freeze |
| Wachiraphan: Sensor Node | เฟิร์มแวร์ node A/B, ISR ประตู, ESP-NOW + ACK/retry | `firmware/node/` | มีโค้ดร่าง + เพิ่มโหมด OTA ให้ตรง E6 "(และ node)" |
| Napat: Gateway & Cloud | เฟิร์มแวร์ gateway, FreeRTOS, คุมพัดลม, MQTT/TLS, OTA | `firmware/gateway/` | มีโค้ดร่างครบ ลบการยิง Telegram ออกจาก gateway |
| Natthadon: UI/UX & Dashboard | dashboard และแจ้งเตือน Telegram | `dashboard/` | เพิ่ม notifier.py (Telegram) ทดสอบแล้ว; dashboard ต้องตั้งบนเว็บ cloud |
| Methasit: PM & Test Lead | แผนงาน แผนทดสอบ เก็บ metric และสคริปต์ AI | `ai/`, `docs/test_plan.md` | สคริปต์ AI ทดสอบด้วยข้อมูลสังเคราะห์แล้ว; test plan เป็นแม่แบบ |

### สิ่งที่ปรับโค้ดให้ตรงกับ proposal
1. **Telegram:** block diagram ของ proposal ต่อ Telegram ออกจาก Cloud และงานเป็นของ Natthadon โค้ดเดิมให้ gateway ยิง Telegram เอง
   จึงย้ายเป็น gateway publish alert event (`lensguard/alerts`) แล้ว `dashboard/notifier.py` ส่ง Telegram
2. **OTA ที่ node:** proposal ข้อ E6 ระบุ "gateway (และ node)" แต่โค้ด node ยังไม่มี OTA จึงเพิ่มโหมด OTA ด้วยจัมเปอร์ GPIO32-GND
3. **Gateway offline:** เพิ่ม MQTT last will ให้ notifier แจ้งเมื่อ gateway หลุด (ครอบคลุม "เซ็นเซอร์ไม่ส่งข้อมูล" กรณี gateway เองหาย)
4. จำนวน task ของ gateway เหลือ 4 ตรงกับ proposal (อ่านเซ็นเซอร์ / รับ ESP-NOW / ส่ง cloud / ควบคุมพัดลม)

## 2) งานที่ proposal ยังไม่ได้กำหนดเจ้าของ (ทีมต้องตกลงกัน)

| งาน | กระทบคะแนน | ข้อเสนอเจ้าของ (ยังไม่ได้ตกลง) |
|---|---|---|
| ประกอบฮาร์ดแวร์ perfboard, เจาะและอุดกล่อง, จัดสาย | Build Quality 3, Robustness 3 | Teerapat + Wachiraphan |
| วงจรขับพัดลม MOSFET และความปลอดภัยไฟ | M2, M7 | Napat |
| ติดต่อ problem owner / site จริง และเก็บหลักฐานปัญหา | Evidence 2, Field Testing 3 | Methasit |
| ตัด checkpoint video (20 พ.ย.) และวิดีโอสุดท้าย | Video 4, milestone -2 | Natthadon |
| รวมเล่มรายงาน (<= 10 หน้า) และสไลด์ | Report 6, Slides 1 | Methasit (ทุกคนเขียนส่วนตัวเอง) |
| เลือก cloud (NETPIE / ThingSpeak / broker ทั่วไป) | M5, E5 | Teerapat ตัดสินใจ ร่วมกับ Napat และ Natthadon |

## 3) ภาระงานและความเสี่ยง
- **Napat** หนักที่สุด: gateway เป็นโค้ดชิ้นใหญ่ที่สุด รวมวงจรพัดลมและ cloud
- **Methasit** หนักรองลงมา: PM + แผนทดสอบ + AI + metric ถ้าไม่แบ่ง งานทดสอบจะกองท้ายโครงการ
- **Wachiraphan** โค้ด node เล็กกว่า จึงเหมาะรับงานประกอบฮาร์ดแวร์ ช่วยรันทดสอบ (PDR, failure) และ E3 สำรอง
- คะแนนรายบุคคลขึ้นกับ peer factor จากแบบประเมินเพื่อนและ Git commit history ให้ทุกคน commit ในโฟลเดอร์ของตัวเอง
  แต่ละ commit ขึ้นต้นด้วยแท็ก เช่น `[node]`, `[gateway]`, `[dashboard]`, `[ai]`, `[docs]`
- ช่วง Q&A สุ่มถามรายคนเรื่องส่วนของตัวเอง ใครอธิบายโค้ดตัวเองไม่ได้ อาจไม่ได้คะแนนส่วนนั้น
  (โค้ดในรีโปร่างด้วยเครื่องมือ AI ทุกคนต้องอ่านและเข้าใจก่อน)

## 4) เรื่องที่ต้องตัดสินใจก่อน (กำหนดภายใน 8 พ.ย.)
เลือก cloud: โค้ดตอนนี้ใช้หัวข้อ MQTT แบบทั่วไป `lensguard/...` ใช้ได้กับ broker ที่ตั้งหัวข้อเองได้ ส่วน NETPIE และ
ThingSpeak มีรูปแบบหัวข้อ การยืนยันตัวตน และข้อจำกัดข้อมูลของตัวเอง (ตรวจเอกสารล่าสุดของบริการนั้น)
ถ้าเลือกสองบริการนี้ ต้องปรับ gateway (Napat), ai/ (Methasit) และ notifier (Natthadon) ให้ตรง และ Teerapat ปรับ `docs/message_format.md`

## 5) ปฏิทิน (วันนี้ 5 พ.ย. 2026; เหลือ 15 วันถึง checkpoint, 33 วันถึงวันนำเสนอ)

| ช่วง | เป้าหมาย |
|---|---|
| 5-11 พ.ย. | ตั้งรีโป, เลือก cloud, ต่อ node/gateway บน breadboard ให้ ESP-NOW ทำงาน, freeze message_format (9 พ.ย.) |
| 12-19 พ.ย. | ข้อมูลไหล node -> gateway -> cloud -> dashboard, เริ่ม `collect.py` ไม่เกิน 14 พ.ย., ถ่าย checkpoint video 18 พ.ย. และส่งภายใน 19 พ.ย. (กำหนดจริง 20 พ.ย. 23:59) |
| 20-26 พ.ย. | พัดลม + threshold + notifier + dashboard, ย้ายลง perfboard, เจาะกล่อง |
| 27 พ.ย.-2 ธ.ค. | เทรน AI จากข้อมูลจริง, closed loop, E1/E2/E4/E5/E6 ครบ, เริ่มทดสอบ 24 ชม. ไม่เกิน 30 พ.ย. |
| 1-5 ธ.ค. | ติดตั้งที่ site จริง, วัด metric ซ้ำ, ทดสอบ failure ทั้งหมด |
| 3-7 ธ.ค. | รายงาน สไลด์ วิดีโอ ซ้อมนำเสนอและ demo (เตรียม hotspot และวิดีโอสำรอง) |
| 8 ธ.ค. | นำเสนอ ส่งทุกชิ้นก่อน 23:59 (ผ่อนผันถึง 06:00 วันที่ 9 ธ.ค.) และทุกคนส่งแบบประเมินเพื่อน |

วันที่ในตารางเป็นข้อเสนอของผู้ช่วยร่าง ปรับตามตารางเรียนของทีมได้ แต่ 20 พ.ย. และ 8 ธ.ค. เป็นกำหนดจริงของโจทย์
