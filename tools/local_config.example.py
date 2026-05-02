"""
Local (gitignored) configuration for the IPS server and Pages demo build.

USAGE:
  cp tools/local_config.example.py tools/local_config.py
  # then edit tools/local_config.py with your private values

`tools/local_config.py` is in .gitignore — your real values never leave
your machine.  Both `tools/server.py` and `tools/build_demo.py` will
import it if it exists.

────────────────────────────────────────────────────────────────────────
Fields you can override:
────────────────────────────────────────────────────────────────────────
"""

# ── M5StickC backend ─────────────────────────────────────────────────
# Where the real M5 (or your home PC running server.py) lives, as seen
# from the machine running tools/server.py.  Examples:
#   "http://192.168.1.42"        # home LAN  (not committed; private)
#   "http://pendulum.local"      # if mDNS is set up
#   "http://100.96.12.34"        # Tailscale IP of the M5's host PC
M5_URL = "http://192.168.4.1"   # default = the firmware's AP-mode IP

# Server bind port.  Override only if 5000 conflicts.
PORT = 5000

# ── CORS for GitHub Pages → home server ──────────────────────────────
# When you switch the Pages UI into LIVE mode, the browser at
#   https://<owner>.github.io
# will cross-origin fetch your home server.  List the origin(s) you want
# to allow.  Use "*" only for testing (any site can talk to your server).
CORS_ALLOW_ORIGINS = [
    # "https://ktanino10.github.io",
]

# ── LIVE-mode password gate (Pages demo) ─────────────────────────────
# Set this to a password ONLY YOU know.  build_demo.py will hash it
# (SHA-256) and bake the hash — not the password itself — into the
# generated docs/demo/index.html.  Visitors who triple-tap the
# bottom-right corner are prompted; only matching SHA-256 unlocks
# LIVE mode.
#
# Leave empty ("") to disable the LIVE gate entirely (demo only).
LIVE_PASSWORD = ""

# Optional default Backend URL pre-filled in the LIVE-mode prompt.
# Most often this is your home PC's Tailscale address + port:
#   "http://100.96.12.34:5000"
LIVE_DEFAULT_URL = ""
