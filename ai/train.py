"""Train the RH forecaster from data/telemetry.csv and report MAE (use these numbers in the report).

Usage: python train.py [path/to/telemetry.csv]
"""
import json
import sys
from pathlib import Path

import joblib
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.linear_model import LinearRegression
from sklearn.metrics import mean_absolute_error

from common import ROOT
from features import FEATURES, HORIZON_MIN, build_training_frame


def main():
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "data" / "telemetry.csv"
    raw = pd.read_csv(path, parse_dates=["ts"])
    raw["ts"] = pd.to_datetime(raw["ts"], utc=True).dt.tz_localize(None)
    for c in ["rh", "temp", "door", "fan"]:
        raw[c] = pd.to_numeric(raw[c], errors="coerce")
    frame = build_training_frame(raw)
    if len(frame) < 200:
        sys.exit(f"Only {len(frame)} usable rows. Collect at least 2-3 days of data first.")

    split = int(len(frame) * 0.8)  # chronological split, no shuffling
    train, test = frame.iloc[:split], frame.iloc[split:]
    X_tr, y_tr, X_te, y_te = train[FEATURES], train["rh_future"], test[FEATURES], test["rh_future"]

    models = {
        "linear_regression": LinearRegression(),
        "random_forest": RandomForestRegressor(n_estimators=100, max_depth=8, random_state=0, n_jobs=-1),
    }
    metrics = {"horizon_min": HORIZON_MIN, "train_rows": len(train), "test_rows": len(test),
               "baseline_persistence_mae": float(mean_absolute_error(y_te, test["rh"]))}
    best_name, best_mae = None, 1e9
    for name, m in models.items():
        m.fit(X_tr, y_tr)
        mae = float(mean_absolute_error(y_te, m.predict(X_te)))
        metrics[name + "_mae"] = mae
        if mae < best_mae:
            best_name, best_mae = name, mae
    metrics["selected"] = best_name

    out_dir = ROOT / "models"
    out_dir.mkdir(exist_ok=True)
    joblib.dump({"model": models[best_name], "features": FEATURES, "horizon_min": HORIZON_MIN},
                out_dir / "model.joblib")
    (out_dir / "metrics.json").write_text(json.dumps(metrics, indent=2))
    print(json.dumps(metrics, indent=2))


if __name__ == "__main__":
    main()
