"""Log node telemetry from MQTT to data/telemetry.csv (this becomes your training data)."""
import csv
import json
from datetime import datetime, timezone

from common import ROOT, NODES, TOPIC_PREFIX, connect, make_client

OUT = ROOT / "data" / "telemetry.csv"
COLS = ["ts", "node_id", "seq", "temp", "rh", "door", "fan", "type"]


def main():
    OUT.parent.mkdir(exist_ok=True)
    new_file = not OUT.exists()
    f = OUT.open("a", newline="")
    writer = csv.writer(f)
    if new_file:
        writer.writerow(COLS)

    def on_connect(client, userdata, flags, rc, *args):
        client.subscribe(TOPIC_PREFIX + "+/telemetry")
        print("subscribed, logging to", OUT)

    def on_message(client, userdata, msg):
        try:
            d = json.loads(msg.payload)
        except ValueError:
            return
        if d.get("node_id") not in NODES or "rh" not in d:
            return
        ts = datetime.fromtimestamp(d["ts"], timezone.utc) if d.get("ts") else datetime.now(timezone.utc)
        writer.writerow([ts.isoformat(), d["node_id"], d.get("seq"), d.get("temp"), d["rh"],
                         d.get("door", 0), d.get("fan", 0), d.get("type", "periodic")])
        f.flush()

    c = make_client("lensguard-collector")
    c.on_connect = on_connect
    c.on_message = on_message
    connect(c)
    c.loop_forever()


if __name__ == "__main__":
    main()
