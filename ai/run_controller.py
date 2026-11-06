"""Live AI controller: predicts RH 15 min ahead per box and publishes fan commands over MQTT.

The gateway applies the command only if it is fresh; otherwise it falls back to a simple
threshold rule, and it always enforces its own safety limits (max run time, RH_HARD).
"""
import json
import time
from collections import defaultdict, deque
from datetime import datetime, timezone

import joblib
import pandas as pd

from common import ROOT, NODES, TOPIC_PREFIX, connect, make_client
from features import FEATURES, MIN_HISTORY, add_features

RH_ON = 55.0    # start when predicted (or current) RH reaches this
RH_OFF = 45.0   # stop when current RH is at or below this
PUBLISH_EVERY_S = 60

bundle = joblib.load(ROOT / "models" / "model.joblib")
model = bundle["model"]
history = defaultdict(lambda: deque(maxlen=60))   # per node: one sample per minute
fan_state = defaultdict(bool)
last_pub = defaultdict(float)


def decide(node, rh, pred):
    if rh <= RH_OFF:
        fan_state[node] = False
    elif pred >= RH_ON or rh >= RH_ON:
        fan_state[node] = True
    return fan_state[node]


def on_message(client, userdata, msg):
    try:
        d = json.loads(msg.payload)
    except ValueError:
        return
    node = d.get("node_id")
    if node not in NODES or "rh" not in d or "temp" not in d:
        return
    now = datetime.now(timezone.utc).replace(tzinfo=None)
    minute = now.replace(second=0, microsecond=0)
    h = history[node]
    row = {"ts": minute, "rh": d["rh"], "temp": d["temp"], "door": d.get("door", 0), "fan": d.get("fan", 0)}
    if h and h[-1]["ts"] == minute:
        h[-1] = row
    else:
        h.append(row)
    if len(h) < MIN_HISTORY:
        return
    df = pd.DataFrame(list(h)).set_index("ts")
    df = df.asfreq("1min").interpolate(limit=3)
    df = add_features(df).dropna(subset=FEATURES)
    if df.empty:
        return
    pred = float(model.predict(df[FEATURES].iloc[[-1]])[0])
    want = decide(node, d["rh"], pred)
    if time.time() - last_pub[node] >= PUBLISH_EVERY_S:
        last_pub[node] = time.time()
        client.publish(f"{TOPIC_PREFIX}cmd/fan/{node}",
                       json.dumps({"fan": want, "pred_rh": round(pred, 1), "rh": d["rh"]}))
        print(node, "rh", d["rh"], "pred", round(pred, 1), "fan", want)


def main():
    c = make_client("lensguard-ai")
    c.on_connect = lambda client, *a: client.subscribe(TOPIC_PREFIX + "+/telemetry")
    c.on_message = on_message
    connect(c)
    c.loop_forever()


if __name__ == "__main__":
    main()
