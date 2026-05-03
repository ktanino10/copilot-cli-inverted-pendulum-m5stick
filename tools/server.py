"""
Inverted Pendulum tuning server.
Hosts test session UI, proxies to M5StickC, saves sessions to disk.
Usage: python3 tools/server.py
"""
import io
import json
import os
import threading
import time
from datetime import datetime
from pathlib import Path

import requests
from flask import Flask, jsonify, request, send_from_directory

try:
    from PIL import Image  # type: ignore
    _PIL_OK = True
except Exception:
    _PIL_OK = False

# ── config (local_config.py overrides; env vars override that) ────────
try:
    from local_config import (  # type: ignore
        M5_URL as _CFG_M5_URL,
        PORT as _CFG_PORT,
        CORS_ALLOW_ORIGINS as _CFG_CORS,
    )
except Exception:
    _CFG_M5_URL = "http://192.168.4.1"
    _CFG_PORT = 5000
    _CFG_CORS = []

M5_BASE = os.environ.get("M5_URL", _CFG_M5_URL)
PORT    = int(os.environ.get("PORT", _CFG_PORT))
ROOT    = Path(__file__).parent
SESSDIR = ROOT / "data" / "sessions"
SESSDIR.mkdir(parents=True, exist_ok=True)

# Mutable runtime state — the dashboard can override M5_BASE at runtime
# via POST /api/m5_target without restarting the server. Useful when the
# device's IP changes between locations (home WiFi vs phone tethering).
_runtime = {"m5_base": M5_BASE}

def _normalize_url(raw: str) -> str:
    s = (raw or "").strip().rstrip("/")
    if not s:
        return ""
    if not s.startswith(("http://", "https://")):
        s = "http://" + s
    return s

# CORS_ALLOW_ORIGINS env var (comma-sep) wins; else local_config list.
_env_cors = os.environ.get("CORS_ALLOW_ORIGINS", "").strip()
if _env_cors:
    CORS_ORIGINS = [o.strip() for o in _env_cors.split(",") if o.strip()]
else:
    CORS_ORIGINS = list(_CFG_CORS)

app = Flask(__name__, static_folder=str(ROOT / "templates"), static_url_path="")

# ── CORS — required for Pages LIVE mode (cross-origin fetches) ────────
@app.after_request
def _add_cors(resp):
    origin = request.headers.get("Origin", "")
    if origin and (origin in CORS_ORIGINS or "*" in CORS_ORIGINS):
        resp.headers["Access-Control-Allow-Origin"] = origin
        resp.headers["Vary"] = "Origin"
        resp.headers["Access-Control-Allow-Methods"] = "GET,POST,DELETE,OPTIONS"
        resp.headers["Access-Control-Allow-Headers"] = "Content-Type"
    return resp

@app.route("/api/<path:_p>", methods=["OPTIONS"])
def _cors_preflight(_p):
    return ("", 204)

# ── routes ────────────────────────────────────────────────────────────
@app.route("/")
def index():
    return send_from_directory(app.static_folder, "index.html")

@app.route("/dash")
def dash():
    return send_from_directory(app.static_folder, "dash.html")

@app.route("/api/m5_target", methods=["GET", "POST"])
def api_m5_target():
    if request.method == "POST":
        body = request.get_json(silent=True) or {}
        new_url = _normalize_url(body.get("url", ""))
        if not new_url:
            return jsonify({"error": "empty url"}), 400
        _runtime["m5_base"] = new_url
    return jsonify({"url": _runtime["m5_base"]})

@app.route("/api/s")
def api_status():
    try:
        r = requests.get(f"{_runtime['m5_base']}/s", timeout=0.4)
        return (r.text, r.status_code, {"Content-Type": "application/json"})
    except Exception as e:
        return jsonify({"error": str(e)}), 502

@app.route("/api/c")
def api_cmd():
    q = request.args.get("q", "")
    try:
        r = requests.get(f"{_runtime['m5_base']}/c", params={"q": q}, timeout=1.0)
        return (r.text, r.status_code, {"Content-Type": "text/plain"})
    except Exception as e:
        return f"ERR {e}", 502

# ─── Mona-on-M5 face-link streamer ───────────────────────────────────
# When the dashboard toggles "🔗 LINK MONA", we start a background thread
# that streams ~5 FPS JPEG frames of the dashboard mascot to the M5 LCD
# (POST /face). The M5 firmware caches the latest frame and renders it on
# the right side of the LCD next to the existing PID/IP status text.
#
# Why streaming (not pre-uploaded) — mascot.gif is 3.5 MB, far too large
# for the device's flash. Streaming lets the M5 mirror exactly what the
# browser shows without any local storage cost.
#
# The thread auto-stops on UNLINK; the firmware also auto-reverts to
# text-only mode after 5 s of silence (failsafe in case this process
# dies or the network drops).
_MASCOT_GIF = ROOT / "templates" / "assets" / "mascot.gif"
_FACE_SIZE  = 72                           # mascot resized to this; +4px border each side = 80
_FACE_FPS   = 5
_FACE_STATE_COLORS = {                     # border color per attitude band
    "off":  (90, 90, 90),
    "calm": (60, 220, 120),
    "warn": (255, 200, 80),
    "crit": (255, 70, 70),
}

