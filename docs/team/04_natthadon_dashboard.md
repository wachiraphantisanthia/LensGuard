# Natthadon Chairuangsirikul: UI/UX & Dashboard Developer

**Proposal ระบุ:** dashboard และการแจ้งเตือน Telegram; ข้อ M5 (ส่วน dashboard), M6

## ไฟล์ที่รับผิดชอบ
`dashboard/**` (commit ขึ้นต้น `[dashboard]`) และ dashboard บนเว็บของ cloud ที่เลือก

## งานที่ต้องทำ
- [ ] 5-8 พ.ย. สร้าง Telegram bot (ขั้นตอนใน `dashboard/README.md`) เอา token และ chat id ใส่ `dashboard/.env`
      ทดสอบ `NOTIFIER_DRY_RUN=1 python notifier.py` แล้วทดสอบส่งจริง
- [ ] ร่วมเลือก cloud (8 พ.ย.) และศึกษารูปแบบข้อมูลของบริการนั้น (ตรวจเอกสารล่าสุด)
- [ ] 12-19 พ.ย. ตั้ง dashboard ให้เห็นข้อมูลของ box_A และ box_B แยกกัน (widget ตามรายการใน `dashboard/README.md`)
      เพื่อใช้ใน checkpoint video
- [ ] ทดสอบ alert ทั้ง 4 ชนิด + gateway offline/online กับ gateway จริง ปรับข้อความใน `TEMPLATES` ให้ชัดและมีประโยชน์
- [ ] ให้คนนอกทีมลองใช้ dashboard โดยไม่อธิบาย แล้วบันทึกข้อสังเกต (เกณฑ์ UX 3 คะแนน: intuitive, ใช้โดยไม่ต้องสอน)
- [ ] ถ่ายภาพหน้าจอ dashboard และ Telegram ไว้ใส่รายงาน
- [ ] ตัด checkpoint video (20 พ.ย.) และวิดีโอสุดท้าย 3-5 นาที (ข้อเสนอ ยังไม่ได้ตกลงกับทีม)

## ต้องอธิบายได้ตอน Q&A
- ลำดับ alert: gateway ตรวจเงื่อนไข -> publish event -> notifier -> Telegram; cooldown 30 นาทีต่อชนิดต่อกล่องอยู่ที่ gateway
- ทำไมแยก notifier ออกจาก gateway และข้อจำกัด (ต้องรันบนเครื่องที่เปิดตลอด ถ้าปิด Telegram ไม่ส่ง)
- last will ทำให้รู้ว่า gateway หลุดได้อย่างไร
- dashboard แยก node ด้วย `node_id` อย่างไร และแต่ละ widget มาจากฟิลด์ไหน

## ทดสอบ notifier โดยไม่มี gateway
ใช้ MQTT client ใดก็ได้ publish ไปที่ `lensguard/alerts` ตัวอย่างข้อความ:
`{"event":"rh_high","node_id":"box_A","value":66.4}` (event อื่น: `node_offline`, `door_open`, `gel_saturated`)
