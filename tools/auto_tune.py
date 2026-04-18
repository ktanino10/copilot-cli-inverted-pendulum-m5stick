#!/usr/bin/env python3
"""
PID 自動チューニングスクリプト
パラメータを自動で変えながらデータ収集し、安定時間を記録。
最適領域を自動計算。

使い方:
  1. ロボットを直立させてUSB接続
  2. python3 tools/auto_tune.py を実行
  3. 各パラメータセットで自動的にON→データ収集→OFF→次のパラメータ
  4. 結果をCSVとサマリーで出力
"""

import serial
import time
import sys
import os
from datetime import datetime

PORT = "/dev/cu.usbserial-5B090208431"
BAUD = 115200
DATA_DIR = os.path.join(os.path.dirname(__file__), "data")

# パラメータスイープ範囲
KP_RANGE = [10, 15, 20, 25, 30, 40, 50]
KD_RANGE = [2, 5, 8, 12, 15, 20]
KI_FIXED = 1.0
KSPD_FIXED = 5.0
KDST_FIXED = 0.14

# テスト時間（秒）
TEST_DURATION = 5
# 安定判定: Angle が ±STABLE_ANGLE 以内の時間
STABLE_ANGLE = 10.0
# 倒れた判定
FALLEN_ANGLE = 40.0

def send_cmd(ser, cmd):
    ser.write((cmd + "\n").encode())
    time.sleep(0.2)
    while ser.in_waiting:
        ser.readline()

def run_test(ser, kp, kd, ki):
    """1回のテスト: パラメータ設定→ON→データ収集→OFF"""
    # パラメータ設定
    send_cmd(ser, f"kp={kp}")
    send_cmd(ser, f"kd={kd}")
    send_cmd(ser, f"ki={ki}")
    time.sleep(0.3)
    
    # ON
    ser.flushInput()
    send_cmd(ser, "on")
    
    # データ収集
    angles = []
    powers = []
    start = time.time()
    fallen_time = None
    
    while time.time() - start < TEST_DURATION:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith("D,"):
                parts = line.split(",")
                if len(parts) >= 3:
                    angle = float(parts[1])
                    power = int(parts[2])
                    angles.append(angle)
                    powers.append(power)
                    
                    # 倒れた判定
                    if abs(angle) > FALLEN_ANGLE and fallen_time is None:
                        fallen_time = time.time() - start
        except:
            pass
    
    # OFF
    send_cmd(ser, "off")
    time.sleep(1.0)  # 安定するまで待つ
    
    # 結果分析
    if not angles:
        return {"stable_time": 0, "mean_angle": 0, "max_angle": 0, "fallen": True}
    
    # 安定時間: Angle が ±STABLE_ANGLE 以内だった時間の割合 × テスト時間
    stable_count = sum(1 for a in angles if abs(a) < STABLE_ANGLE)
    stable_ratio = stable_count / len(angles)
    stable_time = stable_ratio * TEST_DURATION
    
    mean_angle = sum(abs(a) for a in angles) / len(angles)
    max_angle = max(abs(a) for a in angles)
    
    return {
        "stable_time": stable_time,
        "stable_ratio": stable_ratio,
        "mean_angle": mean_angle,
        "max_angle": max_angle,
        "fallen_time": fallen_time,
        "fallen": fallen_time is not None,
        "n_points": len(angles)
    }

def main():
    os.makedirs(DATA_DIR, exist_ok=True)
    
    print(f"Connecting to {PORT}...")
    try:
        ser = serial.Serial(PORT, BAUD, timeout=0.1)
    except Exception as e:
        print(f"Error: {e}")
        return
    
    time.sleep(2)
    ser.flushInput()
    
    # 固定パラメータ設定
    send_cmd(ser, f"ki={KI_FIXED}")
    send_cmd(ser, f"kspd={KSPD_FIXED}")
    send_cmd(ser, f"kdst={KDST_FIXED}")
    
    results = []
    total = len(KP_RANGE) * len(KD_RANGE)
    count = 0
    
    print("=" * 60)
    print(f"Auto PID Tuning — {total} parameter sets")
    print(f"kp: {KP_RANGE}")
    print(f"kd: {KD_RANGE}")
    print(f"ki={KI_FIXED} kspd={KSPD_FIXED} kdst={KDST_FIXED}")
    print(f"Test duration: {TEST_DURATION}s each")
    print("=" * 60)
    print()
    print(">>> HOLD ROBOT UPRIGHT! Tests start in 3 seconds...")
    time.sleep(3)
    
    for kp in KP_RANGE:
        for kd in KD_RANGE:
            count += 1
            print(f"\n[{count}/{total}] kp={kp} kd={kd} ki={KI_FIXED} ... ", end="", flush=True)
            
            result = run_test(ser, kp, kd, KI_FIXED)
            result["kp"] = kp
            result["kd"] = kd
            result["ki"] = KI_FIXED
            results.append(result)
            
            st = result["stable_time"]
            ma = result["mean_angle"]
            fallen = "FALLEN" if result["fallen"] else "OK"
            print(f"stable={st:.1f}s mean_angle={ma:.1f}° {fallen}")
            
            # ロボットを立て直す時間
            print("    >>> Stand robot upright! (2s pause)", flush=True)
            time.sleep(2)
    
    ser.close()
    
    # 結果保存
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    result_file = os.path.join(DATA_DIR, f"autotune_{timestamp}.csv")
    with open(result_file, "w") as f:
        f.write("kp,kd,ki,stable_time,stable_ratio,mean_angle,max_angle,fallen,fallen_time,n_points\n")
        for r in results:
            f.write(f"{r['kp']},{r['kd']},{r['ki']},{r['stable_time']:.2f},"
                    f"{r.get('stable_ratio',0):.3f},{r['mean_angle']:.2f},{r['max_angle']:.1f},"
                    f"{r['fallen']},{r.get('fallen_time','')},{r.get('n_points',0)}\n")
    
    print("\n" + "=" * 60)
    print("RESULTS SUMMARY")
    print("=" * 60)
    
    # ベスト5を表示
    sorted_results = sorted(results, key=lambda r: r["stable_time"], reverse=True)
    print("\nTop 5 most stable parameter sets:")
    print(f"{'Rank':>4} {'kp':>5} {'kd':>5} {'Stable':>8} {'MeanAng':>8} {'Status':>8}")
    print("-" * 45)
    for i, r in enumerate(sorted_results[:5]):
        status = "FALLEN" if r["fallen"] else "OK"
        print(f"{i+1:>4} {r['kp']:>5.0f} {r['kd']:>5.0f} {r['stable_time']:>7.1f}s {r['mean_angle']:>7.1f}° {status:>8}")
    
    print(f"\nResults saved to: {result_file}")
    print("Visualize with: python3 tools/visualize_root.py " + result_file)

if __name__ == "__main__":
    main()
