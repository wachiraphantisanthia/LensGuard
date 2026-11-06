"""Generate SYNTHETIC telemetry to test the training pipeline without hardware.

Do NOT use this data or its MAE in your report: it is made up.
Usage: python tools/simulate_data.py data/synthetic_telemetry.csv
"""
import sys
from pathlib import Path

import numpy as np
import pandas as pd

rng = np.random.default_rng(0)
out = Path(sys.argv[1] if len(sys.argv) > 1 else "data/synthetic_telemetry.csv")
out.parent.mkdir(parents=True, exist_ok=True)
rows = []
start = pd.Timestamp("2026-11-10 00:00:00")
for node, offset in [("box_A", 0.0), ("box_B", 4.0)]:
    rh = 50.0 + offset
    fan = 0
    seq = 0
    for i in range(3 * 24 * 60 * 2):                # 3 days, one sample per 30 s
        t = start + pd.Timedelta(seconds=30 * i)
        hour = t.hour + t.minute / 60
        ambient = 60 + 12 * np.sin(2 * np.pi * (hour - 9) / 24) + offset
        door = int(rng.random() < 0.002)
        fan = 1 if rh > 55 else (0 if rh < 45 else fan)
        rh += 0.002 * (ambient - rh) - 0.02 * fan + 0.3 * door + rng.normal(0, 0.05)
        seq += 1
        rows.append([t.isoformat() + "+00:00", node, seq, 28 + rng.normal(0, 0.2), rh, door, fan, "periodic"])
pd.DataFrame(rows, columns=["ts", "node_id", "seq", "temp", "rh", "door", "fan", "type"]).to_csv(out, index=False)
print("wrote", out, len(rows), "rows (synthetic)")
