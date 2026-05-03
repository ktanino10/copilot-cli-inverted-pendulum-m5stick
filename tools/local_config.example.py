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
#   "http://10.39.0.42"          # phone-tether DHCP lease
#   "http://pendulum.local"      # if mDNS is set up
M5_URL = "http://192.168.4.1"   # default = the firmware's AP-mode IP

# Server bind port.  Override only if 5000 conflicts.
PORT = 5000

# ── CORS for GitHub Pages → home server ──────────────────────────────
# When the Pages UI talks to your home server (cross-origin), the
# browser needs an explicit allowlist.  Add the origin you serve from.
CORS_ALLOW_ORIGINS = [
    # "https://ktanino10.github.io",
]

# ── (Internal) Owner-only password gate ──────────────────────────────
# Used by tools/build_demo.py when generating docs/demo/.  Only the
# SHA-256 hash is baked into the public output — the password itself
# never leaves this gitignored file.  Operating procedure is intentionally
# not documented in this template; see your private notes.
#
# Leave empty ("") to skip baking a hash entirely.
LIVE_PASSWORD = ""

# Optional default backend URL stored alongside the hash for owner use.
LIVE_DEFAULT_URL = ""
