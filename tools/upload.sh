#!/usr/bin/env bash
# M5StickC Plus2 倒立振子ファームの書き込みヘルパー
# 使い方:
#   ./tools/upload.sh           # コンパイル + 書き込み
#   ./tools/upload.sh -c        # コンパイルのみ
#   ./tools/upload.sh -m        # 書き込み後にシリアルモニタを開く
#   PORT=/dev/cu.xxx ./tools/upload.sh   # ポート上書き

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SKETCH="$REPO_ROOT/inverted_pendulum"
FQBN="m5stack:esp32:m5stack_stickc_plus2"
PORT="${PORT:-/dev/cu.usbserial-5B090208431}"
BAUD=115200

COMPILE_ONLY=0
OPEN_MONITOR=0
while getopts "cm" opt; do
  case "$opt" in
    c) COMPILE_ONLY=1 ;;
    m) OPEN_MONITOR=1 ;;
    *) echo "Usage: $0 [-c] [-m]" >&2; exit 1 ;;
  esac
done

echo "▶ Compiling $SKETCH ..."
arduino-cli compile --fqbn "$FQBN" "$SKETCH"

if [[ "$COMPILE_ONLY" -eq 1 ]]; then
  echo "✓ Compile only (skipped upload)"
  exit 0
fi

if [[ ! -e "$PORT" ]]; then
  echo "✗ Serial port not found: $PORT" >&2
  echo "  Available ports:" >&2
  ls /dev/cu.* 2>/dev/null | sed 's/^/    /' >&2
  exit 1
fi

echo "▶ Uploading to $PORT ..."
arduino-cli upload --fqbn "$FQBN" --port "$PORT" "$SKETCH"
echo "✓ Upload complete"

if [[ "$OPEN_MONITOR" -eq 1 ]]; then
  echo "▶ Opening serial monitor ($BAUD baud). Press Ctrl-A then Ctrl-X to exit screen."
  sleep 1
  exec screen "$PORT" "$BAUD"
fi
