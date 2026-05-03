# 進捗ログ・実験ログ (Progress Log)

> このファイルは README.md から切り出された詳細な実験記録です。
> M5StickC Plus2 倒立振子プロジェクトを 2026-04-13 から 2026-05-02 まで日々進めた、半田付け→ファーム移植→ハードウェアトラブル→IMU軸特定→PIDチューニング→GitHub Pages デモ化…までの試行錯誤の記録。
>
> 📖 [README に戻る](../README.md) ・ 🇬🇧 [English version](progress-log.en.md)

---


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

5つの運用シナリオ（家／AP／スマホテザリング／Tailscale遠隔／GitHub Pages）は README §「🌍 持ち運び・遠隔運用」に表で整理。

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

##### (6) 遠隔運用 (Tailscale など)

「オフィスから自宅のM5を操作したい」場合：

1. 自宅PCで `python3 tools/server.py` を常駐
2. 自宅PCとオフィスPCに **Tailscale**（無料枠）をインストール、同じTailnetに参加
3. オフィスPCのブラウザで `http://<自宅PCのTailscale IP>:5000/` を開く

ポート開放やDDNS設定なしで**P2P VPN**が張れる。Flaskが薄いプロキシなので、自宅LAN→M5のWiFi通信はそのまま、外側だけ暗号トンネルになる構成。

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
