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
│
├── servo_diag/
│   └── servo_diag.ino                 ← 🔍 GPIO全ピン自動スキャン診断スケッチ
│
├── servo_calibrate/
│   └── servo_calibrate.ino            ← 🔧 サーボキャリブレーションツール
│
├── motor_dir_test/
│   └── motor_dir_test.ino             ← 🔧 モーター方向テスト / IMU軸ビューアー
│
├── octocat_display/
│   └── octocat_display.ino            ← 🐙 GitHub ステッカースライドショー（息抜き）
│
├── tools/
│   ├── monitor.py                     ← 📊 リアルタイムPIDモニター（シリアル）
│   ├── auto_tune.py                   ← 🔬 自動PIDパラメータスイープ
│   ├── collect_data.py                ← 📝 シリアルデータ収集
│   ├── visualize_root.py              ← 📈 CERN ROOT / matplotlib可視化
│   └── data/                          ← 📂 収集データ・ヒートマップ画像
│
└── docs/
    ├── pid_guide.md                   ← 🎓 PID制御 初心者ガイド
    └── pid_theory.md                  ← 📐 PID制御 理論編（上級者向け）
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
- サーボ動作テスト・ピン自動スキャン・キャリブレーションスケッチの作成
- 自動PIDパラメータスイープツール + ヒートマップ可視化
- リアルタイムシリアルモニター・データ収集ツールの作成
- `arduino-cli` によるコンパイル・書き込みの実行
- IMU軸方向、atan2境界問題、モーター方向のデバッグ
- n_shinichi氏のリファレンス実装との体系的コード比較
- バイリンガル（日英）プロジェクトドキュメントの作成・更新

### ハードウェア

- **マイコン**: M5StickC Plus2 (ESP32-PICO-V3-02, IMU: MPU6886, 電源IC: AXP2101)
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

