# Wachiraphan Tisanthia: Embedded Developer (Sensor Node)

**Proposal ระบุ:** เฟิร์มแวร์ node A/B, ISR ประตู, ESP-NOW + ACK/retry; ข้อ E2 (node A, B), E4 (ฝั่ง node),
E1 และ E6 "(และ node)", E3 สำรอง (deep sleep ของ node)

## ไฟล์ที่รับผิดชอบ
`firmware/node/**` (commit ขึ้นต้น `[node]`)

## งานที่ต้องทำ
- [ ] 5-9 พ.ย. ต่อ node บน breadboard (DHT22 GPIO4 + pull-up 10k, reed switch GPIO33) คอมไพล์ `pio run -e box_a`
      แก้ error แล้วจด MAC ของ node
- [ ] รับ MAC และ channel ของ gateway จาก Napat ใส่ใน `include/config.h`
- [ ] 10-14 พ.ย. ทดสอบ ESP-NOW + ACK ตรวจ Serial ว่ามี "not acknowledged" เกิดเมื่อไร ปรับ `ACK_TIMEOUT_MS`/`MAX_RETRIES`
- [ ] ตรวจขั้วของ reed switch (ประตูปิด = LOW ตามโค้ด) ถ้ากลับด้านให้แก้ `doorOpen()` ทดสอบเหตุการณ์ประตูส่งทันที
- [ ] flash box_A และ box_B (`-e box_a`, `-e box_b`) และทดสอบสองตัวพร้อมกัน
- [ ] OTA ของ node (E6): ต่อจัมเปอร์ GPIO32-GND, รีเซ็ต, `pio run -e box_a_ota -t upload` ต้อง demo ได้
- [ ] ช่วยประกอบกล่องร่วมกับ Teerapat (ข้อเสนอ) และช่วย Methasit รันทดสอบ T7, T10 และวัด packet delivery
- [ ] (สำรอง E3) ถ้ามีเวลา: deep sleep + วัดกระแส แต่ต้องออกแบบให้ node ยังรับ ACK ได้ ปัจจุบันโค้ดไม่มี sleep

## ต้องอธิบายได้ตอน Q&A
- E2: ทำไม ISR ต้องสั้น (ส่งแค่สัญญาณเข้า queue), debounce 50 ms ทำที่ไหน, ทำไมใช้ queue ไม่ใช้ busy polling
- E1: 3 tasks (sensor, tx, door) และเหตุผลของ priority ที่ตั้ง (door=3, tx=2, sensor=1)
- E4: ลำดับ ACK/retry: ส่ง -> รอ event bit -> ตรวจ `seq` -> ส่งซ้ำสูงสุด 4 ครั้ง; ทำไมส่งซ้ำใช้ `seq` เดิม
- E6: โหมด OTA ทำงานอย่างไร ทำไมต้องใช้จัมเปอร์
- ถ้า gateway ล่ม node ทำอะไร (ตอนนี้: ส่งไม่สำเร็จแล้วทิ้งข้อความ ไม่มี buffer ที่ node)

## ช่องว่างที่รู้อยู่แล้ว
node ไม่เก็บข้อความที่ส่งไม่สำเร็จ, ESP-NOW ไม่เข้ารหัส, DHT22 อ่านได้ช้า (ประมาณ 2 วินาทีต่อครั้ง) ซึ่งพอสำหรับรอบ 30 วินาที
