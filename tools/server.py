"""
Inverted Pendulum tuning server.
Hosts test session UI, proxies to M5StickC, saves sessions to disk.
Usage: python3 tools/server.py
"""
import json
import os
import time
from datetime import datetime
from pathlib import Path

import requests
from flask import Flask, jsonify, request, send_from_directory

# ── config ────────────────────────────────────────────────────────────
M5_BASE = os.environ.get("M5_URL", "http://192.168.10.32")
PORT    = int(os.environ.get("PORT", 5000))
ROOT    = Path(__file__).parent
SESSDIR = ROOT / "data" / "sessions"
SESSDIR.mkdir(parents=True, exist_ok=True)

app = Flask(__name__, static_folder=str(ROOT / "templates"), static_url_path="")

# ── routes ────────────────────────────────────────────────────────────
@app.route("/")
def index():
    return send_from_directory(app.static_folder, "index.html")

@app.route("/dash")
def dash():
    return send_from_directory(app.static_folder, "dash.html")

@app.route("/api/s")
def api_status():
    try:
        r = requests.get(f"{M5_BASE}/s", timeout=0.4)
        return (r.text, r.status_code, {"Content-Type": "application/json"})
    except Exception as e:
        return jsonify({"error": str(e)}), 502

@app.route("/api/c")
def api_cmd():
    q = request.args.get("q", "")
    try:
        r = requests.get(f"{M5_BASE}/c", params={"q": q}, timeout=1.0)
        return (r.text, r.status_code, {"Content-Type": "text/plain"})
    except Exception as e:
        return f"ERR {e}", 502

@app.route("/api/sessions", methods=["GET"])
def list_sessions():
    items = []
    for f in sorted(SESSDIR.glob("*.json")):
        try:
            with open(f) as fh:
                d = json.load(fh)
            items.append({
                "id":       f.stem,
                "name":     d.get("name", ""),
                "ts":       d.get("ts", 0),
                "duration": d.get("duration", 0),
                "samples":  len(d.get("samples", [])),
                "max_abs_angle": d.get("stats", {}).get("max_abs_angle"),
                "fall_dir": d.get("stats", {}).get("fall_dir"),
                "green_ratio": d.get("stats", {}).get("green_ratio"),
                "mean_abs_angle": d.get("stats", {}).get("mean_abs_angle"),
                "score":    d.get("stats", {}).get("score"),
                "params":   d.get("params", {}),
            })
        except Exception:
            pass
    return jsonify(items)

@app.route("/api/sessions/<sid>", methods=["GET"])
def get_session(sid):
    f = SESSDIR / f"{sid}.json"
    if not f.exists():
        return jsonify({"error": "not found"}), 404
    return send_from_directory(str(SESSDIR), f"{sid}.json")

@app.route("/api/sessions", methods=["POST"])
def save_session():
    d = request.get_json(force=True)
    name = (d.get("name") or "session").replace("/", "_").replace(" ", "_")
    ts = d.get("ts") or int(time.time() * 1000)
    stamp = datetime.fromtimestamp(ts / 1000).strftime("%Y%m%d_%H%M%S")
    sid = f"{stamp}_{name}"
    path = SESSDIR / f"{sid}.json"
    with open(path, "w") as fh:
        json.dump(d, fh, indent=1)
    # also drop a CSV alongside
    csv_path = SESSDIR / f"{sid}.csv"
    samples = d.get("samples", [])
    if samples:
        keys = list(samples[0].keys())
        with open(csv_path, "w") as fh:
            fh.write(",".join(keys) + "\n")
            for s in samples:
                fh.write(",".join(str(s.get(k, "")) for k in keys) + "\n")
    return jsonify({"id": sid, "path": str(path)})

@app.route("/api/sessions/<sid>", methods=["DELETE"])
def delete_session(sid):
    for ext in (".json", ".csv"):
        p = SESSDIR / f"{sid}{ext}"
        if p.exists():
            p.unlink()
    return jsonify({"ok": True})

# ── main ──────────────────────────────────────────────────────────────
if __name__ == "__main__":
    print(f"M5 base   : {M5_BASE}")
    print(f"Sessions  : {SESSDIR}")
    print(f"UI        : http://localhost:{PORT}/")
    app.run(host="0.0.0.0", port=PORT, debug=False, threaded=True)
