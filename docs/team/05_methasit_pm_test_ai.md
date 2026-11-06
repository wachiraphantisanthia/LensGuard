# Methasit Phanawongwat: Project Manager & Test Lead

**Proposal ระบุ:** แผนงาน แผนทดสอบ เก็บ metric และสคริปต์ AI; ข้อ M4 (AI closed loop)

## ไฟล์ที่รับผิดชอบ
`ai/**`, `docs/test_plan.md`, `tools/simulate_data.py` (commit ขึ้นต้น `[ai]` / `[test]`)

## งานบริหาร
- [ ] ติดตามกำหนดจริง: checkpoint video 20 พ.ย. 23:59, นำเสนอและส่งงาน 8 ธ.ค. 23:59 (พลาด milestone หัก 2 คะแนน)
- [ ] ประชุมสั้นรายสัปดาห์ ตรวจ Git commit ของทุกคนทุกสัปดาห์ (peer factor อิงประวัติ commit และแบบประเมินเพื่อน)
- [ ] ติดตามรายการ M1-M8 ใน `docs/checklist.md`
- [ ] ติดต่อ problem owner/site จริง และเก็บหลักฐานปัญหา (ภาพเลนส์ขึ้นรา ข้อมูล RH ที่วัดเอง) (ข้อเสนอ)

## งาน AI
- [ ] เริ่ม `python collect.py` ทันทีที่ข้อมูลไหล ไม่เกิน 14 พ.ย. (ต้องข้อมูลจริง >= 2-3 วัน)
- [ ] ประมาณ 27 พ.ย. `python train.py` ดู `models/metrics.json` เทียบ MAE ของโมเดลกับ persistence baseline
      (ถ้าโมเดลไม่ชนะ baseline ให้รายงานตามจริง อย่าปรับให้ดูดี)
- [ ] `python run_controller.py` ต่อกับ gateway ทดสอบ closed loop: พัดลมตอบสนองต่อ `cmd/fan`, และเมื่อปิดสคริปต์ gateway ถอยไปกฎสำรอง
- [ ] ปรับ `RH_ON`/`RH_OFF` ให้ตรงกับ `firmware/gateway/include/settings.h` (เกณฑ์ใน proposal: 55% / 45%)
- ห้ามใช้ข้อมูลหรือ MAE จาก `simulate_data.py` ในรายงาน ข้อมูลนั้นสร้างขึ้นเอง

## งานทดสอบ
- [ ] กรอก `docs/test_plan.md` (ปกติ, edge, failure พร้อมขั้นตอนและผลจริง)
- [ ] วัด metric >= 3 อย่าง ทำซ้ำอย่างน้อย 3 รอบ รายงาน mean และ min/max: packet delivery rate, latency, sensor accuracy,
      AI MAE, uptime >= 24 ชม.
- [ ] วิเคราะห์เทียบ objective และเทียบกับตู้กันชื้นสำเร็จรูป/ซิลิกาเจลอย่างเดียวด้วยตัวเลขที่วัดเอง
- [ ] รวมเล่มรายงาน (<= 10 หน้า) และตรวจให้ครบ 6 หัวข้อตามโจทย์ (ข้อเสนอ)

## ต้องอธิบายได้ตอน Q&A
ฟีเจอร์ของโมเดลและเหตุผล, ทำไมแบ่งข้อมูลเทรน/ทดสอบตามเวลา (ไม่สุ่ม), MAE ที่ได้เทียบกับ baseline, ข้อจำกัดของโมเดล
(ข้อมูลน้อย, ฤดูกาลเดียว), วิธีวัดแต่ละ metric, ผลทดสอบ failure

## หมายเหตุภาระงาน
งานของคุณหนักเป็นอันดับสอง ขอให้ Wachiraphan ช่วยรันทดสอบ (T7, T10, PDR) และ Teerapat ช่วยรวมเล่มรายงาน
