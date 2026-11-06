# รูปแบบข้อความ (เจ้าของ: Teerapat) เวอร์ชัน 1

## Node -> Gateway (ESP-NOW, JSON string)
```json
{"node_id":"box_A","seq":1042,"up":86400,"type":"periodic","temp":28.4,"rh":52.1,"door":0}
```
- `seq` เพิ่มทีละ 1 ใช้นับแพ็กเก็ตที่หาย ส่งซ้ำ (retry) ใช้ `seq` เดิม
- `up` คือ uptime วินาที (node ไม่มีนาฬิกาจริง)
- `type` เป็น `periodic` หรือ `door` (เหตุการณ์จาก ISR)
- `temp`/`rh` จะไม่มีในข้อความถ้าอ่าน DHT22 ไม่ได้

## Gateway -> Node (ACK, 4 ไบต์)
`uint32_t seq` ของข้อความที่ได้รับ

## Gateway -> Cloud (MQTT)
| หัวข้อ | เนื้อหา | ผู้ใช้ |
|---|---|---|
| `lensguard/<node_id>/telemetry` | `{"node_id","seq","ts","type","temp","rh","door","fan","rx","lost"}` | dashboard, ai/collect.py, ai/run_controller.py |
| `lensguard/gateway/telemetry` | `{"node_id":"gateway","ts","temp_ref","ldr","mq135","mq_ready"}` | dashboard |
| `lensguard/alerts` | `{"event","node_id","value","ts"}` | dashboard/notifier.py |
| `lensguard/gateway/status` | `online` / `offline` (retained; offline มาจาก MQTT last will) | dashboard/notifier.py |

`event` เป็น `rh_high` (value = RH), `node_offline` (value = วินาทีที่เงียบ), `door_open` (value = วินาทีที่เปิดค้าง),
`gel_saturated` (value = RH ปัจจุบัน)
`ts` เป็น epoch จาก NTP (เป็น 0 ถ้ายังซิงก์ไม่ได้) `rx`/`lost` ใช้คำนวณ packet delivery rate

## AI -> Gateway (MQTT)
หัวข้อ `lensguard/cmd/fan/<node_id>`: `{"fan":true,"pred_rh":56.1,"rh":55.0}` (ส่งทุก 60 วินาที)

## หมายเหตุเรื่อง cloud
หัวข้อข้างบนเป็นแบบ MQTT ทั่วไป NETPIE และ ThingSpeak มีรูปแบบหัวข้อ/ข้อมูลของตัวเอง ถ้าเลือกบริการใด
ต้องปรับตารางนี้และโค้ดสามจุด (gateway, ai, notifier) ให้ตรง
