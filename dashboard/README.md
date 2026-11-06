# dashboard/ (เจ้าของ: Natthadon)

## ส่วนที่เป็นโค้ด
`notifier.py` รับ alert event จาก gateway แล้วส่ง Telegram (ดูวิธีรันใน README หลัก)

## ตั้ง Telegram bot
1. คุยกับ @BotFather สร้างบอต ได้ token
2. ส่งข้อความหาบอตหนึ่งครั้ง แล้วเปิด `https://api.telegram.org/bot<TOKEN>/getUpdates` ดู `chat.id`
3. ใส่ `TELEGRAM_BOT_TOKEN` และ `TELEGRAM_CHAT_ID` ใน `dashboard/.env` (ไฟล์นี้ git-ignore) ทดสอบด้วย
   `NOTIFIER_DRY_RUN=1 python notifier.py` ก่อน

## ส่วนที่ตั้งบนเว็บของ cloud (ไม่มีไฟล์ในรีโป ให้ถ่ายภาพหน้าจอไว้ใส่รายงาน)
widget ที่ควรมีต่อกล่อง (box_A, box_B แยกกัน): กราฟ RH และอุณหภูมิ, สถานะประตู, สถานะพัดลม, RH ที่ AI ทำนาย
(`pred_rh` จากหัวข้อ `lensguard/cmd/fan/<node_id>`), packet delivery (`rx`, `lost`)
widget ของ gateway: `temp_ref`, `ldr`, `mq135` และสถานะ gateway online/offline
เกณฑ์ UX: คนที่ไม่เคยเห็นเปิดแล้วเข้าใจโดยไม่ต้องอธิบาย, สีบอกสถานะเสี่ยง (เช่น RH เกินเกณฑ์), แจ้งเตือนชัดเจน

## ข้อควรรู้เรื่อง cloud
NETPIE และ ThingSpeak ไม่ใช้หัวข้อ `lensguard/...` ตรง ๆ ต้องตรวจเอกสารล่าสุดของบริการที่เลือกเรื่องรูปแบบหัวข้อ
การยืนยันตัวตน ข้อจำกัดจำนวนข้อมูลและความถี่ แล้วตกลงกับ Napat และ Teerapat ก่อนว่าจะปรับส่วนไหน
