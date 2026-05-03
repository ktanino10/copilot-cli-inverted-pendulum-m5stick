# 進捗ログ・実験ログ — Progress Log

> このファイルは README.md から切り出された詳細な実験記録です。
> M5StickC Plus2 倒立振子プロジェクトを 2026-04-13 から 2026-05-02 まで日々進めた、半田付け→ファーム移植→ハードウェアトラブル→IMU軸特定→PIDチューニング→GitHub Pages デモ化…までの試行錯誤の記録。
>
> This file is the detailed experiment log extracted from README.md.
> A daily record of the M5StickC Plus2 inverted pendulum project from 2026-04-13 to 2026-05-02 — soldering → firmware port → hardware troubles → IMU axis identification → PID tuning → GitHub Pages demo, etc.
>
> 📖 [README に戻る / Back to README](../README.md)
>
> 🇯🇵 [日本語](#japanese) ・ 🇬🇧 [English](#english)

---

<a id="japanese"></a>
## 日本語

### 2026-04-13: 半田付け完了・サーボ動作確認

- キット基板へのサーボ半田付けを完了
- サーボ動作テスト (`servo_test.ino`) およびピン自動スキャン診断を実施
- 配線修正後、サーボの動作を確認 ✅
- 再半田付け中に M5StickC Plus が過熱・故障 → 基板ショートによる損傷の可能性大
- USB 接続するだけで発熱し画面表示なし、復旧不可と判断 → **M5StickC Plus 交換待ち**

### 2026-04-16: M5StickC Plus2 で復活・ファームウェア移植完了

- M5StickC Plus の代替として **M5StickC Plus2** を入手
- Plus → Plus2 の移植で判明した差異と解決策を以下にまとめる（⚠️ 同じキットで Plus2 を使う人向け）

#### Plus → Plus2 移植時のトラブルと対策

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 1 | **画面が表示されない** | `M5.begin()` + `M5.Lcd` は Plus 専用。Plus2 では API が異なる | `M5StickCPlus2.h` をインクルードし、`StickCP2.begin(cfg)` + `StickCP2.Display` を使用 |
| 2 | **サーボが回らない（PWM出力はOK）** | Plus2 はデフォルトで5V外部出力が無効。HATコネクタから5Vが出ていなかった | `auto cfg = M5.config(); cfg.output_power = true;` で5V出力を有効化 |
| 3 | **ボタンを押しても制御が止まらない** | Plus の `M5.BtnA.wasPressed()` / `wasClicked()` が Plus2 で期待通りに動作しない | `StickCP2.BtnA.wasReleased()` を使用（ボタンを離した瞬間にトグル） |
| 4 | **IDLEに切り替わるがモーターが止まらない** | `servo.write(90)` だけでは FS90R が完全停止しないことがある（PWM信号が残る） | `servo.detach()` でPWM信号を完全切断。再開時に `servo.attach()` で再接続 |
| 5 | **G0 に繋いだサーボが動かない** | Plus2 では HATコネクタが 8ピン→18ピンに変更され、G0 のピン位置が異なる | サーボピンを G0 → **G25** に変更（`SERVO1_PIN 25`） |
| 6 | **片方のサーボだけ回らない / 回転速度が左右で大きく異なる** | FS90R の個体差でニュートラル点（停止位置）が `write(90)` からずれている。今回の個体は停止点が約75°付近にあり、小さい制御出力だと停止域に入って回らなかった | `SERVO2_TRIM = -15` でニュートラル補正。`servo.write(90 + TRIM - cmd)` で停止点を調整。正しい値は個体ごとに異なるため、IDLE状態でサーボが静止する値を探して設定する |

#### Plus と Plus2 のハードウェア・API 対応表

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

### 2026-04-16: 息抜き — GitHub ステッカースライドショー 🐙

倒立制御のデバッグの合間に、M5StickC Plus2 の画面に GitHub ステッカーを表示するスケッチを作成。Aボタンで Octocat → Copilot → Duck → Mascot を切り替え表示できる。

- PNG/GIF 画像を Python（Pillow）で 110×110 に縮小し、RGB565 ビットマップに変換
- **バイトオーダーの罠**: M5GFX（LovyanGFX）の `pushImage` はビッグエンディアン順の RGB565 を期待する。ESP32 はリトルエンディアンなので、各ピクセルの上位・下位バイトをスワップしないと色が崩れる
- スケッチ: `octocat_display/octocat_display.ino`

### 2026-04-18: 完全やり直し — pulse_drive 方式で両輪駆動成功 🎉

前回（4/16）の試行錯誤を踏まえ、ゼロからやり直し。n_shinichi氏のオリジナル実装を調査した結果、**ESP32Servo ライブラリが根本原因**と判明。手動パルス生成（`pulse_drive`）方式に全面書き換えし、**G0 + G26 の両サーボが同じ勢いで安定駆動に成功**。

<!-- YouTube: 両輪サーボ駆動テスト -->
[![両輪サーボ駆動テスト](https://img.youtube.com/vi/syauXEm0TFY/0.jpg)](https://www.youtube.com/watch?v=syauXEm0TFY)

#### 判明した追加の問題と対策

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 7 | **両サーボ接続時に片方だけ遅い** | `pulse_drive` が OFF のサーボにも1500μsパルスを出し続けていた。個体差でニュートラルがずれているサーボが微回転し、電流を消費して他方に影響 | OFF のサーボにはパルスを一切出さない（`digitalWrite(pin, LOW)` のまま）。`servo_stop()` もパルスなしに変更 |
| 8 | **ボタンが反応しない** | `digitalRead(BTN_A)` のチェックが1秒に1回のループ内にあり、タイミングが合わないと検出できなかった | ボタン処理を100msループに移動 + 300msデバウンス追加 |

#### 技術的な学び

- **ESP32Servo vs pulse_drive**: `servo.attach()` は LEDC チャネルを占有し G0 のブート問題を起こす。`digitalWrite` による手動パルスなら G0 も安全に使える
- **マイクロ秒制御**: `servo.write(90)` の角度指定ではなく、1500μs = 停止、500-2500μs = 速度制御のμsベースが正確
- **パルス制御の副作用**: 停止のつもりで1500μsパルスを出し続けると、ニュートラルずれのあるサーボが微回転する。完全停止にはパルスを止める必要がある

### 2026-04-18: 方向制御の確立 — IMU軸の特定と全制御系の完成 🎯

前日の pulse_drive 成功に続き、PID制御の方向を正しく設定。**多くの試行錯誤**の末、全制御系が正しく動作するに至った。

#### 解決した問題と得られた知見

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 9 | **ONにしてもモーターが動かない** | `PITCH_OFFSET=81.0` の定数が残っていて Angle が常に大きな値 → ANGLE_LIMIT 超過で PID が動かない | PITCH_OFFSET 定数を削除。ON にした瞬間の Pitch を基準値として自動設定 |
| 10 | **ボタンが反応しない** | `digitalRead(BTN_A)` が1秒ループ内にあり、タイミング不一致 | 100ms ループに移動 + 300ms デバウンス |
| 11 | **数秒でOFFになる** | ANGLE_LIMIT 超過時に `motor_sw=0` にリセットしていた | PID リセットのみ行い、motor_sw は ON のまま維持 |
| 12 | **前後どちらに傾けても同じ方向に進む** | `atan2(acc[1], acc[2])` が直立付近（90°付近）では前後の傾きに対して同符号を返す。Plus2 の IMU 取り付け向きでは Y/Z 軸ではなく **Z 軸が前後の傾き方向** | IMU 軸確認ツールで実機テスト → **Z 軸が前後傾き**と特定。`getPitch()` を `asin(-acc[2])` に変更 |
| 13 | **モーター方向がスピンする** | サーボの取り付け向きが不明で、`+power/+power`、`-power/-power`、`+power/-power`、`-power/+power` の4パターンが未確定 | 4パターン方向テストスケッチで実機確認 → **`+power/-power` が前進**と確定 |

#### IMU 軸の特定手順（再現可能な方法）

1. IMU軸確認スケッチ（`motor_dir_test.ino`）を書き込む — 画面に X/Y/Z の加速度をリアルタイム表示
2. ロボットを直立させて各軸の値を確認
3. **前に傾ける** → どの軸が変化するか記録
4. **後ろに傾ける** → 同じ軸が逆符号に変化するか確認
5. その軸を `getPitch()` で使用

本プロジェクトの M5StickC Plus2 では：
- **X 軸**: 変化しない（左右方向）
- **Y 軸**: 少し変化（重力方向）
- **Z 軸**: 大きく変化（**前後方向** — 前=マイナス、後ろ=プラス）

#### 確定した制御パラメータ

```
モーターピン: G0 (左), G26 (右)
モーター方向: powerL = +power, powerR = -power
IMU角度: getPitch() = asin(-acc[2])
ジャイロ: -gyro[2]（Z軸）
電源: BAT端子からサーボ給電
```

- 前後応答テスト → **前に傾けたら前、後ろに傾けたら後ろに移動** ✅
- **次のステップ**: PIDチューニング（kp → kd → ki の順に手動調整）

### 2026-04-18: 42パターン自動PIDスイープ + ヒートマップ可視化

- `tools/auto_tune.py` で kp × kd のグリッドサーチ（42組み合わせ）を自動実行
- 各パラメータ組で10秒間バランスを試み、安定時間と平均角度を記録
- 結果を `tools/data/autotune_heatmap.png` にヒートマップ可視化
- **発見**: USB ケーブルのテンション（張力）がバランスを大幅に補助していた
  - USB 接続時: 5秒以上安定
  - USB 切断時: 即座に転倒
- 単純なPIDゲイン調整だけでは不十分、ファームウェア構造自体の見直しが必要と判断

### 2026-04-25: n_shinichi氏コード完全準拠 — クリーンリライト + IMU軸修正 🔧

n_shinichi氏のオリジナルコードと自分のコードを徹底比較し、**10個の差異**を特定。クリーンリライトを実施。

#### 発見された重大な問題

| # | 問題 | 原因 | 対策 |
|---|------|------|------|
| 14 | **カルマンフィルタのジャイロ軸が間違っていた** | `gyro[2]`（Z軸）を使用していたが、Plus2では**X軸 (`gyro[0]`)が前後回転軸**だった。ジャイロ軸ビューアーで確認 | `gyro[0]` に変更。**最も影響の大きかった修正** |
| 15 | **getPitch() の atan2 境界問題** | `atan2(-acc[2], -acc[1])` では直立位置が -90°（atan2 の ±90° 境界付近）にあり、前後どちらに傾けても同符号の角度が返る | `atan2(-acc[1], acc[2])` に変更。直立 = 0° で ±90° 境界から遠い安定領域に移動 |
| 16 | **Pitch_offset が動的だった** | ON にした瞬間の Pitch_filter を保存する方式。タイミングで値が変わり不安定 | n_shinichi氏に準拠し固定値に変更 |
| 17 | **ANGLE_LIMIT の判定が間違った変数** | Pitch_filter（オフセット込み）をチェックしていて、Angle（オフセット差引後）をチェックしていなかった | `Angle` で判定するように修正 |

#### atan2 境界問題の詳細（重要な教訓）

Plus2 のIMUでは直立時の加速度値が `acc[1]≈0, acc[2]≈1` で、以下のように atan2 の引数によって結果が大きく異なる：

```
            直立時の値     前傾5°    後傾5°     問題
atan2(-acc[2], -acc[1])  -89.8°    -95.7°    -84.3°    ← 直立が±90°境界！大傾斜で符号反転
atan2(acc[1], acc[2])      0.0°     +5.7°     -5.7°    ← 符号は正しいがジャイロと逆向き
atan2(-acc[1], acc[2])     0.0°     -5.7°     +5.7°    ← ✅ 直立=0°, 符号・ジャイロ一致
```

**結論**: IMU軸の向きに合わせて、**直立位置が atan2 の 0° 付近**になる引数の組み合わせを選ぶこと。±90° 境界付近に直立があると、大きな傾斜で符号が反転して制御不能になる。

#### 小角度での制御成功データ（シリアルログ）

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

#### 現在の確定パラメータ

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

#### 残タスク

- [ ] atan2(-acc[1], acc[2]) + ジャイロ符号の最終検証
- [ ] USB切断状態でのバランステスト
- [ ] PID微調整（auto_tune.py + ヒートマップ）
- [ ] CERN ROOT による時系列データ可視化

### 2026-05-01: 前後符号の再検証と「後ろへ逃げる」問題のログ化

4/30深夜〜5/1にかけて、前後の角度符号・PID・モーター出力を実機ログで再検証した。最終的に、**acc[2] の偏差を使う線形角度**で前後の符号が分かれることを確認した。

#### 今日わかったこと

| 項目 | 結論 |
|------|------|
| 角度軸 | `acc[2]` の変化が前後の分離に効く。`acc[1]` は前後どちらでも同方向に動く場面があり、単独では不安定 |
| 角度式 | 現在は `getPitch() = (acc[2] - 1.0) * 57.3`。物理角度そのものではないが、制御用の符号・偏差として利用 |
| ゼロ基準 | `on` / AボタンON時に `Pitch_offset = Pitch_filter` として、今の姿勢を自動で `Angle=0` にする |
| 詳細ログ | `X,...` 行を追加し、`Angle,dAngle,P,I,D,Speed,power,L/R,acc,gyro` をCSV保存できるようにした |
| I項・速度項 | `ki=0` でも `kspd/kpower` を入れると激しく振動しやすい。現時点ではOFF |
| 最低出力 | `minp=80` はFS90R不感帯対策として試したが、段差的に効いて振動源になりやすい。現時点ではOFF |
| 前後非対称 | 後ろへ逃げやすいため、`pneg`（後退補正）を前進側より強くする方向は効果あり |

#### 保存した転倒ログ

以下のCSVは、次回の解析用に保存した。

```
tools/data/failure_log_latest.csv        # ki=0 でも kdst/kspd が残って走り込みを増幅したログ
tools/data/failure_log_oscillation.csv   # D項・出力飽和が強く、小刻み振動→ANGLE_LIMIT超過したログ
tools/data/failure_log_backward_run.csv  # 後ろへ逃げるが、飽和は少なく補正不足寄りだったログ
```

#### 転倒ログの可視化（PNG表示）

`tools/visualize_root.py` を詳細ログ形式に対応させ、各ログからPNGを生成した。READMEではクリックしなくても見えるように、PNG画像を横幅いっぱいに表示する。

```
python3 tools/visualize_root.py tools/data/failure_log_*.csv
```

##### 後ろへ逃げるログ

`failure_log_backward_run.csv`

Angle は小さい範囲に留まる一方、power が負側に偏り続ける。補正方向は合っているが、後退方向の補正不足またはサーボ低速域の効き不足が疑われる。

<p align="center">
  <img src="tools/data/failure_log_backward_run.png" alt="後ろへ逃げるログの可視化" width="100%">
</p>

##### kspd/kdst が残って走り込むログ

`failure_log_latest.csv`

`ki=0` でも速度・距離系が残り、走り込みを増幅していた時期のログ。

<p align="center">
  <img src="tools/data/failure_log_latest.png" alt="速度距離系が残って走り込むログの可視化" width="100%">
</p>

##### 小刻み振動・飽和ログ

`failure_log_oscillation.csv`

D項・出力が強く、power が飽和してから Angle が ±45° を超え、制御停止する様子が見える。

<p align="center">
  <img src="tools/data/failure_log_oscillation.png" alt="小刻み振動と出力飽和ログの可視化" width="100%">
</p>

可視化パネルには以下を含める：

- Angle vs Time（±45°のANGLE_LIMIT線つき）
- Motor Output vs Time（`power`, `L-1500`, `R-1500`）
- PID Terms（P/I/D）
- Phase plot（Angle vs dAngle）
- IMU Signals（accZ / gyroX）
- Angle vs Power

`.root` ファイルも同じディレクトリに保存しているが、READMEでは見やすさ優先でPNGのみ表示する。

#### 現在のファームウェア設定（次回の出発点）

```
getPitch(): (acc[2] - 1.0) * 57.3
ON時: Pitch_offset = Pitch_filter（自動ゼロ合わせ）
ANGLE_LIMIT: ±45°

kp=10.0
ki=0.0
kd=0.35
kpower=0.0
kspd=0.0
kdst=0.0

power_limit=350
power_pos_scale=0.85   # +power = 前進補正
power_neg_scale=1.70   # -power = 後退補正
min_drive_power=0      # 最低出力補正OFF
targetBias=0.0
```

#### 実機での観察

- 設定によっては **3〜4秒程度立つ** ところまで改善。
- ただし安定点には入らず、少し振動しながら、ある傾きから前または後ろへ走りながら倒れる。
- 後ろへ逃げる傾向が比較的多い。
- `kspd=1.2` や `minp=80` など段差・速度系を入れると激しい振動が増えた。
- `pneg=1.70` は立つ時間を伸ばしたが、完全には止まらない。

#### 次回の候補

1. `failure_log_backward_run.csv` を可視化し、後ろへ逃げる直前の `Angle/dAngle/P/D/power` の位相を確認する。
2. `pneg` を大きくする前に、`power_neg_scale` ではなく **後退側だけの小さな `bias`** または **左右/前後のサーボオフセット**を試す。
3. `gyro[0]` のD項ではなく、`Angle` の差分から作る `dAngle` 比較モードを追加し、D項の位相ズレを確認する。
4. `fil_N=5` の遅れを疑い、`fil_N=3` をログ付きで比較する。
5. 自動PIDスイープは、現在の「線形 `acc[2]` + 自動ゼロ合わせ + 前後非対称補正」を基準にやり直す。

### 2026-05-02: チューニングUIの解析強化・ブート演出・GitHub Pagesデモ・どこでも起動

実機のハードはそのままで、**ソフト側を一気に整えた日**。前半はチューニングUIの解析機能と演出、後半は持ち運び対応とウェブデモ化。

#### 1. チューニングUIの統計解析強化（STATS LAB）

> 「期待姿勢が緑（|∠|≤5°）を保つための最適なパラメータの特定」を統計的に出せるようにしたい、という要件。

- **GREEN ZONE バナー** — 全解析結果のサマリにベスト構成を抽出表示
- **STATS LAB パネル** — 2D ヒストグラム / 3D ヒストグラム / KP×KD 散布図 / 複数パラメータ組合せ解析
- **エラーバー / 信頼区間**入りのパラメータ感度プロット
- **score（0〜100）** を新設：
  - `duration·3`（最大30）+ `green%·50`（最大50）+ `max(0, 20−mean|∠|·2)`（最大20）
  - 1テスト1スコア。緑帯滞在時間 + 倒れない時間 + 振れ幅の小ささを統合
- **`⇅ APPLY` ボタン**：過去の好スコア構成のパラメータを **ワンクリックで実機に再投入**（`/api/c?q=k=v` を順に push）
- セッション一覧を `score` 降順で並べてベスト設定が一目でわかる

#### 2. マスコット演出とi18n

| マスコット | 役割 |
|---|---|
| **Mona**（GIF） | リアルタイムATTITUDE表示。安定時はゆったり、転倒の瞬間に高速アニメ。`+/-` の角度に合わせて顔の向きが追従 |
| **Copilot**（GIF） | 解析画面遷移時のヒーロー演出 |
| **Ducky**（GIF） | 言語切替時に画面中央に大きく登場（モード切替の明示） / 通信ロスト時の見守りバナー |

> 途中で「倒れた時のDuckyは要らない」というフィードバックがあり、PWM飽和バッジ・QUICK FALLトースト・診断パネルのリーダー切替など、**転倒に関わるDucky表示はすべて削除**。Duckyは「通信・モード切替」専用キャラに役割整理。

#### 3. ブート演出（SEED風スタートアップ診断）

最初のリロード時に**実プローブ付き**の診断シーケンスが流れるように。各行を1行ずつタイプ表示し、本物の `fetch` で確認して `[ OK ]` / `[ WARN ]` / `[ FAIL ]` を色分け表示：

```
[02:34:15.21] > HUD INTERFACE / DOM            ........ [ OK ]
[02:34:15.45] > NETWORK STACK                  ........ [ OK ]
[02:34:15.71] > TUNING SERVER · LOCAL API      ........ [ OK ]
[02:34:15.98] > WI-FI LINK · M5STICKC PLUS2    ........ [ OK ]   GATEWAY 192.168 · 200 OK
[02:34:16.21] > IMU · MPU6886 GYRO/ACCEL       ........ [ OK ]   ∠=0.42° · LIVE
...
◆ ALL SYSTEMS NOMINAL · LAUNCH
```

固定タイマーではなく**全プローブが完了するまで**閉じない。`WARN` があればアンバー、`FAIL` があれば赤バナーで2.2秒滞留。

#### 4. GitHub Pages デモ化（`docs/demo/`）

ハードを持っていない人にもUIを触ってもらえるように、Flaskなしで動く**完全静的版**をビルド。

- `tools/build_demo.py` で `tools/templates/index.html` から自動生成
- `window.IPS_DEMO = true` フラグを注入し、`fetch` をシムが横取り：
  - `GET /api/s` → シミュレートされた50Hzテレメトリ（PIDパラメータ反映つき）
  - `GET /api/c?q=k=v` → シム内の状態を実際に書き換え（スライダーがちゃんと効く）
  - `GET /api/sessions` → バンドルされた `data/sessions/index.json`
  - `GET /api/sessions/<id>` → 同 `data/sessions/<id>.json`
- 5本の合成セッション（`baseline / kp_too_high / better_kd / pneg_tuned / winner`）を内包し、score 13〜32 の比較が見える
- 左下に紫の **`DEMO MODE`** バッジで明示

#### 5. どこでも起動できるようにする — マルチSSID + APフォールバック

「オフィスに持って行ったとき、どうやって倒立振子を動かす？」への回答。**ファームに2層の安全網**を実装：

```cpp
// (B) 推奨: 複数SSIDを順に試す
#define IPS_WIFI_HAS_LIST
static const IpsWifiNet IPS_WIFI_LIST[] = {
  {"home_2g",       "home_password"},
  {"my_iphone_hot", "phone_password"},   // テザリング
  {"office_guest",  "office_password"},
};
// (どれも圏外なら自動でAPモード: SSID=IPS-CTRL  pass=ips12345)
```

- 起動時に **WiFiスキャン** → 視界にある既知SSIDのみ8秒ずつトライ → 全滅なら **自分自身がアクセスポイント**になり `http://192.168.4.1/` を開放
- LCD下段に **`<SSID> <IP>`** を常時表示。ブラウザで何を打てばいいか迷わない

5つの運用シナリオ（家／AP／スマホテザリング／GitHub Pages）は README §「🌍 持ち運び・遠隔運用」に表で整理。

#### 6. 通信まわりの技術解説（このプロジェクトでの実装）

> 上の機能を理解しておくと、自分で拡張しやすい。

##### (1) 制御UIとM5の通信モデル

```
ブラウザ ──HTTP──▶ Flask(tools/server.py) ──HTTP──▶ M5StickC Plus2
           50ms毎                            50ms毎
   /api/s, /api/c                       /s, /c
```

- **Flaskはただのプロキシ**。CORSやLAN/WiFi切替の差を吸収する薄い中継層。
  - 実装は `tools/server.py` の `requests.get(f"{M5_BASE}/s")` 一行だけ
  - `M5_URL` 環境変数で接続先IPを切替（家・AP・テザリングそれぞれ別IP）
- **M5側はESP32の `WebServer`**：`server.on("/s", handler)` 程度の最小実装で、ステータスJSONとコマンド文字列を返す
- **ポーリング**は50ms周期。WebSocket等のpushではなく単純な `setInterval(tick, 50)` で十分間に合う（人間の反応速度より速いし、ESP32側の処理負荷も小さい）

##### (2) WiFiの3モード — STA / AP / STA+AP

| モード | 役割 | このプロジェクトでの使いどころ |
|---|---|---|
| **STA**（Station） | 既存ルータの**子機**として参加。インターネットアクセス可、ブラウザ→M5は同一LAN必要 | 家・オフィスで普通に使う。`WiFi.begin(ssid, pass)` |
| **AP**（Access Point） | M5自身が**親機**になる。ルータ不要だがインターネットなし | フォールバック。社外プレゼンや電車内など、信用できないWiFi下で完全独立動作 |
| **STA+AP**（Hybrid） | 両方同時。STAでルータに繋ぎつつ、自分もAP（中継機のような動き） | このプロジェクトでは未使用。将来 mDNS応答+遠隔管理を一緒にやりたい時の選択肢 |

切替は `WiFi.mode(WIFI_STA / WIFI_AP / WIFI_AP_STA)`。本プロジェクトはまずSTAで全SSID試行 → 全敗時に `WIFI_AP` に切替 + `WiFi.softAP(SSID, PASS)` を実施。

##### (3) `WiFi.scanNetworks()` で「圏外なのに繋ぎに行く」を回避

`WiFi.begin()` は接続失敗時に8秒前後ブロックする。3つ並べると**最悪24秒待たされる**。先に `scanNetworks()` で電波が見えるSSIDだけに絞ると、家にいるときは1試行で即座に繋がる：

```cpp
int n = WiFi.scanNetworks();
for (each known ssid) {
  bool inRange = false;
  for (int j=0; j<n; j++) if (WiFi.SSID(j) == ssid) inRange = true;
  if (!inRange) continue;     // 電波がない → スキップ
  if (tryConnect(ssid, pass, 8000)) break;
}
```

##### (4) ソフトAPモードの仕様（ESP32）

- パスワードは **8文字以上必須**（短いと `softAP()` が `false` を返す）
- デフォルトIPは **`192.168.4.1`**（DHCPサーバとして 192.168.4.2〜 を配布）
- **インターネットアクセスは無し** — つまりブラウザ側がスマホ等の「モバイルデータ」を併用していない限り外には出られない
- ノートPC側からは「IPS-CTRL」というSSIDが普通のWiFiとして見える

##### (5) `fetch` シム（DEMOモード）の仕組み

GitHub Pagesに置くとFlaskがいないので `/api/*` が全部404になる。ブラウザの `window.fetch` を**起動時に差し替え**て、URLパターンに応じて：

- `/api/s` → JSオブジェクトを `Response` として返す（実際のHTTPは飛ばない）
- `/api/c?q=...` → JS変数を書き換える
- `/api/sessions/...` → 隣接の静的JSONを `_origFetch` で読みに行く

これは**Service Workerなしのモック**。Service Workerを使うとオフラインキャッシュ等もできるが、デモ用途には1ファイルで完結するこの方式が手軽。

#### 7. 今日のコミット

```
4210dac  Mona パレード filter 修正（白すぎ問題）
9184b19  per-session score + APPLY ボタン + 転倒時Ducky除去
c45f4a7  ブート画面: ライブ診断シーケンス
e4a80ce  ブート画面サブタイトルから "SEED" 表記削除
(latest) GitHub Pages デモ + マルチSSID + APフォールバック + 通信解説
```

#### 8. 明日以降の小ネタ候補

- `mDNS` 応答（`pendulum.local`）でIP固定不要に
- BLE経由のワイヤレスシリアル（Wi-Fiが完全にダメな場所用）
- GitHub Actions でデモを自動再ビルド + Pages デプロイ
- セッションのスコア順ランキングをCSVエクスポート

---

### 2026-05-03: 持ち運び対応（Pixel テザリング）と「ネットワーク」を学んだ日 🌐

家でも外（カフェ・オフィス）でも **同じやり方で動かせる構成**を整備した日。

> 💡 **今回の学び**: 倒立振子は「M5（小さなコンピュータ）」と「PC のブラウザ」が同じネットワーク上にいないと喋れない。家では家の Wi-Fi、外では Pixel のテザリング（スマホで作る小さな Wi-Fi）を使うことで「同じネットワーク」を持ち歩ける。

#### 1. なぜテザリング？

カフェやオフィスの公共 Wi-Fi はそもそも繋がらないことが多い：

| よくある罠 | 何が起きるか |
|---|---|
| 企業向け WPA2-Enterprise | ESP32 用の特殊コードが必要 |
| クライアント分離 | 同じ Wi-Fi にいても M5 と PC が直接話せない |
| キャプティブポータル | ブラウザでログインしないと外に出られない |
| MAC アドレス事前登録 | M5 は登録されてないので弾かれる |

→ **スマホのテザリング（Pixel ホットスポット）を使えば、これら全部を回避** できる。スマホが「自分専用の小さな Wi-Fi ルーター」になるイメージ。

#### 2. ファームウェア側の対応 — マルチ SSID

`wifi_config.h` に **複数の Wi-Fi を順番付きで** 登録できるようにした：

```cpp
IPS_WIFI_LIST = {
  {"自宅Wi-Fi",   "..."},   // 1番目に試す
  {"Pixel_4220",  "..."},   // 自宅が見えなければ Pixel
};
```

M5 の起動時に Wi-Fi スキャン → リストの順に試す → 繋がった所で止まる。**家にいれば自宅、外にいれば Pixel と勝手に切り替わる**。

#### 3. 「IP の壁」と解決策（ダッシュボード上の M5 切替ウィジェット）

問題：自宅とテザリングでは M5 に割り当てられる IP アドレスが違う（家庭内 vs スマホが配る IP）。毎回コード書き換え + 再書き込みは現実的でない。

解決：**ダッシュボードの上中央に「M5: ◯◯」ウィジェット** を追加。クリックして M5 の LCD に表示されている IP を入れて APPLY するだけで、サーバ再起動不要で接続先が切り替わる。

#### 4. 重要な気づき — サーバー立ち上げを忘れない

ブラウザだけでは M5 と直接話せない。間に **`tools/server.py`（中継役の Flask サーバ）が必要**。

ターミナルから `ips` で一発起動できるようエイリアス化：

```bash
alias ips='cd ~/workspace/inverted-pendulum-m5stick && (sleep 2 && open http://localhost:5000/) & python3 tools/server.py'
alias ips-kill='lsof -ti :5000 | xargs kill 2>/dev/null && echo "freed port 5000"'
```

→ `ips` で起動、`ips-kill` で停止。

#### 5. 手順早見表

**🏠 家で動かす**

```
1. M5 の電源 ON （LCD に IP 表示）
2. ターミナルで `ips`（前回と同じ IP なのでウィジェット触らなくて OK）
3. 動作確認 → 倒立スタート
```

**🏢 外で動かす**

```
1. M5 の電源 OFF にしてから持ち運ぶ（電源入れたままだと自宅 Wi-Fi に再接続を試み続けるため）
2. 現地で Pixel ホットスポット ON（2.4GHz / Both）
3. PC の Wi-Fi を Pixel に切替
4. M5 の電源 ON → LCD の新しい IP をメモ
5. ターミナルで `ips`
6. ブラウザの上中央「M5:」ウィジェットをクリック → IP 入力 → APPLY
7. 動作確認 → 倒立スタート
```

#### 6. ハマりポイント

- **「PC の Wi-Fi を Pixel に切替忘れ」が一番多い** — M5 は Pixel にいるのに PC が自宅 Wi-Fi だと別ネットワークで通信不可
- **M5 を電源 ON のまま移動させない** — Wi-Fi 切替は起動時のみ実行されるため、転居したら一度 OFF → ON
- **LCD が判断材料** — `192.168.x` 系なら自宅、`10.x.x` 系なら Pixel。これを見れば今どっちにいるか一発で分かる
- **`Address already in use` エラー** → 前回サーバが残ってる → `ips-kill` で解放

#### 7. 学びまとめ（初心者目線）

- ✅ **「ネットワーク」とは「会話できる範囲」** — 同じ Wi-Fi にいないと話せない
- ✅ **スマホは小さな Wi-Fi ルーターになれる** — それがテザリング
- ✅ **IP は引っ越し先で変わる** — でもダッシュボードから書き換えれば良い、コードは触らなくて OK
- ✅ **ブラウザ ⇄ M5 の間にサーバが必要** — `ips` 一発で立ち上がる

---

### 2026-05-03 (PM): 🔗 LINK MONA — ダッシュボードのモナを M5 LCD にネットワーク配信

ダッシュボードのモナ（角度状態で色や動きが変わるマスコット）を、**M5StickC Plus2 の LCD にも同じものを表示** したい、という話。

> 💡 アイデア: GIF を M5 のフラッシュに焼くと容量を食う（mascot.gif は **3.5 MB**）。
> → **PC からネットワーク経由で 1フレームずつ送る** ことで、M5 側の容量消費ゼロでミラー表示できる。

#### 1. 設計

```
[ブラウザ Mona]                [PC: server.py]                 [M5 LCD]
    │                              │                              │
    🔗 LINK ──▶ /api/lcd_link ──▶ ストリーマースレッド開始       │
                                   │                              │
                          200ms 周期 (5 FPS) で:                  │
                          1. mascot.gif の次フレーム取得 (Pillow)  │
                          2. 80×80 にリサイズ                      │
                          3. Angle で 4色枠 (灰/緑/黄/赤) を tint  │
                          4. JPEG エンコード (~2 KB)               │
                          5. POST http://M5/face ───────────────▶  │
                                                                  ▼
                                                   drawJpg() で右側 80×80 に描画
                                                   左側は従来の PID/IP テキスト維持
                                                   5秒受信なし → 自動で全テキスト復帰
```

#### 2. ダッシュボード上の操作

上中央 **「M5: ◯◯」ウィジェット** の隣に **🔓 LINK MONA** ボタンを追加。

- 押すと → **🔗 LINKED** に切り替わって緑点灯、PC が M5 LCD にモナを送り始める
- もう一度押すと → **🔓 LINK MONA** に戻り、ストリーミング停止

「IP を打ち込む途中に勝手に LCD が顔モードに切り替わって混乱する」を避けるため、**LINK は明示的なオプトイン**。普段は今までどおり IP/SSID 表示。

#### 3. M5 LCD レイアウト（LINK 中）

```
┌──────────────────────────────────────┐
│ ON  +1.2                  ┌──────┐  │
│ P=10.0 I=0.0 D=0.4        │      │  │
│ pw=-100 L=1600 R=1400     │ Mona │  │  ← 80×80 ストリーミング
│ po=0 po2=0.0 4.1V         │      │  │
│ HomeWiFi 192.168.10.32    └──────┘  │
└──────────────────────────────────────┘
```

- 左側：従来テキスト（ON/OFF・角度・PID・電源・電池・SSID/IP）すべて維持
- 右側：80×80 のモナ（5 FPS）

#### 4. 状態別の枠色（角度連動）

| `|Angle|` | 状態 | 枠色 | 意味 |
|---|---|---|---|
| OFF | off | 灰色 | 制御停止中 |
| < 5° | calm | 緑 | 安定中（緑帯） |
| 5–15° | warn | 黄 | 揺れ始め |
| ≥ 15° | crit | 赤 | 倒れそう／倒れた（フレームを2倍速で進めて緊迫感） |

ダッシュボードの CSS drop-shadow で表現していた「色変化」を、ネットワーク配信先でも視覚的に再現。

#### 5. フェイルセーフ

- **5秒間 `/face` 受信が途切れる → M5 側で自動的にテキスト全画面表示に戻る**（PC が落ちても LCD が固まらない）
- ストリーマースレッドは daemon 化、PC のサーバを終了（`Ctrl+C`）すれば自動的に止まる
- M5 が圏外でも `requests.post(timeout=0.4)` で短時間で諦め、次の周期に再挑戦

#### 6. 通信負荷

- フレームサイズ：~2 KB（80×80 JPEG quality=70）
- 周期：200ms (5 FPS)
- 帯域：約 **10 KB/s** — テザリングでも余裕
- M5 側 CPU：drawJpg 1回 ~30-50ms。50Hz 制御ループ（10ms周期）に影響しないよう、新フレーム到着時のみ描画

#### 7. 実装ファイル

- `firmware/inverted_pendulum/inverted_pendulum.ino` — `/face` POST ハンドラ・`updateDisplay()` 改修・`g_face_active` タイムアウト復帰
- `tools/server.py` — `/api/lcd_link` エンドポイント・Pillow ベースのストリーマースレッド・状態判定
- `tools/templates/_demo_shim.js` — M5 ウィジェット内に LINK MONA トグルボタン追加（DEMO モードでは描画されないので Pages 側は無影響）

#### 8. 学んだこと

- **GIF を「ファイルとして保存」せず「フレーム単位でストリーミング」** することで、組み込みデバイスの容量制約を回避できる
- **5 FPS でも雰囲気は十分伝わる** — 完全同期は無理だが、状態変化（緑→黄→赤）は1秒以内に反映されるので視覚的フィードバックとして機能する
- **ESP32 `WebServer` の `arg("plain")`** は実は raw POST body（バイナリでも）を取れる便利な仕様

---

<a id="english"></a>
## English

### 2026-04-13: Soldering complete / Servo test

- Completed soldering servos to the kit PCB
- Ran servo tests (`servo_test.ino`) and pin auto-scan diagnostics
- Confirmed servo operation after wiring fix ✅
- M5StickC Plus overheated during re-soldering → likely PCB short damage
- Unit unresponsive on USB connection → **awaiting replacement**

### 2026-04-16: M5StickC Plus2 — firmware ported & working

- Replaced dead M5StickC Plus with **M5StickC Plus2**
- Ported all firmware from Plus to Plus2 — several breaking differences found and resolved

#### Troubleshooting: Plus → Plus2 Migration

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 1 | **Screen blank after flash** | `M5.begin()` + `M5.Lcd` are Plus-only APIs | Use `M5StickCPlus2.h`, `StickCP2.begin(cfg)`, `StickCP2.Display` |
| 2 | **Servo won't spin (PWM signal OK)** | Plus2 disables 5V external output by default — no power on HAT pins | Set `cfg.output_power = true` before `StickCP2.begin(cfg)` |
| 3 | **Button press doesn't stop control** | `wasPressed()` / `wasClicked()` behave differently on Plus2 | Use `StickCP2.BtnA.wasReleased()` — triggers on button release |
| 4 | **Motor keeps spinning after IDLE** | `servo.write(90)` alone doesn't fully stop FS90R (residual PWM) | Call `servo.detach()` to kill PWM signal; `servo.attach()` on restart |
| 5 | **G0 servo doesn't respond** | Plus2 HAT connector changed from 8-pin to 18-pin — G0 pin location differs | Change servo pin from G0 to **G25** (`SERVO1_PIN 25`) |
| 6 | **One servo barely spins / large speed difference between left and right** | FS90R neutral point (stop position) varies per unit. One servo's actual stop point was ~75° instead of 90°, so small PID outputs fell into the dead zone | Add `SERVO2_TRIM = -15` for neutral offset compensation. Use `servo.write(90 + TRIM - cmd)`. Find the correct trim value per servo by checking it stays still at IDLE |

#### Plus vs Plus2 API Reference

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

### 2026-04-16: Fun break — GitHub Sticker Slideshow 🐙

During a debugging break, created a sketch that displays GitHub stickers on the M5StickC Plus2 screen. Press A button to cycle through Octocat → Copilot → Duck → Mascot.

- Converted PNG/GIF images to 110×110 RGB565 bitmaps using Python (Pillow)
- **Byte order gotcha**: M5GFX (LovyanGFX) `pushImage` expects big-endian RGB565, but ESP32 is little-endian. Without byte-swapping each pixel, colors appear corrupted (reds and blues swap)
- Sketch: `octocat_display/octocat_display.ino`

### 2026-04-18: Full restart — Both servos running with pulse_drive 🎉

After the 4/16 struggles, started completely fresh. Research into n_shinichi's original code revealed **ESP32Servo library was the root cause**. Rewrote everything using manual pulse generation (`pulse_drive`), and **both servos (G0 + G26) now run at equal speed stably**.

<!-- YouTube: Dual servo test -->
[![Dual servo drive test](https://img.youtube.com/vi/syauXEm0TFY/0.jpg)](https://www.youtube.com/watch?v=syauXEm0TFY)

#### Additional issues found and fixed

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 7 | **One servo slow when both connected** | `pulse_drive` was sending 1500μs pulses to OFF servos. Due to neutral offset, these caused micro-rotation and current draw affecting the other servo | Don't send any pulses to OFF servos (`digitalWrite(pin, LOW)` only). Changed `servo_stop()` to stop pulses completely |
| 8 | **Button unresponsive** | `digitalRead(BTN_A)` check was inside 1-second loop — timing mismatch | Moved button handling to 100ms loop + 300ms debounce |

#### Technical lessons learned

- **ESP32Servo vs pulse_drive**: `servo.attach()` occupies LEDC channels and breaks G0 boot. Manual `digitalWrite` pulses are safe for G0
- **Microsecond control**: Use μs-based control (1500μs = stop, 500-2500μs = speed) instead of `servo.write(90)` angle-based
- **Pulse side-effect**: Continuously sending 1500μs "stop" pulses causes servos with neutral offset to micro-rotate. True stop requires no pulses at all

### 2026-04-18: Direction control established — IMU axis identification & full control system 🎯

Following the pulse_drive success, established correct PID control direction through extensive trial and error.

#### Issues resolved and lessons learned

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 9 | **Motors don't move when ON** | `PITCH_OFFSET=81.0` constant caused Angle to always exceed ANGLE_LIMIT → PID never activated | Removed constant. Auto-set Pitch reference when turning ON |
| 10 | **Button unresponsive** | `digitalRead` in 1-second loop | Moved to 100ms loop + debounce |
| 11 | **Auto-OFF after seconds** | ANGLE_LIMIT exceeded → `motor_sw=0` | Only reset PID, keep motor ON |
| 12 | **Same direction regardless of tilt** | `atan2(acc[1], acc[2])` returns same sign at ~90° (upright). On Plus2, **Z-axis is the forward/backward tilt axis** | Built IMU axis viewer tool → identified Z-axis. Changed `getPitch()` to `asin(-acc[2])` |
| 13 | **Motors spin (opposite directions)** | Unknown servo mounting orientation | Built 4-pattern direction test → confirmed **`+power/-power` = forward** |

#### How to identify the correct IMU axis (reproducible method)

1. Flash the IMU axis viewer sketch — displays X/Y/Z accelerometer values on screen
2. Hold robot upright and note values
3. **Tilt forward** → which axis changes?
4. **Tilt backward** → does the same axis change with opposite sign?
5. Use that axis in `getPitch()`

For this M5StickC Plus2 build:
- **X-axis**: No change (left/right)
- **Y-axis**: Slight change (gravity)
- **Z-axis**: Large change (**forward/backward** — forward=minus, backward=plus)

#### Confirmed control parameters

```
Motor pins: G0 (left), G26 (right)
Motor direction: powerL = +power, powerR = -power
IMU angle: getPitch() = asin(-acc[2])
Gyro: -gyro[2] (Z-axis)
Power: BAT pin for servo supply
```

- Forward/backward response test → **tilt forward=move forward, tilt back=move back** ✅
- **Next**: PID tuning (kp → kd → ki manual adjustment)

### 2026-04-18: 42-pattern automatic PID sweep + heatmap visualization

- Ran kp × kd grid search (42 combinations) using `tools/auto_tune.py`
- Each parameter set tested for 10 seconds, recording stability time and mean angle
- Results visualized as heatmap in `tools/data/autotune_heatmap.png`
- **Discovery**: USB cable tension was significantly assisting balance
  - USB connected: stable 5+ seconds
  - USB disconnected: immediate fall
- Concluded simple PID gain tuning insufficient — firmware architecture review needed

### 2026-04-25: Full alignment to n_shinichi's code — Clean rewrite + IMU axis fix 🔧

Performed thorough diff analysis between n_shinichi's original code and ours, identifying **10 differences**. Executed a clean rewrite.

#### Critical issues discovered

| # | Problem | Root Cause | Fix |
|---|---------|-----------|-----|
| 14 | **Wrong gyro axis for Kalman filter** | Using `gyro[2]` (Z-axis) but on Plus2, **X-axis (`gyro[0]`) is the forward/backward rotation axis**. Confirmed with gyro axis viewer | Changed to `gyro[0]`. **Most impactful fix** |
| 15 | **atan2 boundary problem in getPitch()** | `atan2(-acc[2], -acc[1])` placed upright at -90° (near atan2's ±90° boundary). Both forward and backward tilt returned the same sign | Changed to `atan2(-acc[1], acc[2])`. Upright = 0°, far from ±90° boundary |
| 16 | **Dynamic Pitch_offset** | Captured Pitch_filter at the moment of turning ON — inconsistent timing | Changed to fixed value following n_shinichi |
| 17 | **ANGLE_LIMIT checking wrong variable** | Checking Pitch_filter (with offset) instead of Angle (offset-subtracted) | Fixed to check `Angle` |

#### atan2 boundary problem in detail (key lesson)

On Plus2's IMU, upright accelerometer values are `acc[1]≈0, acc[2]≈1`. The choice of atan2 arguments dramatically affects behavior:

```
                          Upright   Fwd 5°    Back 5°   Issue
atan2(-acc[2], -acc[1])   -89.8°   -95.7°    -84.3°    ← Upright at ±90° boundary! Sign reversal at large tilts
atan2(acc[1], acc[2])       0.0°    +5.7°     -5.7°    ← Correct signs but opposite to gyro direction
atan2(-acc[1], acc[2])      0.0°    -5.7°     +5.7°    ← ✅ Upright=0°, signs match gyro
```

**Lesson**: Choose atan2 arguments so that **upright is near 0°** in atan2 space. If upright is near ±90°, large tilts cause sign reversal and loss of control.

#### Small-angle control success data (serial log)

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

#### Current confirmed parameters

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

#### Remaining tasks

- [ ] Final verification of atan2(-acc[1], acc[2]) + gyro sign
- [ ] Balance test without USB cable
- [ ] PID fine-tuning (auto_tune.py + heatmap)
- [ ] Time-series data visualization with CERN ROOT

### 2026-05-01: Re-verification of fwd/back signs and logging the "running away backward" problem

From late 4/30 into 5/1, I re-verified the forward/backward angle sign, PID, and motor output with on-device logs. Confirmed that **using the deviation of `acc[2]` as a linear angle** correctly separates fwd/back direction.

#### Findings today

| Item | Conclusion |
|------|------|
| Angle axis | `acc[2]` change works for fwd/back separation. `acc[1]` alone moves in the same direction for both fwd/back tilts in some cases — unstable as sole input |
| Angle formula | Currently `getPitch() = (acc[2] - 1.0) * 57.3`. Not a true physical angle, but used as a deviation/sign for control |
| Zero reference | On `on` / button-A press, set `Pitch_offset = Pitch_filter` so the current pose becomes `Angle=0` automatically |
| Detailed log | Added `X,...` lines so `Angle, dAngle, P, I, D, Speed, power, L/R, acc, gyro` can be saved as CSV |
| I term / Speed term | Even with `ki=0`, adding `kspd/kpower` makes oscillation worse. Currently OFF |
| Min drive power | `minp=80` (FS90R deadband workaround) acts step-wise and seeds oscillation. Currently OFF |
| Fwd/back asymmetry | The bot tends to run away backward. Making `pneg` (backward correction) stronger than forward helps |

#### Saved fall logs

```
tools/data/failure_log_latest.csv        # ki=0 but kdst/kspd left over → amplified runaway
tools/data/failure_log_oscillation.csv   # D-term + saturation strong → micro-oscillation → ANGLE_LIMIT trip
tools/data/failure_log_backward_run.csv  # Backward run, less saturation, more "correction insufficient"
```

#### Visualizations (PNG)

`tools/visualize_root.py` was extended for the detailed log format and produces PNG per CSV. Embedded full-width in README so they're visible without clicking through.

```
python3 tools/visualize_root.py tools/data/failure_log_*.csv
```

The visualization panel includes: Angle vs Time (with ±45° ANGLE_LIMIT line), Motor Output, PID Terms (P/I/D), Phase plot (Angle vs dAngle), IMU Signals (accZ / gyroX), and Angle vs Power. `.root` files are saved in the same directory but README shows PNG only for readability.

#### Current firmware setup (next session's starting point)

```
getPitch(): (acc[2] - 1.0) * 57.3
On 'on': Pitch_offset = Pitch_filter (auto zero)
ANGLE_LIMIT: ±45°

kp=10.0, ki=0.0, kd=0.35
kpower=0.0, kspd=0.0, kdst=0.0

power_limit=350
power_pos_scale=0.85   # +power = forward correction
power_neg_scale=1.70   # -power = backward correction
min_drive_power=0
targetBias=0.0
```

#### Observations on the bench

- With certain settings, **stays up for ~3-4 seconds**.
- But never enters a steady state: it oscillates, then runs forward or backward and falls.
- Backward runaway is more frequent.
- `kspd=1.2` or `minp=80` (step-wise / velocity terms) increased oscillation.
- `pneg=1.70` extended stand time but didn't fully stop runaway.

#### Next candidates

1. Visualize `failure_log_backward_run.csv` and inspect the phase of `Angle/dAngle/P/D/power` right before backward runaway.
2. Before raising `pneg` further, try a **small backward-only `bias`** or per-side servo offset instead of `power_neg_scale`.
3. Add a comparison mode for `dAngle` computed from `Angle` differencing (vs `gyro[0]` D term) to check D-term phase mismatch.
4. Suspect filter delay (`fil_N=5`) — compare against `fil_N=3` with logging.
5. Restart auto-PID-sweep based on the current "linear `acc[2]` + auto-zero + asymmetric correction" baseline.

### 2026-05-02: Tuning UI analytics, boot diag, GitHub Pages demo, and "anywhere launch"

Hardware untouched. **Day to overhaul the software side.** First half: tuning UI analytics and presentation. Second half: portability and web demo.

#### 1. Tuning UI statistical analysis (STATS LAB)

> Requirement: "Statistically identify the optimal parameters that keep the expected pose green (|∠|≤5°)."

- **GREEN ZONE banner** — pulls the best configuration from all analyses into a top summary
- **STATS LAB panel** — 2D histogram / 3D histogram / KP×KD scatter / multi-parameter combinatorial analysis
- **Parameter sensitivity plots** with error bars / confidence intervals
- **`score` (0–100)** introduced:
  - `duration·3` (max 30) + `green%·50` (max 50) + `max(0, 20−mean|∠|·2)` (max 20)
  - One score per test. Fuses time-in-green + survival time + small swing amplitude
- **`⇅ APPLY` button**: re-injects parameters from a high-scoring past run **back into the live device** with a single click (sequentially POSTs `/api/c?q=k=v`)
- Session list sorted descending by `score` so the best setup is obvious at a glance

#### 2. Mascots and i18n

| Mascot | Role |
|---|---|
| **Mona** (GIF) | Real-time ATTITUDE display. Calm when stable, fast animation at the moment of fall. Face direction follows the `±` angle |
| **Copilot** (GIF) | Hero animation when transitioning into the analysis screen |
| **Ducky** (GIF) | Large center-screen entrance on language switch (clear mode change) / link-lost watchdog banner |

> Mid-session feedback: "no Ducky on falls" — so **all fall-related Ducky displays were removed** (PWM saturation badge, QUICK FALL toast, diagnosis panel leader switching, etc.). Ducky is now reserved purely for "communication / mode-switch" events.

#### 3. Boot animation (SEED-style startup diagnostic)

On the first reload, an **actual probe-driven** diagnostic sequence runs. Each line types out one at a time, real `fetch` calls verify, and `[ OK ] / [ WARN ] / [ FAIL ]` is color-coded:

```
[02:34:15.21] > HUD INTERFACE / DOM            ........ [ OK ]
[02:34:15.45] > NETWORK STACK                  ........ [ OK ]
[02:34:15.71] > TUNING SERVER · LOCAL API      ........ [ OK ]
[02:34:15.98] > WI-FI LINK · M5STICKC PLUS2    ........ [ OK ]   GATEWAY 192.168 · 200 OK
[02:34:16.21] > IMU · MPU6886 GYRO/ACCEL       ........ [ OK ]   ∠=0.42° · LIVE
...
◆ ALL SYSTEMS NOMINAL · LAUNCH
```

Not on a fixed timer — **stays open until all probes complete**. Amber banner if any `WARN`, red banner for 2.2 s on `FAIL`.

#### 4. GitHub Pages demo (`docs/demo/`)

So that people without the hardware can play with the UI, built a **fully static version** that runs without Flask.

- Auto-generated by `tools/build_demo.py` from `tools/templates/index.html`
- Injects `window.IPS_DEMO = true`; a shim hijacks `fetch`:
  - `GET /api/s` → simulated 50 Hz telemetry (PID parameters take effect)
  - `GET /api/c?q=k=v` → actually rewrites the shim's internal state (sliders work)
  - `GET /api/sessions` → bundled `data/sessions/index.json`
  - `GET /api/sessions/<id>` → ditto `data/sessions/<id>.json`
- Bundles 5 synthetic sessions (`baseline / kp_too_high / better_kd / pneg_tuned / winner`) — score range 13–32 visible for comparison
- Purple **`DEMO MODE`** badge bottom-left makes the mode explicit

#### 5. Run anywhere — multi-SSID + AP fallback

Answer to "how do I run the pendulum at the office?" — a 2-layer firmware safety net:

```cpp
// (B) recommended: try multiple SSIDs in order
#define IPS_WIFI_HAS_LIST
static const IpsWifiNet IPS_WIFI_LIST[] = {
  {"home_2g",       "home_password"},
  {"my_iphone_hot", "phone_password"},   // tethering
  {"office_guest",  "office_password"},
};
// (if all out of range: auto AP mode SSID=IPS-CTRL  pass=ips12345)
```

- On boot, **WiFi scan** → try only known SSIDs that are in range, 8 s each → if all fail, **become an access point** and serve `http://192.168.4.1/`
- LCD bottom row shows **`<SSID> <IP>`** so you never wonder what URL to type

The 4 operational scenarios (home / AP / phone-tether / GitHub Pages) are tabulated in README §"🌍 Portable / Remote Operation".

#### 6. Networking deep dive (how this project actually does it)

> Useful background for extending the system yourself.

##### (1) Control-UI ↔ M5 communication model

```
Browser ──HTTP──▶ Flask(tools/server.py) ──HTTP──▶ M5StickC Plus2
           every 50 ms                       every 50 ms
   /api/s, /api/c                       /s, /c
```

- **Flask is just a proxy**. A thin relay layer that absorbs CORS and LAN/WiFi target differences.
  - Actually one line: `requests.get(f"{M5_BASE}/s")` in `tools/server.py`
  - `M5_URL` env switches the upstream IP (different IPs at home / AP / tether)
- **M5 side uses ESP32 `WebServer`**: minimal `server.on("/s", handler)` returning JSON status & accepting command strings
- **Polling** every 50 ms. Simple `setInterval(tick, 50)` — no WebSocket needed (faster than human reaction time, and easy on ESP32)

##### (2) WiFi's three modes — STA / AP / STA+AP

| Mode | Role | Where this project uses it |
|---|---|---|
| **STA** (Station) | Joins existing router as a **client**. Internet access OK; browser→M5 must share the LAN | Normal operation at home / office. `WiFi.begin(ssid, pass)` |
| **AP** (Access Point) | M5 itself becomes the **router**. No internet needed | Fallback. Off-site demos, on a train, untrusted WiFi → standalone operation |
| **STA+AP** (Hybrid) | Both at once. Router-connected AND its own AP (relay-like) | Not used here. A future option if combining mDNS responder + remote management |

Switching: `WiFi.mode(WIFI_STA / WIFI_AP / WIFI_AP_STA)`. This project tries STA across all SSIDs first → on total failure, switches to `WIFI_AP` and calls `WiFi.softAP(SSID, PASS)`.

##### (3) `WiFi.scanNetworks()` to avoid "trying to connect to nothing"

`WiFi.begin()` blocks for ~8 s on failure. With three SSIDs that's **24 s worst case**. Pre-filtering with `scanNetworks()` to only the SSIDs that are actually visible means a 1-shot connect at home:

```cpp
int n = WiFi.scanNetworks();
for (each known ssid) {
  bool inRange = false;
  for (int j=0; j<n; j++) if (WiFi.SSID(j) == ssid) inRange = true;
  if (!inRange) continue;     // not visible → skip
  if (tryConnect(ssid, pass, 8000)) break;
}
```

##### (4) ESP32 soft-AP specs

- Password **must be ≥ 8 chars** (otherwise `softAP()` returns `false`)
- Default IP is **`192.168.4.1`** (DHCP serves 192.168.4.2+)
- **No internet** — clients can't reach the outside world unless they layer in their own mobile data
- Laptop sees "IPS-CTRL" as an ordinary WiFi network

##### (5) `fetch` shim (DEMO mode) mechanics

GitHub Pages has no Flask, so all `/api/*` calls would 404. The shim **swaps `window.fetch` at startup** and routes by URL pattern:

- `/api/s` → returns a `Response` from a JS object (no actual HTTP)
- `/api/c?q=...` → mutates JS variables
- `/api/sessions/...` → reads adjacent static JSON via `_origFetch`

This is a **mock without a Service Worker**. Service Workers can offer offline caching too, but for demo use a single self-contained file is simpler.

#### 7. Today's commits

```
4210dac  Mona parade filter fix (too white)
9184b19  per-session score + APPLY button + remove Ducky on falls
c45f4a7  Boot screen: live diagnostic sequence
e4a80ce  Drop "SEED" label from boot subtitle
(latest) GitHub Pages demo + multi-SSID + AP fallback + networking docs
```

#### 8. Small ideas for tomorrow

- `mDNS` response (`pendulum.local`) so we don't pin IPs
- Wireless serial via BLE (for fully WiFi-hostile environments)
- GitHub Actions to auto-rebuild + deploy the demo on push
- Score-ranked CSV export of sessions

---

### 2026-05-03: Portability via Pixel tethering and a day spent learning "networking" 🌐

A day to set up **the same workflow whether at home or out (cafés, the office)**.

> 💡 **Today's lesson**: The pendulum needs the M5 (a small computer) and the PC's browser to be on the same network to talk. At home that's home WiFi. Outside, Pixel's hotspot (a small WiFi the phone makes) becomes a portable "same network" you carry with you.

#### 1. Why tethering?

Public WiFi at cafés / offices often won't even connect:

| Common trap | What happens |
|---|---|
| Enterprise WPA2-Enterprise | Needs special ESP32-side code |
| Client isolation | Even on the same WiFi, M5 ↔ PC can't talk directly |
| Captive portal | You can't go anywhere until you log in via browser |
| MAC pre-registration | M5 isn't registered, so it's blocked |

→ **A phone hotspot (Pixel) bypasses all of these.** Think of the phone as "your own personal mini WiFi router."

#### 2. Firmware side — multi-SSID

`wifi_config.h` now supports **multiple WiFis in priority order**:

```cpp
IPS_WIFI_LIST = {
  {"HomeWiFi",   "..."},   // try first
  {"Pixel_4220", "..."},   // fall back to Pixel if home not visible
};
```

On boot, M5 scans → tries the list in order → stops on first success. **Home WiFi at home, Pixel outside, automatically.**

#### 3. The "IP wall" and the M5-switch widget on the dashboard

Problem: Home WiFi and the tethered hotspot assign different IPs to the M5 (LAN-issued vs phone-issued). Editing code & re-flashing every time is impractical.

Solution: Added an **"M5: ◯◯" widget at the top center** of the dashboard. Click it, paste the IP shown on the M5 LCD, hit APPLY — backend swaps targets without restarting the server.

#### 4. Important reminder — don't forget to start the server

The browser can't talk directly to the M5. **`tools/server.py` (the relay Flask server) sits between them.**

Aliased so a single terminal command launches it:

```bash
alias ips='cd ~/workspace/inverted-pendulum-m5stick && (sleep 2 && open http://localhost:5000/) & python3 tools/server.py'
alias ips-kill='lsof -ti :5000 | xargs kill 2>/dev/null && echo "freed port 5000"'
```

→ `ips` to start, `ips-kill` to stop.

#### 5. Cheat sheet

**🏠 Home**

```
1. Power on M5 (LCD shows the IP)
2. Run `ips` in a terminal (same IP as last time, no widget tweaking needed)
3. Verify, then start the inverted-pendulum control
```

**🏢 Outside**

```
1. Power off M5 before you leave (otherwise it'll keep retrying home WiFi)
2. On site: turn on Pixel hotspot (2.4 GHz / Both)
3. Switch the PC's WiFi to Pixel
4. Power on M5 → note the new IP on the LCD
5. Run `ips` in a terminal
6. Click the "M5:" widget at the top-center of the browser → enter IP → APPLY
7. Verify, then start the inverted-pendulum control
```

#### 6. Gotchas

- **Forgetting to switch the PC's WiFi to Pixel is the #1 mistake** — M5 is on Pixel, PC is still on home WiFi → different networks → no communication
- **Don't move the M5 while powered on** — WiFi switching only happens at boot, so power-cycle after relocating
- **The LCD is your debugger** — `192.168.x.x` = home, `10.x.x.x` = Pixel. Tells you immediately which network you're on
- **`Address already in use`** → previous server still running → run `ips-kill`

#### 7. Beginner takeaways

- ✅ **A "network" is the boundary of conversation** — devices not on the same WiFi can't talk
- ✅ **A phone can be a WiFi router** — that's tethering
- ✅ **IPs change when you "move house"** — but the dashboard widget rewrites it; no need to touch code
- ✅ **A server sits between the browser and the M5** — `ips` brings it up in one shot

### 2026-05-03 (PM): 🔗 LINK MONA — stream the dashboard mascot to the M5 LCD over the network

Goal: take the same Mona (the angle-reactive mascot on the dashboard) and **also show it on the M5StickC Plus2's LCD**.

> 💡 Idea: burning a GIF into the device's flash is wasteful (mascot.gif is **3.5 MB**).
> → **Stream one frame at a time from the PC over the network** — zero storage on the M5, faithful mirror of the dashboard.

#### 1. Design

```
[browser Mona]              [PC: server.py]                  [M5 LCD]
    │                            │                              │
    🔗 LINK ──▶ /api/lcd_link ──▶ start streamer thread          │
                                 │                              │
                       every 200 ms (5 FPS):                    │
                       1. next mascot.gif frame (Pillow)        │
                       2. resize to 80×80                       │
                       3. tint border by Angle (4 colors)       │
                       4. JPEG-encode (~2 KB)                   │
                       5. POST http://M5/face ────────────────▶ │
                                                                ▼
                                                drawJpg() into right 80×80
                                                left side keeps PID/IP text
                                                5 s of silence → auto-revert
```

#### 2. Dashboard control

A **🔓 LINK MONA** button is added next to the existing top-center **"M5: ◯◯"** widget.

- Click → switches to **🔗 LINKED** (green); PC starts streaming Mona to the M5 LCD
- Click again → reverts to **🔓 LINK MONA**; streaming stops

To avoid surprising the user mid-IP-typing, **LINK is an explicit opt-in**. By default the M5 LCD keeps showing IP/SSID as before.

#### 3. M5 LCD layout (while LINKED)

```
┌──────────────────────────────────────┐
│ ON  +1.2                  ┌──────┐  │
│ P=10.0 I=0.0 D=0.4        │      │  │
│ pw=-100 L=1600 R=1400     │ Mona │  │  ← 80×80 streaming
│ po=0 po2=0.0 4.1V         │      │  │
│ HomeWiFi 192.168.10.32    └──────┘  │
└──────────────────────────────────────┘
```

- Left: existing text (ON/OFF · angle · PID · power · battery · SSID/IP) — all preserved
- Right: 80×80 Mona (5 FPS)

#### 4. State-driven border color (Angle linkage)

| `|Angle|` | State | Border | Meaning |
|---|---|---|---|
| OFF | off | gray | control disabled |
| < 5° | calm | green | stable (in green band) |
| 5–15° | warn | amber | starting to wobble |
| ≥ 15° | crit | red | about to fall / fell (frames advance 2× for visual urgency) |

Reproduces the dashboard's CSS drop-shadow color shift on the streamed side.

#### 5. Failsafes

- **No `/face` for 5 s → M5 auto-reverts to full-text display** (LCD never gets stuck on a stale face)
- Streamer thread is daemonized — `Ctrl+C` of `server.py` cleanly stops it
- Even if M5 is unreachable, `requests.post(timeout=0.4)` gives up quickly and retries on the next cycle

#### 6. Network load

- Frame size: ~2 KB (80×80 JPEG quality=70)
- Cycle: 200 ms (5 FPS)
- Bandwidth: about **10 KB/s** — comfortable even on phone tethering
- M5 CPU: `drawJpg` ~30–50 ms per call. Only fires when a new frame arrives, so the 50 Hz control loop (10 ms cycle) stays unaffected.

#### 7. Files touched

- `firmware/inverted_pendulum/inverted_pendulum.ino` — new `/face` POST handler, `updateDisplay()` refit, `g_face_active` timeout revert
- `tools/server.py` — `/api/lcd_link` endpoint + Pillow-based streamer thread + state classifier
- `tools/templates/_demo_shim.js` — adds the LINK MONA toggle into the M5 widget (skipped in DEMO mode, so Pages is unaffected)

#### 8. Lessons

- **Streaming a GIF frame-by-frame instead of storing it as a file** sidesteps embedded-flash size limits
- **Even 5 FPS conveys the mood adequately** — perfect sync isn't possible, but state transitions (green → amber → red) reach the LCD within 1 s so the visual feedback still works
- **ESP32 `WebServer.arg("plain")`** turns out to expose the raw POST body (binary-safe) — perfect for shipping JPEGs without multipart

---
