#!/usr/bin/env bash
# Run the tuning UI server.
#   M5_URL=http://192.168.1.50 ./tools/run_server.sh
set -e
cd "$(dirname "$0")"
python3 -m pip install -q --user --break-system-packages -r requirements.txt
exec python3 server.py
