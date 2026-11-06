# Checklist (อ้างอิงโจทย์ Final Project 2026 และ proposal)

## Minimum requirements (ขาดข้อละ -2)
- [ ] M1 sensor >= 5 ชนิด: node (DHT22, reed) | gateway (DS18B20, LDR, MQ-135)
- [ ] M2 actuator: พัดลม DC ผ่าน MOSFET
- [ ] M3 node -> gateway (ESP-NOW + ACK/retry)
- [ ] M4 AI ปิด closed loop (ai/run_controller.py -> พัดลม)
- [ ] M5 raw data ขึ้น cloud + dashboard
- [ ] M6 แจ้งเตือน Telegram (dashboard/notifier.py)
- [ ] M7 safety: ไฟ DC ต่ำ, พัดลมมี timeout, ไม่มีของเหลว
- [ ] M8 Git repo สาธารณะ + commit ครบทุกคน

## Milestones (ขาด/ช้า -2)
- [x] 24 ต.ค. 2026 ลงทะเบียนกลุ่มและหัวข้อ (ยืนยันกับทีม)
- [x] 30 ต.ค. 2026 proposal (ส่งแล้ว)
- [ ] 20 พ.ย. 2026 23:59 checkpoint video (<= 2 นาที: node -> gateway -> cloud -> dashboard)
- [ ] 8 ธ.ค. 2026 23:59 นำเสนอ + ส่งงาน (ผ่อนผันถึง 06:00 วันที่ 9 ธ.ค.)

## Engineering Depth ที่เลือกใน proposal (5 ข้อ = 10 คะแนน, ต้อง demo + อธิบายได้)
- [ ] E1 FreeRTOS (gateway 4 tasks, node 3 tasks)  - [ ] E2 ISR reed switch  - [ ] E4 ACK/retry + buffer
- [ ] E5 MQTT TLS + ไม่ commit secret  - [ ] E6 OTA (gateway และ node)  (สำรอง: E3 deep sleep ของ node)

## ส่งงาน 8 ธ.ค.
- [ ] สไลด์  - [ ] รายงาน PDF <= 10 หน้า (6 หัวข้อ รวมบทบาท ลิงก์ Git และระบุเครื่องมือ AI)
- [ ] วิดีโอ 3-5 นาที (เปิดเผยสมาชิกและมหาวิทยาลัย)  - [ ] ซอร์สโค้ด zip + ลิงก์ Git  - [ ] แบบประเมินเพื่อน (รายบุคคล)