_face = {
    "on": False,
    "thread": None,
    "stop": threading.Event(),
    "frames": None,                         # cached PIL frames (RGB, _FACE_SIZE^2)
    "last_err": "",
    "frames_sent": 0,
}

def _load_mascot_frames():
    if _face["frames"] is not None:
        return _face["frames"]
    if not _PIL_OK or not _MASCOT_GIF.exists():
        _face["frames"] = []
        return _face["frames"]
    out = []
    with Image.open(_MASCOT_GIF) as im:
        try:
            i = 0
            while True:
                im.seek(i)
                fr = im.convert("RGBA")
                fr = fr.resize((_FACE_SIZE, _FACE_SIZE), Image.LANCZOS)
                bg = Image.new("RGB", fr.size, (0, 0, 0))
                bg.paste(fr, mask=fr.split()[3])
                out.append(bg)
                i += 1
        except EOFError:
            pass
    _face["frames"] = out
    return out

def _attitude_state():
    """Pull latest /s from M5 and bucket Angle into calm/warn/crit/off."""
    try:
        r = requests.get(f"{_runtime['m5_base']}/s", timeout=0.3)
        d = r.json()
    except Exception:
        return "off"
    if not d.get("on"):
        return "off"
    a = abs(float(d.get("angle", 0.0)))
    if a < 5.0:
        return "calm"
    if a < 15.0:
        return "warn"
    return "crit"

def _build_face_jpeg(frame_rgb, state):
    color = _FACE_STATE_COLORS.get(state, (90, 90, 90))
    canvas = Image.new("RGB", (_FACE_SIZE + 8, _FACE_SIZE + 8), color)
    canvas.paste(frame_rgb, (4, 4))
    buf = io.BytesIO()
    canvas.save(buf, format="JPEG", quality=70, optimize=False)
    return buf.getvalue()

def _face_streamer_loop():
    period = 1.0 / _FACE_FPS
    frames = _load_mascot_frames()
    if not frames:
        _face["last_err"] = "mascot.gif not found or PIL missing"
        _face["on"] = False
        return
    idx = 0
    while not _face["stop"].is_set():
        t0 = time.monotonic()
        state = _attitude_state()
        # crit advances frames at 2x for visible urgency, mirroring the
        # CSS animation-duration shift on the dashboard.
        step = 2 if state == "crit" else 1
        idx = (idx + step) % len(frames)
        try:
            jpeg = _build_face_jpeg(frames[idx], state)
            requests.post(
                f"{_runtime['m5_base']}/face",
                data=jpeg,
                headers={"Content-Type": "image/jpeg"},
                timeout=0.4,
            )
            _face["frames_sent"] += 1
            _face["last_err"] = ""
        except Exception as e:
            _face["last_err"] = str(e)
        # Sleep the remainder of the cycle.
        dt = time.monotonic() - t0
        if dt < period:
            _face["stop"].wait(period - dt)

@app.route("/api/lcd_link", methods=["GET", "POST"])
def api_lcd_link():
    if request.method == "POST":
        body = request.get_json(silent=True) or {}
        want_on = bool(body.get("on", False))
        if want_on and not _face["on"]:
            if not _PIL_OK:
                return jsonify({"on": False, "error": "Pillow not installed"}), 500
            if not _MASCOT_GIF.exists():
                return jsonify({"on": False, "error": "mascot.gif missing"}), 500
            _face["stop"].clear()
            t = threading.Thread(target=_face_streamer_loop, daemon=True)
            _face["thread"] = t
            _face["on"] = True
            _face["frames_sent"] = 0
            t.start()
        elif not want_on and _face["on"]:
            _face["stop"].set()
            _face["on"] = False
    return jsonify({
        "on":          _face["on"],
        "fps":         _FACE_FPS,
        "frames_sent": _face["frames_sent"],
        "last_err":    _face["last_err"],
    })

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
    print(f"M5 base   : {_runtime['m5_base']}  (changeable from dashboard)")
    print(f"Sessions  : {SESSDIR}")
    if CORS_ORIGINS:
        print(f"CORS      : {', '.join(CORS_ORIGINS)}")
    print(f"UI        : http://localhost:{PORT}/")
    app.run(host="0.0.0.0", port=PORT, debug=False, threaded=True)
