# Teerapat Sardsud: System Architect & Integration

**Proposal ระบุ:** ออกแบบระบบ รูปแบบข้อความ JSON และรวมทุกส่วน

## ไฟล์ที่รับผิดชอบ
`README.md`, `docs/message_format.md`, `docs/pinout.md`, `docs/bom.md`, `docs/checklist.md`, `docs/team/*`, `.gitignore`
(commit ขึ้นต้น `[docs]`)

## งานที่ต้องทำ
- [ ] 5-6 พ.ย. สร้าง Git repo สาธารณะ เชิญสมาชิกทุกคน push โครงจาก zip ตั้งกติกา commit (แท็กตามโฟลเดอร์)
- [ ] 8 พ.ย. นัดทีมตัดสินใจเลือก cloud (ดู `00_overview.md` ข้อ 4) และบันทึกผลใน `message_format.md`
- [ ] 9 พ.ย. freeze `message_format.md` เวอร์ชัน 1 แจ้งทุกคน (ถ้าเปลี่ยนทีหลังต้องแจ้งทุกคนที่ใช้)
- [ ] 12-19 พ.ย. ทดสอบ end-to-end ด้วยกันกับ Natthadon/Napat: ข้อมูลของ box_A และ box_B ต้องแยกกันชัดบน dashboard
      (หมวด Scalability 3 คะแนนต้องมี node จริงสองตัวทำงานพร้อมกัน)
- [ ] ประกอบฮาร์ดแวร์ร่วมกับ Wachiraphan (ข้อเสนอ): perfboard, female header, เจาะและอุดรูสายพัดลม, จัดสาย
- [ ] วาด block diagram และ wiring diagram ฉบับจริงสำหรับรายงาน (ปรับให้ Telegram ต่อจาก Cloud ตรงกับโค้ด)
- [ ] เขียนรายงานหัวข้อ System design และหัวข้อบทบาท/ลิงก์ Git/การระบุเครื่องมือ AI

## ต้องอธิบายได้ตอน Q&A
สถาปัตยกรรมและเส้นทางข้อมูลทั้งหมด, ทำไมใช้ ESP-NOW ไม่ใช้สาย, ความหมายแต่ละฟิลด์ใน JSON (`node_id`, `seq`, `type`),
การรองรับหลาย node, กรณีล้มเหลวแต่ละจุด (Wi-Fi หลุด, AI ล่ม, gateway ล่ม) ระบบทำอะไร

## พึ่งพาคนอื่น / คนอื่นพึ่งคุณ
คุณส่ง message format ให้ทุกคน; ต้องการรายการสุดท้ายของอุปกรณ์จาก Napat (วงจรพัดลม) และ Wachiraphan (node)