- [KalmanFilter](https://github.com/TKJElectronics/KalmanFilter) — センサフュージョン
- [M5StickCPlus2](https://github.com/m5stack/M5StickCPlus2) — Plus2用ライブラリ
- ボードマネージャ: M5Stack (ESP32)

### 使い方

1. `arduino-cli` またはArduino IDEでファームウェアを書き込む
2. M5StickC Plus2 を**直立状態**で電源投入（ジャイロキャリブレーション）
3. Aボタン押下 → 倒立制御スタート/ストップ
4. シリアルコマンド（115200bps）でリアルタイム調整可能:
   - `kp=6.3` `kd=0.48` `ki=1.4` — PIDパラメータ変更
   - `po=0.0` — Pitch_offset変更
   - `on` / `off` — モーター制御
   - `?` — 現在値表示

### PIDデフォルト値

```cpp
float kpower = 0.003;
float kp = 6.3;
float ki = 1.4;
float kd = 0.48;
float kspd = 5.0;
float kdst = 0.14;
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

#### 2026-04-18: 方向制御の確立 — IMU軸の特定と全制御系の完成 🎯

前日の pulse_drive 成功に続き、PID制御の方向を正しく設定。**多くの試行錯誤**の末、全制御系が正しく動作するに至った。

##### 解決した問題と得られた知見

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 9 | **ONにしてもモーターが動かない** | `PITCH_OFFSET=81.0` の定数が残っていて Angle が常に大きな値 → ANGLE_LIMIT 超過で PID が動かない | PITCH_OFFSET 定数を削除。ON にした瞬間の Pitch を基準値として自動設定 |
| 10 | **ボタンが反応しない** | `digitalRead(BTN_A)` が1秒ループ内にあり、タイミング不一致 | 100ms ループに移動 + 300ms デバウンス |
| 11 | **数秒でOFFになる** | ANGLE_LIMIT 超過時に `motor_sw=0` にリセットしていた | PID リセットのみ行い、motor_sw は ON のまま維持 |
| 12 | **前後どちらに傾けても同じ方向に進む** | `atan2(acc[1], acc[2])` が直立付近（90°付近）では前後の傾きに対して同符号を返す。Plus2 の IMU 取り付け向きでは Y/Z 軸ではなく **Z 軸が前後の傾き方向** | IMU 軸確認ツールで実機テスト → **Z 軸が前後傾き**と特定。`getPitch()` を `asin(-acc[2])` に変更 |
| 13 | **モーター方向がスピンする** | サーボの取り付け向きが不明で、`+power/+power`、`-power/-power`、`+power/-power`、`-power/+power` の4パターンが未確定 | 4パターン方向テストスケッチで実機確認 → **`+power/-power` が前進**と確定 |

##### IMU 軸の特定手順（再現可能な方法）

1. IMU軸確認スケッチ（`motor_dir_test.ino`）を書き込む — 画面に X/Y/Z の加速度をリアルタイム表示
2. ロボットを直立させて各軸の値を確認
3. **前に傾ける** → どの軸が変化するか記録
4. **後ろに傾ける** → 同じ軸が逆符号に変化するか確認
5. その軸を `getPitch()` で使用

本プロジェクトの M5StickC Plus2 では：
- **X 軸**: 変化しない（左右方向）
- **Y 軸**: 少し変化（重力方向）
- **Z 軸**: 大きく変化（**前後方向** — 前=マイナス、後ろ=プラス）

##### 確定した制御パラメータ

```
モーターピン: G0 (左), G26 (右)
モーター方向: powerL = +power, powerR = -power
IMU角度: getPitch() = asin(-acc[2])
ジャイロ: -gyro[2]（Z軸）
電源: BAT端子からサーボ給電
```

- 前後応答テスト → **前に傾けたら前、後ろに傾けたら後ろに移動** ✅
- **次のステップ**: PIDチューニング（kp → kd → ki の順に手動調整）

#### 2026-04-18: 42パターン自動PIDスイープ + ヒートマップ可視化

- `tools/auto_tune.py` で kp × kd のグリッドサーチ（42組み合わせ）を自動実行
- 各パラメータ組で10秒間バランスを試み、安定時間と平均角度を記録
- 結果を `tools/data/autotune_heatmap.png` にヒートマップ可視化
- **発見**: USB ケーブルのテンション（張力）がバランスを大幅に補助していた
  - USB 接続時: 5秒以上安定
  - USB 切断時: 即座に転倒
- 単純なPIDゲイン調整だけでは不十分、ファームウェア構造自体の見直しが必要と判断

#### 2026-04-25: n_shinichi氏コード完全準拠 — クリーンリライト + IMU軸修正 🔧

n_shinichi氏のオリジナルコードと自分のコードを徹底比較し、**10個の差異**を特定。クリーンリライトを実施。

##### 発見された重大な問題

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 14 | **カルマンフィルタのジャイロ軸が間違っていた** | `gyro[2]`（Z軸）を使用していたが、Plus2では**X軸 (`gyro[0]`)が前後回転軸**だった。ジャイロ軸ビューアーで確認 | `gyro[0]` に変更。**最も影響の大きかった修正** |
| 15 | **getPitch() の atan2 境界問題** | `atan2(-acc[2], -acc[1])` では直立位置が -90°（atan2 の ±90° 境界付近）にあり、前後どちらに傾けても同符号の角度が返る | `atan2(-acc[1], acc[2])` に変更。直立 = 0° で ±90° 境界から遠い安定領域に移動 |
| 16 | **Pitch_offset が動的だった** | ON にした瞬間の Pitch_filter を保存する方式。タイミングで値が変わり不安定 | n_shinichi氏に準拠し固定値に変更 |
| 17 | **ANGLE_LIMIT の判定が間違った変数** | Pitch_filter（オフセット込み）をチェックしていて、Angle（オフセット差引後）をチェックしていなかった | `Angle` で判定するように修正 |

##### atan2 境界問題の詳細（重要な教訓）

Plus2 のIMUでは直立時の加速度値が `acc[1]≈0, acc[2]≈1` で、以下のように atan2 の引数によって結果が大きく異なる：

```
            直立時の値     前傾5°    後傾5°     問題
atan2(-acc[2], -acc[1])  -89.8°    -95.7°    -84.3°    ← 直立が±90°境界！大傾斜で符号反転
atan2(acc[1], acc[2])      0.0°     +5.7°     -5.7°    ← 符号は正しいがジャイロと逆向き
atan2(-acc[1], acc[2])     0.0°     -5.7°     +5.7°    ← ✅ 直立=0°, 符号・ジャイロ一致
```

**結論**: IMU軸の向きに合わせて、**直立位置が atan2 の 0° 付近**になる引数の組み合わせを選ぶこと。±90° 境界付近に直立があると、大きな傾斜で符号が反転して制御不能になる。

##### 小角度での制御成功データ（シリアルログ）

以下はUSB接続状態でのシリアルログ。**±2° 以内のマイクロバランス**が動作していることを確認：

```
Angle= +0.5  power=  -1    ← ほぼ直立、微調整
Angle= +1.4  power= -12
Angle= +0.8  power=  -3
Angle= +0.5  power= +12
Angle= +0.9  power= +10
Angle= +0.4  power= +19
Angle= +0.2  power=  +7
Angle= -0.2  power= +15    ← 後傾→前進で補正
Angle= +1.2  power= -42
Angle= +1.2  power= -16
Angle= +0.1  power=  -4
```

##### 現在の確定パラメータ

```
モーターピン: G0 (左), G26 (右)
モーター方向: powerL = +power, powerR = -power
角度算出: getPitch() = atan2(-acc[1], acc[2])  ← 直立=0°
カルマンフィルタ: gyro[0]（X軸）
dAngle: gyro[0] - gyroOffset[0]
PID符号: P=-kp*A, I+=-ki*A, D=-kd*dA（旧コードとの互換のため反転）
ANGLE_LIMIT: ±20°（atan2境界を回避）
PID: kp=6.3, ki=1.4, kd=0.48, kspd=5.0, kdst=0.14
Pitch_offset: 0.0（atan2版では直立≈0°）
```

##### 残タスク

- [ ] atan2(-acc[1], acc[2]) + ジャイロ符号の最終検証
- [ ] USB切断状態でのバランステスト
- [ ] PID微調整（auto_tune.py + ヒートマップ）
- [ ] CERN ROOT による時系列データ可視化

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
│
├── servo_test/
│   └── servo_test.ino                 ← 🔧 Servo test sketch
│
├── servo_diag/
│   └── servo_diag.ino                 ← 🔍 GPIO auto-scan diagnostic sketch
│
├── servo_calibrate/
│   └── servo_calibrate.ino            ← 🔧 Servo calibration tool
│
├── motor_dir_test/
│   └── motor_dir_test.ino             ← 🔧 Motor direction test / IMU axis viewer
│
├── octocat_display/
│   └── octocat_display.ino            ← 🐙 GitHub sticker slideshow (fun break)
│
├── tools/
│   ├── monitor.py                     ← 📊 Real-time PID monitor (serial)
│   ├── auto_tune.py                   ← 🔬 Automatic PID parameter sweep
│   ├── collect_data.py                ← 📝 Serial data collection
│   ├── visualize_root.py              ← 📈 CERN ROOT / matplotlib visualization
│   └── data/                          ← 📂 Collected data & heatmap images
│
└── docs/
    ├── pid_guide.md                   ← 🎓 PID Control — Beginner's Guide
    └── pid_theory.md                  ← 📐 PID Control — Theory (Advanced)
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
- Created servo test, pin auto-scan diagnostic, and calibration sketches
- Built automatic PID parameter sweep tool with heatmap visualization
- Created real-time serial monitoring and data collection tools
- Compiled and flashed firmware via `arduino-cli`
- Debugged IMU axis orientation, atan2 boundary issues, and motor direction
- Performed systematic code comparison with n_shinichi's reference implementation
- Created and maintained bilingual project documentation

### Hardware

- **MCU**: M5StickC Plus2 (ESP32-PICO-V3-02, IMU: MPU6886, Power IC: AXP2101)
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

- [KalmanFilter](https://github.com/TKJElectronics/KalmanFilter) — Sensor fusion
- [M5StickCPlus2](https://github.com/m5stack/M5StickCPlus2) — Plus2 library
- Board Manager: M5Stack (ESP32)

### Usage

1. Flash firmware using `arduino-cli` or Arduino IDE
2. Power on M5StickC Plus2 in **upright position** (gyro calibration)
3. Press A button → start/stop balance control
4. Real-time tuning via serial commands (115200bps):
   - `kp=6.3` `kd=0.48` `ki=1.4` — change PID parameters
   - `po=0.0` — change Pitch_offset
   - `on` / `off` — motor control
   - `?` — show current values

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

#### 2026-04-18: Direction control established — IMU axis identification & full control system 🎯

Following the pulse_drive success, established correct PID control direction through extensive trial and error.

##### Issues resolved and lessons learned

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 9 | **Motors don't move when ON** | `PITCH_OFFSET=81.0` constant caused Angle to always exceed ANGLE_LIMIT → PID never activated | Removed constant. Auto-set Pitch reference when turning ON |
| 10 | **Button unresponsive** | `digitalRead` in 1-second loop | Moved to 100ms loop + debounce |
| 11 | **Auto-OFF after seconds** | ANGLE_LIMIT exceeded → `motor_sw=0` | Only reset PID, keep motor ON |
| 12 | **Same direction regardless of tilt** | `atan2(acc[1], acc[2])` returns same sign at ~90° (upright). On Plus2, **Z-axis is the forward/backward tilt axis** | Built IMU axis viewer tool → identified Z-axis. Changed `getPitch()` to `asin(-acc[2])` |
| 13 | **Motors spin (opposite directions)** | Unknown servo mounting orientation | Built 4-pattern direction test → confirmed **`+power/-power` = forward** |

##### How to identify the correct IMU axis (reproducible method)

1. Flash the IMU axis viewer sketch — displays X/Y/Z accelerometer values on screen
2. Hold robot upright and note values
3. **Tilt forward** → which axis changes?
4. **Tilt backward** → does the same axis change with opposite sign?
5. Use that axis in `getPitch()`

For this M5StickC Plus2 build:
- **X-axis**: No change (left/right)
- **Y-axis**: Slight change (gravity)
- **Z-axis**: Large change (**forward/backward** — forward=minus, backward=plus)

##### Confirmed control parameters

```
Motor pins: G0 (left), G26 (right)
Motor direction: powerL = +power, powerR = -power
IMU angle: getPitch() = asin(-acc[2])
Gyro: -gyro[2] (Z-axis)
Power: BAT pin for servo supply
```

- Forward/backward response test → **tilt forward=move forward, tilt back=move back** ✅
- **Next**: PID tuning (kp → kd → ki manual adjustment)

#### 2026-04-18: 42-pattern automatic PID sweep + heatmap visualization

- Ran kp × kd grid search (42 combinations) using `tools/auto_tune.py`
- Each parameter set tested for 10 seconds, recording stability time and mean angle
- Results visualized as heatmap in `tools/data/autotune_heatmap.png`
- **Discovery**: USB cable tension was significantly assisting balance
  - USB connected: stable 5+ seconds
  - USB disconnected: immediate fall
- Concluded simple PID gain tuning insufficient — firmware architecture review needed

#### 2026-04-25: Full alignment to n_shinichi's code — Clean rewrite + IMU axis fix 🔧

Performed thorough diff analysis between n_shinichi's original code and ours, identifying **10 differences**. Executed a clean rewrite.

##### Critical issues discovered

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 14 | **Wrong gyro axis for Kalman filter** | Using `gyro[2]` (Z-axis) but on Plus2, **X-axis (`gyro[0]`) is the forward/backward rotation axis**. Confirmed with gyro axis viewer | Changed to `gyro[0]`. **Most impactful fix** |
| 15 | **atan2 boundary problem in getPitch()** | `atan2(-acc[2], -acc[1])` placed upright at -90° (near atan2's ±90° boundary). Both forward and backward tilt returned the same sign | Changed to `atan2(-acc[1], acc[2])`. Upright = 0°, far from ±90° boundary |
| 16 | **Dynamic Pitch_offset** | Captured Pitch_filter at the moment of turning ON — inconsistent timing | Changed to fixed value following n_shinichi |
| 17 | **ANGLE_LIMIT checking wrong variable** | Checking Pitch_filter (with offset) instead of Angle (offset-subtracted) | Fixed to check `Angle` |

##### atan2 boundary problem in detail (key lesson)

On Plus2's IMU, upright accelerometer values are `acc[1]≈0, acc[2]≈1`. The choice of atan2 arguments dramatically affects behavior:

```
                          Upright   Fwd 5°    Back 5°   Issue
atan2(-acc[2], -acc[1])   -89.8°   -95.7°    -84.3°    ← Upright at ±90° boundary! Sign reversal at large tilts
atan2(acc[1], acc[2])       0.0°    +5.7°     -5.7°    ← Correct signs but opposite to gyro direction
atan2(-acc[1], acc[2])      0.0°    -5.7°     +5.7°    ← ✅ Upright=0°, signs match gyro
```

**Lesson**: Choose atan2 arguments so that **upright is near 0°** in atan2 space. If upright is near ±90°, large tilts cause sign reversal and loss of control.

##### Small-angle control success data (serial log)

Serial log with USB connected. Confirmed **micro-balance within ±2°** is working:

```
Angle= +0.5  power=  -1    ← Near-upright, micro-adjustment
Angle= +1.4  power= -12
Angle= +0.8  power=  -3
Angle= +0.5  power= +12
Angle= +0.9  power= +10
Angle= +0.4  power= +19
Angle= +0.2  power=  +7
Angle= -0.2  power= +15    ← Backward tilt → forward correction
Angle= +1.2  power= -42
Angle= +1.2  power= -16
Angle= +0.1  power=  -4
```

##### Current confirmed parameters

```
Motor pins: G0 (left), G26 (right)
Motor direction: powerL = +power, powerR = -power
Angle calc: getPitch() = atan2(-acc[1], acc[2])  ← upright=0°
Kalman filter: gyro[0] (X-axis)
dAngle: gyro[0] - gyroOffset[0]
PID signs: P=-kp*A, I+=-ki*A, D=-kd*dA (inverted for legacy compat)
ANGLE_LIMIT: ±20° (avoids atan2 boundary)
PID: kp=6.3, ki=1.4, kd=0.48, kspd=5.0, kdst=0.14
Pitch_offset: 0.0 (upright ≈ 0° with atan2 version)
```

##### Remaining tasks

- [ ] Final verification of atan2(-acc[1], acc[2]) + gyro sign
- [ ] Balance test without USB cable
- [ ] PID fine-tuning (auto_tune.py + heatmap)
- [ ] Time-series data visualization with CERN ROOT

### 📚 PID Control Guide

A beginner-friendly guide explaining PID control basics — from the broom-balancing analogy to inverted pendulum application and parameter tuning. Based on the Interface Magazine article content.

👉 **[docs/pid_guide.md](docs/pid_guide.md)**

### 📐 PID Control Theory (Advanced)

Equations of motion, state-space representation, Kalman filter derivation, stability analysis, and advanced methods (LQR, MPC, RL). For those who want the rigorous math.

👉 **[docs/pid_theory.md](docs/pid_theory.md)**

### References

- [Interface Magazine article](https://interface.cqpub.co.jp/202509st/)
- [n_shinichi's inverted pendulum blog post](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903) — Technical guide and sample sketches for M5StickC Plus / Plus2 by the creator of the Interface Magazine kit
