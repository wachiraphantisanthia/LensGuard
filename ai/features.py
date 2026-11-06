"""Feature engineering shared by training and the live controller.

Input: a DataFrame indexed by 1-minute timestamps with columns rh, temp, door, fan.
Training adds the target rh_future = rh shifted by HORIZON minutes.
"""
import numpy as np
import pandas as pd

HORIZON_MIN = 15  # predict RH this many minutes ahead
MIN_HISTORY = 11  # minutes of history needed (largest lag + 1)

FEATURES = ["rh", "rh_lag1", "rh_lag5", "rh_lag10", "rh_diff5",
            "temp", "door", "fan", "hour_sin", "hour_cos"]


def add_features(df):
    df = df.copy()
    df["rh_lag1"] = df["rh"].shift(1)
    df["rh_lag5"] = df["rh"].shift(5)
    df["rh_lag10"] = df["rh"].shift(10)
    df["rh_diff5"] = df["rh"] - df["rh_lag5"]
    hour = df.index.hour + df.index.minute / 60.0
    df["hour_sin"] = np.sin(2 * np.pi * hour / 24.0)
    df["hour_cos"] = np.cos(2 * np.pi * hour / 24.0)
    return df


def to_minute_frame(raw):
    """raw columns: ts (datetime), rh, temp, door, fan -> 1-minute grid for one node."""
    raw = raw.set_index("ts").sort_index()
    minute = raw[["rh", "temp"]].resample("1min").mean()
    minute["door"] = raw["door"].resample("1min").max()
    minute["fan"] = raw["fan"].resample("1min").max()
    minute[["rh", "temp"]] = minute[["rh", "temp"]].interpolate(limit=5)
    minute[["door", "fan"]] = minute[["door", "fan"]].ffill(limit=5)
    return minute


def build_training_frame(raw_all):
    parts = []
    for _, g in raw_all.groupby("node_id"):
        m = add_features(to_minute_frame(g))
        m["rh_future"] = m["rh"].shift(-HORIZON_MIN)
        parts.append(m)
    out = pd.concat(parts).dropna(subset=FEATURES + ["rh_future"])
    return out.sort_index()
