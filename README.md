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
- **次のステップ**: 新しい M5StickC Plus 入手後、半田付け前にジャンパーワイヤーでサーボ動作確認してからメインファームウェア (`inverted_pendulum.ino`) の書き込み＆倒立テスト

##### 購入が必要な部品

- M5StickC Plus（新品交換）
- デジタルテスター（通電前のショート確認用）

### 参考

- [Interface誌 記事ページ](https://interface.cqpub.co.jp/202509st/)

---

<a id="english"></a>
## English

### Overview

An inverted pendulum (self-balancing robot) project using M5StickC Plus.
Everything from firmware development, compilation, flashing, and debugging was done entirely through conversations with **GitHub Copilot CLI**.

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
- **Next**: After getting a new M5StickC Plus, test servos with jumper wires before soldering, then flash main firmware (`inverted_pendulum.ino`)

### References

- [Interface Magazine article](https://interface.cqpub.co.jp/202509st/)
