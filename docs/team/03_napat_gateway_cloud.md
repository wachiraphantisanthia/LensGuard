# Napat Juengthanachote: Embedded Developer (Gateway & Cloud)

**Proposal ระบุ:** เฟิร์มแวร์ gateway, FreeRTOS, ควบคุมพัดลม, MQTT/TLS, OTA; ข้อ E1 (gateway), E4 (ฝั่ง gateway: buffer),
E5, E6

## ไฟล์ที่รับผิดชอบ
`firmware/gateway/**` (commit ขึ้นต้น `[gateway]`) และวงจรขับพัดลม MOSFET (ข้อเสนอ)

## งานที่ต้องทำ
- [ ] 5-8 พ.ย. คัดลอก `secrets.example.h` เป็น `secrets.h` คอมไพล์ gateway แก้ error flash แล้วจด MAC และ Wi-Fi channel
      ส่งให้ Wachiraphan (ใช้ hotspot ตัวเดียวกันตลอด และตั้ง channel ให้คงที่ถ้าทำได้)
- [ ] 8 พ.ย. ร่วมตัดสินใจเลือก cloud กับ Teerapat/Natthadon แล้วตั้งบัญชี/broker และเอา CA certificate มาใส่ `secrets.h`
- [ ] 10-19 พ.ย. ทดสอบ taskRx รับ node สองตัว, ตรวจ `rx`/`lost` ใน telemetry, MQTT TLS เชื่อมได้, ข้อมูลขึ้น cloud
- [ ] 20-26 พ.ย. ต่อวงจรพัดลม (logic-level MOSFET, 100Ω, 10k pull-down, ไดโอด) ทดสอบ GPIO26/27 วัดว่า MOSFET ไม่ร้อน
- [ ] ทดสอบ: ปิด Wi-Fi 5 นาทีแล้วเปิด (buffer ส่งย้อนหลัง), ปิด AI script (ใช้กฎ threshold), พัดลมรันครบ 30 นาทีแล้วหยุด,
      alert event ออกที่ `lensguard/alerts` (ทดสอบร่วมกับ Natthadon), OTA ของ gateway (E6)
- [ ] ปรับโค้ดตาม cloud ที่เลือก (หัวข้อ/รูปแบบข้อมูล) และบันทึกเหตุผลของค่าใน `settings.h`

## ต้องอธิบายได้ตอน Q&A
- E1: 4 tasks (fan=4, rx=3, cloud=2, sensors=1) ทำไมคุมพัดลมต้องมี priority สูงสุด, mutex ป้องกันอะไร
- E4: ring buffer 60 ข้อความ ทำงานอย่างไร (เก็บก่อนส่งเสมอ), ข้อมูลหายเมื่อไร (เต็มหรือไฟดับ), ตรวจ duplicate/`lost` จาก `seq`
- E5: TLS ตรวจใบรับรองด้วย CA ที่ใส่, secrets อยู่ในไฟล์ที่ git-ignore
- ลำดับตัดสินใจเปิดพัดลม: `RH_HARD` > `RH_OFF` > คำสั่ง AI (ถ้ายังใหม่) > กฎสำรอง และกลไก safety (timeout, cooldown)
- ทำไม gateway ไม่ส่ง Telegram เอง (alert event -> notifier ตาม block diagram)

## หมายเหตุภาระงาน
งานของคุณหนักที่สุด ขอให้ Teerapat ช่วยตั้ง broker/cloud และ Wachiraphan ช่วยทดสอบวงจรพัดลม
ค่า `RH_ON/OFF/HARD` และ timeout เป็นค่าสมมติ ต้องปรับตามข้อมูลจริง
