# 🤖 copilot-cli-inverted-pendulum-m5stick

> **GitHub Copilot CLI** でファームウェア開発から書き込み・デバッグまで行う倒立振子プロジェクト

![Copilot CLI](https://img.shields.io/badge/Built_with-Copilot_CLI-8b5cf6?style=for-the-badge&labelColor=0a0a0c)
![M5StickC Plus](https://img.shields.io/badge/M5StickC-Plus-orange?style=for-the-badge)
![PID Control](https://img.shields.io/badge/PID-Control-blue?style=for-the-badge)

**[日本語](#日本語) | [English](#english)**

---

<a id="日本語"></a>
## 日本語

### 概要

M5StickC Plus を使った倒立振子（自立バランスロボット）のプロジェクトです。
ファームウェアの作成、コンパイル、書き込み、デバッグまで、すべて **GitHub Copilot CLI** との対話で進めています。

### リポジトリ構成

このリポジトリの全体像です。**何がどこにあるか**、**どこから読めばいいか**を示します。

```
copilot-cli-inverted-pendulum-m5stick/
│
├── README.md                          ← 📌 今読んでいるファイル（概要・進捗・リンク集）
│
├── inverted_pendulum/
│   └── inverted_pendulum.ino          ← 🎯 メインファームウェア（PID制御 + カルマンフィルタ）
│                                         これが倒立振子を動かす本体コード
│
├── servo_test/
│   └── servo_test.ino                 ← 🔧 サーボ動作確認用テストスケッチ
│                                         ハードウェア到着後、最初に書き込んで動作確認する
│
├── servo_diag/
│   └── servo_diag.ino                 ← 🔍 GPIO全ピン自動スキャン診断スケッチ
│                                         サーボが動かないときのトラブルシュート用
│
└── docs/
    ├── pid_guide.md                   ← 🎓 PID制御 初心者ガイド
    │                                     ほうきバランスの例え話から直感的に理解する
    │
    └── pid_theory.md                  ← 📐 PID制御 理論編（上級者向け）
                                          ラグランジアン・状態空間・カルマンフィルタの数式
```

#### 読む順番のおすすめ

| あなたのレベル | まず読むもの | 次に読むもの |
|--------------|------------|------------|
| 初めてPID制御に触れる | `docs/pid_guide.md` | `README.md` → `inverted_pendulum.ino` のコメント |
| 制御工学の基礎がある | `docs/pid_theory.md` | `inverted_pendulum.ino` のソースコード |
| すぐ動かしたい | `README.md`（使い方セクション） | `servo_test.ino` で動作確認 → `inverted_pendulum.ino` 書き込み |

### きっかけ

書店で [Interface誌 2025年9月号](https://interface.cqpub.co.jp/202509st/) を見かけて、倒立振子の特集記事が目に留まりました。PID制御でロボットが自力でバランスを取るという仕組みが面白そうで、実際に手を動かしてやってみたくなり、キットと部品を揃えて挑戦することにしました。

Arduino や電子工作は多少経験がありますが、制御工学は初めてです。倒立振子を通じて PID 制御やセンサフュージョンを学びながら、将来的には 3 軸姿勢制御やリアルタイム可視化にも発展させていきたいと考えています。

### Copilot CLI でやったこと

- 倒立制御メインファームウェア (`inverted_pendulum.ino`) の設計・実装
- サーボ動作テスト・ピン自動スキャン診断スケッチの作成
- `arduino-cli` によるコンパイル・書き込みの実行
- サーボが動かない問題のデバッグ（配線診断、GPIO テスト）
- プロジェクトドキュメントの作成・更新

### ハードウェア

- **マイコン**: M5StickC Plus (ESP32, IMU: MPU6886)
- **サーボモータ**: FS90R (連続回転サーボ) × 2
- **タイヤ**: FS90R対応 × 2
- **ボディ**: Interface誌専用キット

### 配線

| RCサーボ1 | M5StickC Plus |
|-----------|---------------|
| PWM (橙)  | G0            |
| VCC (赤)  | 5V            |
| GND (茶)  | GND           |

| RCサーボ2 | M5StickC Plus |
|-----------|---------------|
| PWM (橙)  | G26           |
| VCC (赤)  | 5V            |
| GND (茶)  | GND           |

### 必要なライブラリ (Arduino IDE)

- [KalmanFilter](https://github.com/TKJElectronics/KalmanFilter)
- [RemoteXY](https://remotexy.com/en/help/)
- [M5StickCPlus](https://github.com/m5stack/M5StickC-Plus)
- ボードマネージャ: M5Stack

### 使い方

1. Arduino IDEでファームウェアを書き込む
2. M5StickCを水平に持ち、電源を入れ直す（キャリブレーション）
3. 「M5」ボタンを長押し → 倒立制御スタート

### PIDデフォルト値

```cpp
float kpower = 0.001;
float kp = 21.0;
float ki = 7.0;
float kd = 1.6;
float kdst = 0.07;
float kspd = 2.5;
```

### 進捗ログ

#### 2026-04-13: 半田付け完了・サーボ動作確認

- キット基板へのサーボ半田付けを完了
- サーボ動作テスト (`servo_test.ino`) およびピン自動スキャン診断を実施
- 配線修正後、サーボの動作を確認 ✅
- 再半田付け中に M5StickC Plus が過熱・故障 → 基板ショートによる損傷の可能性大
- USB 接続するだけで発熱し画面表示なし、復旧不可と判断 → **M5StickC Plus 交換待ち**

#### 2026-04-16: M5StickC Plus2 で復活・ファームウェア移植完了

- M5StickC Plus の代替として **M5StickC Plus2** を入手
- Plus → Plus2 の移植で判明した差異と解決策を以下にまとめる（⚠️ 同じキットで Plus2 を使う人向け）

##### Plus → Plus2 移植時のトラブルと対策

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 1 | **画面が表示されない** | `M5.begin()` + `M5.Lcd` は Plus 専用。Plus2 では API が異なる | `M5StickCPlus2.h` をインクルードし、`StickCP2.begin(cfg)` + `StickCP2.Display` を使用 |
| 2 | **サーボが回らない（PWM出力はOK）** | Plus2 はデフォルトで5V外部出力が無効。HATコネクタから5Vが出ていなかった | `auto cfg = M5.config(); cfg.output_power = true;` で5V出力を有効化 |
| 3 | **ボタンを押しても制御が止まらない** | Plus の `M5.BtnA.wasPressed()` / `wasClicked()` が Plus2 で期待通りに動作しない | `StickCP2.BtnA.wasReleased()` を使用（ボタンを離した瞬間にトグル） |
| 4 | **IDLEに切り替わるがモーターが止まらない** | `servo.write(90)` だけでは FS90R が完全停止しないことがある（PWM信号が残る） | `servo.detach()` でPWM信号を完全切断。再開時に `servo.attach()` で再接続 |
| 5 | **G0 に繋いだサーボが動かない** | Plus2 では HATコネクタが 8ピン→18ピンに変更され、G0 のピン位置が異なる | サーボピンを G0 → **G25** に変更（`SERVO1_PIN 25`） |
| 6 | **片方のサーボだけ回らない / 回転速度が左右で大きく異なる** | FS90R の個体差でニュートラル点（停止位置）が `write(90)` からずれている。今回の個体は停止点が約75°付近にあり、小さい制御出力だと停止域に入って回らなかった | `SERVO2_TRIM = -15` でニュートラル補正。`servo.write(90 + TRIM - cmd)` で停止点を調整。正しい値は個体ごとに異なるため、IDLE状態でサーボが静止する値を探して設定する |

##### Plus と Plus2 のハードウェア・API 対応表

| 項目 | M5StickC Plus | M5StickC Plus2 |
|------|--------------|----------------|
| 電源IC | AXP192 | AXP2101 |
| ライブラリ | `M5StickCPlus.h` | `M5StickCPlus2.h` |
| 初期化 | `M5.begin()` | `auto cfg = M5.config(); StickCP2.begin(cfg);` |
| 画面 | `M5.Lcd` | `StickCP2.Display` |
| IMU | `M5.Imu.getGyroData(&gx,&gy,&gz)` | `auto d = StickCP2.Imu.getImuData(); d.gyro.x` |
| バッテリー | `M5.Axp.GetBatVoltage()` | `StickCP2.Power.getBatteryLevel()` |
| 5V出力 | `M5.Axp.SetLDO2(true)` | `cfg.output_power = true` |
| ボタン | `M5.BtnA` / `M5.BtnB` | `StickCP2.BtnA` / `StickCP2.BtnB` |
| FQBN | `m5stack_stickc_plus` | `m5stack_stickc_plus2` |
| HATコネクタ | 8ピン | 18ピン（ピン配置変更） |

- サーボ動作テスト (`servo_test.ino`) → G26 で回転確認 ✅
- メインファームウェア (`inverted_pendulum.ino`) → Plus2 対応に移植、書き込み完了 ✅
- ボタン操作（Aボタンでスタート/ストップ）動作確認 ✅
- **次のステップ**: サーボ2本接続して倒立テスト

#### 2026-04-16: 息抜き — GitHub ステッカースライドショー 🐙

倒立制御のデバッグの合間に、M5StickC Plus2 の画面に GitHub ステッカーを表示するスケッチを作成。Aボタンで Octocat → Copilot → Duck → Mascot を切り替え表示できる。

- PNG/GIF 画像を Python（Pillow）で 110×110 に縮小し、RGB565 ビットマップに変換
- **バイトオーダーの罠**: M5GFX（LovyanGFX）の `pushImage` はビッグエンディアン順の RGB565 を期待する。ESP32 はリトルエンディアンなので、各ピクセルの上位・下位バイトをスワップしないと色が崩れる
- スケッチ: `octocat_display/octocat_display.ino`

#### 2026-04-18: 完全やり直し — pulse_drive 方式で両輪駆動成功 🎉

前回（4/16）の試行錯誤を踏まえ、ゼロからやり直し。n_shinichi氏のオリジナル実装を調査した結果、**ESP32Servo ライブラリが根本原因**と判明。手動パルス生成（`pulse_drive`）方式に全面書き換えし、**G0 + G26 の両サーボが同じ勢いで安定駆動に成功**。

<img src="media/servo_test_20260418.jpg" width="400" alt="両輪サーボ駆動テスト成功">

<!-- YouTube: 両輪サーボ駆動テスト -->
[![両輪サーボ駆動テスト](https://img.youtube.com/vi/syauXEm0TFY/0.jpg)](https://www.youtube.com/watch?v=syauXEm0TFY)

##### 判明した追加の問題と対策

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 7 | **両サーボ接続時に片方だけ遅い** | `pulse_drive` が OFF のサーボにも1500μsパルスを出し続けていた。個体差でニュートラルがずれているサーボが微回転し、電流を消費して他方に影響 | OFF のサーボにはパルスを一切出さない（`digitalWrite(pin, LOW)` のまま）。`servo_stop()` もパルスなしに変更 |
| 8 | **ボタンが反応しない** | `digitalRead(BTN_A)` のチェックが1秒に1回のループ内にあり、タイミングが合わないと検出できなかった | ボタン処理を100msループに移動 + 300msデバウンス追加 |

##### 技術的な学び

- **ESP32Servo vs pulse_drive**: `servo.attach()` は LEDC チャネルを占有し G0 のブート問題を起こす。`digitalWrite` による手動パルスなら G0 も安全に使える
- **マイクロ秒制御**: `servo.write(90)` の角度指定ではなく、1500μs = 停止、500-2500μs = 速度制御のμsベースが正確
- **パルス制御の副作用**: 停止のつもりで1500μsパルスを出し続けると、ニュートラルずれのあるサーボが微回転する。完全停止にはパルスを止める必要がある

### 📚 PID制御ガイド

PID制御の基礎を初心者向けに解説したガイドを用意しました。ほうきバランスの例え話から、倒立振子への応用、パラメータ調整の手順まで、Interface誌の内容をベースにわかりやすくまとめています。

👉 **[docs/pid_guide.md](docs/pid_guide.md)**

### 📐 PID制御 理論編（上級者向け）

運動方程式の導出、状態空間表現、カルマンフィルタの数理、安定性解析、LQR等の発展的手法を数式で解説しています。制御工学を深く学びたい方向け。

👉 **[docs/pid_theory.md](docs/pid_theory.md)**

### 参考

- [Interface誌 記事ページ](https://interface.cqpub.co.jp/202509st/)
- [しんさん (n_shinichi) の倒立振子ブログ記事](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903) — M5StickC Plus / Plus2 向けサンプルスケッチの解説。Interface誌キットの制作者ご本人による技術情報

---

<a id="english"></a>
## English

### Overview

An inverted pendulum (self-balancing robot) project using M5StickC Plus.
Everything from firmware development, compilation, flashing, and debugging was done entirely through conversations with **GitHub Copilot CLI**.

### Repository Structure

An overview of the entire repository — **what's where** and **where to start reading**.

```
copilot-cli-inverted-pendulum-m5stick/
│
├── README.md                          ← 📌 You are here (overview, progress, links)
│
├── inverted_pendulum/
│   └── inverted_pendulum.ino          ← 🎯 Main firmware (PID control + Kalman filter)
│                                         The core code that makes the robot balance
│
├── servo_test/
│   └── servo_test.ino                 ← 🔧 Servo test sketch
│                                         Flash this first to verify hardware works
│
├── servo_diag/
│   └── servo_diag.ino                 ← 🔍 GPIO auto-scan diagnostic sketch
│                                         Troubleshooting tool when servos won't spin
│
└── docs/
    ├── pid_guide.md                   ← 🎓 PID Control — Beginner's Guide
    │                                     Intuitive explanations with analogies
    │
    └── pid_theory.md                  ← 📐 PID Control — Theory (Advanced)
                                          Lagrangian, state-space, Kalman filter math
```

#### Suggested Reading Order

| Your Level | Start Here | Then Read |
|-----------|-----------|-----------|
| New to PID control | `docs/pid_guide.md` | `README.md` → comments in `inverted_pendulum.ino` |
| Control theory background | `docs/pid_theory.md` | `inverted_pendulum.ino` source code |
| Just want to run it | `README.md` (Usage section) | `servo_test.ino` to verify → flash `inverted_pendulum.ino` |

### Motivation

Inspired by a feature article on inverted pendulums in [Interface Magazine (Sep 2025)](https://interface.cqpub.co.jp/202509st/). The idea of a robot balancing itself using PID control was fascinating, so I decided to build one with the magazine's kit.

### What Copilot CLI Did

- Designed and implemented the main balance control firmware (`inverted_pendulum.ino`)
- Created servo test and pin auto-scan diagnostic sketches
- Compiled and flashed firmware via `arduino-cli`
- Debugged servo issues (wiring diagnosis, GPIO testing)
- Created and maintained project documentation

### Hardware

- **MCU**: M5StickC Plus (ESP32, IMU: MPU6886)
- **Servos**: FS90R (continuous rotation) × 2
- **Wheels**: FS90R compatible × 2
- **Body**: Interface Magazine kit

### Wiring

| RC Servo 1 | M5StickC Plus |
|------------|---------------|
| PWM (orange) | G0          |
| VCC (red)    | 5V          |
| GND (brown)  | GND         |

| RC Servo 2 | M5StickC Plus |
|------------|---------------|
| PWM (orange) | G26         |
| VCC (red)    | 5V          |
| GND (brown)  | GND         |

### Required Libraries (Arduino IDE)

- [KalmanFilter](https://github.com/TKJElectronics/KalmanFilter)
- [RemoteXY](https://remotexy.com/en/help/)
- [M5StickCPlus](https://github.com/m5stack/M5StickC-Plus)
- Board Manager: M5Stack

### Usage

1. Flash the firmware using Arduino IDE
2. Hold M5StickC horizontally and power on (gyro calibration)
3. Long-press the M5 button → balance control starts

### Progress Log

#### 2026-04-13: Soldering complete / Servo test

- Completed soldering servos to the kit PCB
- Ran servo tests (`servo_test.ino`) and pin auto-scan diagnostics
- Confirmed servo operation after wiring fix ✅
- M5StickC Plus overheated during re-soldering → likely PCB short damage
- Unit unresponsive on USB connection → **awaiting replacement**

#### 2026-04-16: M5StickC Plus2 — firmware ported & working

- Replaced dead M5StickC Plus with **M5StickC Plus2**
- Ported all firmware from Plus to Plus2 — several breaking differences found and resolved

##### Troubleshooting: Plus → Plus2 Migration

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 1 | **Screen blank after flash** | `M5.begin()` + `M5.Lcd` are Plus-only APIs | Use `M5StickCPlus2.h`, `StickCP2.begin(cfg)`, `StickCP2.Display` |
| 2 | **Servo won't spin (PWM signal OK)** | Plus2 disables 5V external output by default — no power on HAT pins | Set `cfg.output_power = true` before `StickCP2.begin(cfg)` |
| 3 | **Button press doesn't stop control** | `wasPressed()` / `wasClicked()` behave differently on Plus2 | Use `StickCP2.BtnA.wasReleased()` — triggers on button release |
| 4 | **Motor keeps spinning after IDLE** | `servo.write(90)` alone doesn't fully stop FS90R (residual PWM) | Call `servo.detach()` to kill PWM signal; `servo.attach()` on restart |
| 5 | **G0 servo doesn't respond** | Plus2 HAT connector changed from 8-pin to 18-pin — G0 pin location differs | Change servo pin from G0 to **G25** (`SERVO1_PIN 25`) |
| 6 | **One servo barely spins / large speed difference between left and right** | FS90R neutral point (stop position) varies per unit. One servo's actual stop point was ~75° instead of 90°, so small PID outputs fell into the dead zone | Add `SERVO2_TRIM = -15` for neutral offset compensation. Use `servo.write(90 + TRIM - cmd)`. Find the correct trim value per servo by checking it stays still at IDLE |

##### Plus vs Plus2 API Reference

| Feature | M5StickC Plus | M5StickC Plus2 |
|---------|--------------|----------------|
| Power IC | AXP192 | AXP2101 |
| Library | `M5StickCPlus.h` | `M5StickCPlus2.h` |
| Init | `M5.begin()` | `auto cfg = M5.config(); StickCP2.begin(cfg);` |
| Display | `M5.Lcd` | `StickCP2.Display` |
| IMU | `M5.Imu.getGyroData(&gx,&gy,&gz)` | `auto d = StickCP2.Imu.getImuData(); d.gyro.x` |
| Battery | `M5.Axp.GetBatVoltage()` | `StickCP2.Power.getBatteryLevel()` |
| 5V output | `M5.Axp.SetLDO2(true)` | `cfg.output_power = true` |
| Buttons | `M5.BtnA` / `M5.BtnB` | `StickCP2.BtnA` / `StickCP2.BtnB` |
| FQBN | `m5stack_stickc_plus` | `m5stack_stickc_plus2` |
| HAT pins | 8-pin | 18-pin (different layout) |

- Servo test (`servo_test.ino`) — G26 confirmed working ✅
- Main firmware (`inverted_pendulum.ino`) — ported and flashed ✅
- Button control (A button start/stop toggle) confirmed ✅
- **Next**: Connect both servos and attempt balancing test

#### 2026-04-16: Fun break — GitHub Sticker Slideshow 🐙

During a debugging break, created a sketch that displays GitHub stickers on the M5StickC Plus2 screen. Press A button to cycle through Octocat → Copilot → Duck → Mascot.

- Converted PNG/GIF images to 110×110 RGB565 bitmaps using Python (Pillow)
- **Byte order gotcha**: M5GFX (LovyanGFX) `pushImage` expects big-endian RGB565, but ESP32 is little-endian. Without byte-swapping each pixel, colors appear corrupted (reds and blues swap)
- Sketch: `octocat_display/octocat_display.ino`

#### 2026-04-18: Full restart — Both servos running with pulse_drive 🎉

After the 4/16 struggles, started completely fresh. Research into n_shinichi's original code revealed **ESP32Servo library was the root cause**. Rewrote everything using manual pulse generation (`pulse_drive`), and **both servos (G0 + G26) now run at equal speed stably**.

<img src="media/servo_test_20260418.jpg" width="400" alt="Both servos running successfully">

<!-- YouTube: Dual servo test -->
[![Dual servo drive test](https://img.youtube.com/vi/syauXEm0TFY/0.jpg)](https://www.youtube.com/watch?v=syauXEm0TFY)

##### Additional issues found and fixed

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 7 | **One servo slow when both connected** | `pulse_drive` was sending 1500μs pulses to OFF servos. Due to neutral offset, these caused micro-rotation and current draw affecting the other servo | Don't send any pulses to OFF servos (`digitalWrite(pin, LOW)` only). Changed `servo_stop()` to stop pulses completely |
| 8 | **Button unresponsive** | `digitalRead(BTN_A)` check was inside 1-second loop — timing mismatch | Moved button handling to 100ms loop + 300ms debounce |

##### Technical lessons learned

- **ESP32Servo vs pulse_drive**: `servo.attach()` occupies LEDC channels and breaks G0 boot. Manual `digitalWrite` pulses are safe for G0
- **Microsecond control**: Use μs-based control (1500μs = stop, 500-2500μs = speed) instead of `servo.write(90)` angle-based
- **Pulse side-effect**: Continuously sending 1500μs "stop" pulses causes servos with neutral offset to micro-rotate. True stop requires no pulses at all

### 📚 PID Control Guide

A beginner-friendly guide explaining PID control basics — from the broom-balancing analogy to inverted pendulum application and parameter tuning. Based on the Interface Magazine article content.

👉 **[docs/pid_guide.md](docs/pid_guide.md)**

### 📐 PID Control Theory (Advanced)

Equations of motion, state-space representation, Kalman filter derivation, stability analysis, and advanced methods (LQR, MPC, RL). For those who want the rigorous math.

👉 **[docs/pid_theory.md](docs/pid_theory.md)**

### References

- [Interface Magazine article](https://interface.cqpub.co.jp/202509st/)
- [n_shinichi's inverted pendulum blog post](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903) — Technical guide and sample sketches for M5StickC Plus / Plus2 by the creator of the Interface Magazine kit
