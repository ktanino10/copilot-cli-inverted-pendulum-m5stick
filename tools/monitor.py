#!/usr/bin/env python3
"""
PID リアルタイムモニター
シリアルデータをリアルタイム表示しながら、コマンドも送信可能。
全データをCSVに自動保存。

使い方:
  python3 tools/monitor.py

操作:
  kp=20     — パラメータ変更（即反映）
  on / off  — モーター制御
  ?         — 現在パラメータ表示
  q         — 終了

全データは tools/data/monitor_XXXXXXXX.csv に自動保存
"""

import serial
import time
import sys
import os
import threading
import select
from datetime import datetime

PORT = "/dev/cu.usbserial-5B090208431"
BAUD = 115200
DATA_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")

def main():
    os.makedirs(DATA_DIR, exist_ok=True)
    
    print(f"Connecting to {PORT}...")
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.05)
    except Exception as e:
        print(f"Error: {e}")
        return
    
    time.sleep(2)
    ser.flushInput()
    
    # ログファイル
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = os.path.join(DATA_DIR, f"monitor_{timestamp}.csv")
    log = open(log_file, "w")
    log.write("time,type,angle,power,powerL,powerR,raw\n")
    
    print("=" * 60)
    print("PID Real-time Monitor")
    print(f"Log: {log_file}")
    print("=" * 60)
    print("Commands: kp=20 kd=5 ki=1 po=0.5 on off ? q")
    print("=" * 60)
    
    running = True
    data_count = 0
    start_time = time.time()
    
    # 受信スレッド
    def reader():
        nonlocal data_count
        while running:
            try:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if not line:
                    continue
                
                t = time.time() - start_time
                
                if line.startswith("D,"):
                    parts = line.split(",")
                    if len(parts) >= 5:
                        angle = parts[1]
                        power = parts[2]
                        pL = parts[3]
                        pR = parts[4]
                        data_count += 1
                        
                        # 画面表示（10回に1回）
                        if data_count % 5 == 0:
                            bar_len = min(40, int(abs(float(angle)) * 2))
                            if float(angle) > 0:
                                bar = " " * 20 + "█" * bar_len
                            else:
                                bar = " " * max(0, 20 - bar_len) + "█" * bar_len
                            sys.stdout.write(f"\r  A={angle:>6s} pw={power:>5s} L={pL:>5s} R={pR:>5s} |{bar}|  ")
                            sys.stdout.flush()
                        
                        # ログ
                        log.write(f"{t:.3f},D,{angle},{power},{pL},{pR},{line}\n")
                        if data_count % 50 == 0:
                            log.flush()
                else:
                    print(f"\n  << {line}")
                    log.write(f"{t:.3f},MSG,,,,, {line}\n")
                    
            except Exception:
                pass
    
    thread = threading.Thread(target=reader, daemon=True)
    thread.start()
    
    try:
        while True:
            cmd = input().strip()
            
            if cmd == "q":
                break
            elif cmd:
                ser.write((cmd + "\n").encode())
                t = time.time() - start_time
                log.write(f"{t:.3f},CMD,,,,, {cmd}\n")
                log.flush()
                print(f"\n  >> {cmd}")
    except (KeyboardInterrupt, EOFError):
        pass
    
    running = False
    log.close()
    ser.close()
    
    print(f"\n\nData points: {data_count}")
    print(f"Log saved: {log_file}")

if __name__ == "__main__":
    main()
