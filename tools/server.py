"""
Inverted Pendulum tuning server.
Hosts test session UI, proxies to M5StickC, saves sessions to disk.
Usage: python3 tools/server.py
"""
import base64
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
    from PIL import Image, ImageDraw, ImageFont  # type: ignore
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
_FACE_CANVAS = 128                         # composed image (must match firmware FACE_W)
_FACE_LABEL_H = 32                         # bottom strip — two lines (EN over JP)
_FACE_SIZE  = _FACE_CANVAS - _FACE_LABEL_H # mascot fills the rest above the label
_FACE_FPS   = 10                           # raised from 5 → smoother motion
                                           # (gated by firmware's 100ms display tick)
_FACE_BASE_ROT_DEG = -90                   # rotate composed image 90° so it
                                           # appears upright when M5 is held
                                           # vertically on its end. ±90 / 180 swap.
# Frame-step per send, indexed by attitude state. Higher = faster
# Mona dance, mirroring the dashboard's --mona-dur. Native gif = 32.6 fps,
# 176 frames per loop. At 10 fps sender:
#   STABLE   ~5.9s loop → step 3
#   WOBBLE   ~0.9s loop → step 20
#   FALLING! ~0.3s loop → step 60 (capped, looks chaotic = perfect for crit)
#   IDLE     ~5.9s loop → step 3
_FACE_STEP = {"off": 3, "calm": 3, "warn": 20, "crit": 60}
# Ripple-ring period (seconds) per state — mirrors --mona-dur in CSS.
_FACE_RING_PERIOD = {"off": 4.0, "calm": 4.0, "warn": 0.9, "crit": 0.18}
# State color = full LCD background (M5 fills entire screen with this; the
# JPEG canvas also uses it as its bg so they blend seamlessly).
_FACE_STATE_COLORS = {
    "off":  (0,   0,   0),     # IDLE → pure black ("通常状態の背景")
    "calm": (40,  170, 90),    # STABLE → green
    "warn": (215, 160, 40),    # WOBBLE → amber
    "crit": (210, 50,  50),    # FALLING! → red
}
# Lighter shade of the BG used for the ripple ring so it stays visible on
# top of its own state color.
_FACE_RING_COLORS = {
    "off":  (110, 200, 240),   # cyan ripple on black (matches dashboard)
    "calm": (170, 255, 200),
    "warn": (255, 230, 150),
    "crit": (255, 200, 200),
}
_FACE_STATE_LABEL = {                      # (English, Japanese) — mirrors updateMona()
    "off":  ("IDLE",     "停止中"),
    "calm": ("STABLE",   "安定"),
    "warn": ("WOBBLE",   "不安定"),
    "crit": ("FALLING!", "転倒中!"),
}

def _load_face_font(size):
    """Pick a CJK-capable bold sans-serif so both EN and JP labels render."""
    candidates = [
        "/System/Library/Fonts/ヒラギノ角ゴシック W7.ttc",
        "/System/Library/Fonts/ヒラギノ角ゴシック W6.ttc",
        "/System/Library/Fonts/ヒラギノ角ゴシック W3.ttc",
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/Hiragino Sans GB.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc",
        "/Library/Fonts/Arial Unicode.ttf",
    ]
    for p in candidates:
        try:
            return ImageFont.truetype(p, size)
        except Exception:
            continue
    return ImageFont.load_default()

_FACE_FONT_EN = None  # 13px latin
_FACE_FONT_JP = None  # 13px kana

_face = {
    "on": False,
    "thread": None,
    "stop": threading.Event(),
    "frames": None,                         # cached PIL frames (RGB, _FACE_SIZE^2)
    "last_err": "",
    "frames_sent": 0,
}

def _load_mascot_frames():
    """Decode mascot.gif once, cached as RGBA frames preserving native alpha
    so they composite cleanly onto any state-color background later."""
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
                fr = im.convert("RGBA").resize(
                    (_FACE_SIZE, _FACE_SIZE), Image.LANCZOS)
                out.append(fr)
                i += 1
        except EOFError:
            pass
    _face["frames"] = out
    return out

def _attitude_snapshot():
    """Pull latest /s and return (state, angle, dangle, motor_on)."""
    try:
        r = requests.get(f"{_runtime['m5_base']}/s", timeout=0.3)
        d = r.json()
    except Exception:
        return ("off", 0.0, 0.0, False)
    motor_on = bool(d.get("on"))
    angle    = float(d.get("angle", 0.0))
    dangle   = float(d.get("dangle", 0.0))
    if not motor_on:
        return ("off", angle, dangle, False)
    a  = abs(angle)
    da = abs(dangle)
    # Mirrors updateMona() in tools/templates/index.html.
    if a >= 15.0 or da > 80.0:
        state = "crit"
    elif a >= 5.0 or da > 30.0:
        state = "warn"
    else:
        state = "calm"
    return (state, angle, dangle, True)

