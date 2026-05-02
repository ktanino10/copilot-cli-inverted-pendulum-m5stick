#!/usr/bin/env bash
# Start an ad-hoc HTTPS tunnel from a *.trycloudflare.com URL → local server.py.
# No Cloudflare account needed.  URL changes every restart.
#
# Usage:
#   ./tools/start_tunnel.sh           # uses PORT 5000
#   PORT=5099 ./tools/start_tunnel.sh # custom port
#
# Watch the output for a line like:
#   https://random-words-1234.trycloudflare.com
# Use that URL as needed.
set -e
PORT="${PORT:-5000}"
echo "→ Starting Cloudflare Quick Tunnel for http://localhost:${PORT}"
echo "→ Look for a line like  https://<random>.trycloudflare.com  below."
echo "→ Press Ctrl-C to stop."
echo
exec cloudflared tunnel --url "http://localhost:${PORT}" --no-autoupdate