def _build_face_jpeg(frame_rgba, state, angle, t):
    """Compose state-color full-screen background + ripple ring + Mona +
    bilingual EN/JP label. The composed JPEG is rotated -90° so it reads
    upright when the M5 is held on its end. Layout (pre-rotation):

        ┌──────────────────────┐  ← entire 128² is filled with state color
        │      ((( ring )))    │     (M5 also fills the LCD edges with the
        │   ╭──────────╮       │      same color via /face?bg=RRGGBB so it
        │   │   MONA   │       │      blends seamlessly across the screen)
        │   │ (alpha)  │       │
        │   ╰──────────╯       │
        │      STABLE          │  ← English label (white, dark stroke)
        │       安定           │  ← Japanese label
        └──────────────────────┘
    """
    global _FACE_FONT_EN, _FACE_FONT_JP
    if _FACE_FONT_EN is None:
        _FACE_FONT_EN = _load_face_font(13)
    if _FACE_FONT_JP is None:
        _FACE_FONT_JP = _load_face_font(13)

    bg = _FACE_STATE_COLORS.get(state, (0, 0, 0))
    ring_col = _FACE_RING_COLORS.get(state, (110, 200, 240))
    label_en, label_jp = _FACE_STATE_LABEL.get(state, ("", ""))

    # ─── Mona: angle-driven tilt + horizontal flip on negative tilt ───
    sx   = 1 if angle >= 0 else -1
    tilt = max(-30.0, min(30.0, angle * 0.7)) * sx
    mona = frame_rgba
    if sx == -1:
        mona = mona.transpose(Image.FLIP_LEFT_RIGHT)
    if abs(tilt) > 0.5:
        mona = mona.rotate(-tilt, resample=Image.BILINEAR)  # RGBA → fillcolor stays transparent

    # ─── Canvas filled with state color (no border now — M5 fills the
    #     surrounding LCD area with the same color so the whole device
    #     looks uniform). ───
    canvas = Image.new("RGBA", (_FACE_CANVAS, _FACE_CANVAS), (*bg, 255))

    mx = (_FACE_CANVAS - _FACE_SIZE) // 2
    my = 0
    cx = mx + _FACE_SIZE // 2
    cy = my + _FACE_SIZE // 2

    # ─── Ripple rings (two echoes, staggered) drawn UNDER Mona so the
    #     character stays in front of the expanding wave. ───
    period = _FACE_RING_PERIOD.get(state, 4.0)
    if period > 0:
        overlay = Image.new("RGBA", canvas.size, (0, 0, 0, 0))
        od = ImageDraw.Draw(overlay)
        n_rings = 2
        if state == "crit":
            scale_lo, scale_hi, alpha_max = 0.85, 1.55, 230
        else:
            scale_lo, scale_hi, alpha_max = 0.90, 1.45, 170
        base_r = _FACE_SIZE // 2
        for k in range(n_rings):
            phase = ((t + k * period / n_rings) % period) / period
            scale = scale_lo + (scale_hi - scale_lo) * phase
            alpha = int(alpha_max * (1.0 - phase))
            if alpha < 8:
                continue
            r = int(base_r * scale)
            od.ellipse([cx - r, cy - r, cx + r, cy + r],
                       outline=(*ring_col, alpha), width=2)
        canvas = Image.alpha_composite(canvas, overlay)

    # ─── Mona pasted on top using its native alpha channel as mask ───
    canvas.paste(mona, (mx, my), mona)

    # ─── Bilingual label centered in the bottom strip ───
    draw = ImageDraw.Draw(canvas)
    text_color  = (255, 255, 255, 255)
    stroke_col  = (0,   0,   0,   255)
    label_top_y = _FACE_CANVAS - _FACE_LABEL_H
    line_h      = _FACE_LABEL_H // 2  # 16

    def _centered_text(s, font, base_y):
        b = draw.textbbox((0, 0), s, font=font, stroke_width=1)
        tw = b[2] - b[0]
        th = b[3] - b[1]
        tx = (_FACE_CANVAS - tw) // 2 - b[0]
        ty = base_y + (line_h - th) // 2 - b[1]
        draw.text((tx, ty), s, font=font, fill=text_color,
                  stroke_width=1, stroke_fill=stroke_col)

    if label_en:
        _centered_text(label_en, _FACE_FONT_EN, label_top_y)
    if label_jp:
        _centered_text(label_jp, _FACE_FONT_JP, label_top_y + line_h)

    # ─── Final rotation so it reads upright on a vertically-held M5 ───
    out = canvas.convert("RGB")
    if _FACE_BASE_ROT_DEG % 360 != 0:
        out = out.rotate(-_FACE_BASE_ROT_DEG, resample=Image.BILINEAR, expand=False)

    buf = io.BytesIO()
    out.save(buf, format="JPEG", quality=72, optimize=False)
    return buf.getvalue()

def _face_streamer_loop():
    period = 1.0 / _FACE_FPS
    frames = _load_mascot_frames()
    if not frames:
        _face["last_err"] = "mascot.gif not found or PIL missing"
        _face["on"] = False
        return
    idx = 0
    t_start = time.monotonic()
    while not _face["stop"].is_set():
        t0 = time.monotonic()
        state, angle, _dangle, _on = _attitude_snapshot()
        step = _FACE_STEP.get(state, 1)
        idx = (idx + step) % len(frames)
        try:
            jpeg = _build_face_jpeg(frames[idx], state, angle, t0 - t_start)
            # Base64-encode body so JPEG NUL bytes don't trip ESP32 WebServer.
            payload = base64.b64encode(jpeg)
            # Send the state color as ?bg=RRGGBB so the M5 can fill the
            # entire LCD with the same color the JPEG uses, giving the
            # illusion of a full-screen state takeover.
            r_bg, g_bg, b_bg = _FACE_STATE_COLORS.get(state, (0, 0, 0))
            bg_hex = f"{r_bg:02X}{g_bg:02X}{b_bg:02X}"
            r = requests.post(
                f"{_runtime['m5_base']}/face?bg={bg_hex}",
                data=payload,
                headers={"Content-Type": "text/plain"},
                timeout=0.4,
            )
            if r.status_code != 200:
                _face["last_err"] = f"M5 returned HTTP {r.status_code} (firmware may be outdated — re-flash to add /face)"
            else:
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
