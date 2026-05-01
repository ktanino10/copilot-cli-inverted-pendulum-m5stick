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
| 初めてPID制御に触れる | `docs/pid_guide.md` | このREADMEの「今回の実験を理論で読む」→ `inverted_pendulum.ino` のコメント |
| 制御工学の基礎がある | `docs/pid_theory.md` | このREADMEの実験ログ → `inverted_pendulum.ino` のソースコード |
| 通信・WiFi周りを知りたい | `docs/wifi_communication.md` | このREADMEの「🌍 持ち運び・遠隔運用」セクション |
| すぐ動かしたい | `README.md`（使い方セクション） | `servo_test.ino` で動作確認 → `inverted_pendulum.ino` 書き込み → 進捗ログ |

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

### 📚 PID制御ガイド・理論編

理論や基礎の話は、実験ログより先に読めるようにここへまとめる。

### 🌍 持ち運び・遠隔運用 — どこででもすぐ起動する方法

倒立振子を**自宅・オフィス・出先**で動かしたいときの実用的な選択肢を、
おすすめ順にまとめる。すべてファームウェア／UIに既に組み込み済み。

> 📡 通信そのものの仕組みは別資料にまとめた：**[docs/wifi_communication.md](docs/wifi_communication.md)** — 初心者向けと技術者向けの2部構成（WiFi/HTTP/AP/Tailscale/fetchシム などをこのプロジェクトの実装に紐づけて解説）。

| # | シナリオ | 使い方 | 必要なもの | 備考 |
|---|---|---|---|---|
| **A** | **マルチSSID**（推奨） | `wifi_config.h` に複数のSSIDを並べておくだけで、起動時にスキャンして電波が入っているものへ自動接続 | `wifi_config.h` 編集のみ | 一度書き込めば、家・オフィス・スマホテザリングを自動切替。設定例は `wifi_config.h.example` 参照 |
| **B** | **ソフトAPフォールバック** | どの既知SSIDも見えないとき、M5自身が `IPS-CTRL`（pass: `ips12345`） というWiFiアクセスポイントを立ち上げる。ノートPC/スマホでそのSSIDに繋ぎ、`http://192.168.4.1/` を開くだけ | M5本体だけ／ルータ不要 | カフェ・電車・社外プレゼンなど、信用できないWiFi環境でも完全独立で動く |
| **C** | **スマホテザリング** | iPhone/Android のテザリングSSID/パスワードを `IPS_WIFI_LIST` に登録 → どこへ行ってもスマホONでM5が自動接続 | スマホ + データプラン | 同じテザリング下でPC側も繋げばUIアクセス可 |
| **D** | **遠隔（オフィスから自宅のM5を操作）** | 自宅PCで `tools/server.py` を起動 → [Tailscale](https://tailscale.com/) などのVPNを自宅PCとオフィスPCで共有 → オフィスから `http://<自宅PCのTailscale IP>:5000/` でアクセス | 自宅PC常時起動 + Tailscale (無料枠OK) | 動画を別カメラで配信すれば本当に遠隔チューニングが可能 |
| **E** | **GitHub Pages デモ** | ハードウェアを持っていない人にUIを見せたいとき。`https://<owner>.github.io/inverted-pendulum-m5stick/` で**シミュレートされたデバイス**と統計解析画面が動く | 何も不要 | `tools/build_demo.py` で自動生成。`docs/demo/` を Pages 公開設定に |

#### `wifi_config.h` のマルチSSID設定例

```cpp
#pragma once

// (B) PORTABLE 形式 — 上から順に試す
#define IPS_WIFI_HAS_LIST
static const IpsWifiNet IPS_WIFI_LIST[] = {
  {"home_2g",       "home_password"},
  {"my_iphone_hot", "phone_password"},   // テザリング
  {"office_guest",  "office_password"},
};

// 既知SSIDが全部圏外なら自動でAP起動（ここはお好みで上書き）
// #define IPS_AP_SSID "my-pendulum"
// #define IPS_AP_PASS "supersecret"
```

#### LCDの表示

接続後、M5の画面下段に **`SSID IP`** が表示される（例: `home_2g 192.168.10.32`）。
APフォールバックに落ちた場合は **`IPS-CTRL (AP) 192.168.4.1`** と出るので、
ブラウザでその IP を開くだけ。

#### サーバ側の切替

`tools/run_server.sh` 起動時に `M5_URL` で接続先を上書きする：

```bash
M5_URL=http://192.168.10.32 ./tools/run_server.sh    # 自宅
M5_URL=http://192.168.4.1   ./tools/run_server.sh    # APモード
M5_URL=http://172.20.10.5   ./tools/run_server.sh    # iPhoneテザリング
```

LCDに表示されたIPをそのまま入れればOK。


| 資料 | 内容 | 対象 |
|------|------|------|
| **[docs/pid_guide.md](docs/pid_guide.md)** | ほうきバランスの例えから、P/I/D の役割、倒立振子への応用、実機での調整手順までをやさしく説明 | 初めてPID制御に触れる人 |
| **[docs/pid_theory.md](docs/pid_theory.md)** | 運動方程式、状態空間表現、カルマンフィルタ、安定性解析、LQRなどを数式で説明 | 制御工学を深く学びたい人 |

### 今回の実験を理論で読む

#### この章を読む意義

ここまで、kp/kd/ki/kspd/kdst/pneg/minp など多くのパラメータを試した。それでも安定しないのは、**「次にどれをどう動かせばいいか」を感覚で決めているから**。

理論を理解する目的は数式のためではなく、**実機の症状（振動・後ろへ逃げる・走り込み）から原因を一意に特定できるようにする**ため。具体的には、次の判断ができるようになる。

| 場面 | 理論なし | 理論あり |
|------|----------|----------|
| 振動した | `kd` を上げる/下げる、どっちか勘で試す | ノイズ起因なら `kd` を下げる、位相遅れ起因ならフィルタを見直す、出力飽和なら `power_limit` を下げる、と切り分けられる |
| 後ろへ走って倒れる | 「符号が逆では？」と疑い符号を入れ替え続ける | `power` が負側に出ているのに動いていない＝サーボ不感帯と分類でき、符号は触らずに `pneg` または最低出力で対処する |
| 立つ時間が短い | `kp` を上げる/下げる、無限ループ | `Angle` の発散か、`power` が出ていないかで原因を分けられる |

つまりこの章は、**今後の試行を「感覚的なパラメータ総当たり」から「ログ→症状分類→対応する1パラメータ修正」へ変えるための土台**。

今回の挙動は、単純な「PIDの値が合っていない」だけではなく、次の4つが重なっている。

1. **倒立振子はそもそも不安定な系**（直立が自然には維持されない）
2. **センサに遅れとノイズがある**（IMU + フィルタ）
3. **D項の位相とスケールが角度と完全には一致していない**
4. **FS90Rサーボに不感帯と個体差がある**

以下、それぞれを「なぜ理解する必要があるか → 理論 → 今回のログでどう現れたか」の順で説明する。

---

#### 1. 倒立振子は「倒れる方向へ走って支点を入れる」制御

##### なぜ理解するか

「前に傾いたら前に走る、それで合っているのか？」という疑問を毎回持たないため。**動作方向は正しい**と確信できれば、症状が出たときに「符号」ではなく「量・タイミング」を疑える。

実際この4日間、何度も「符号が逆では？」と疑い、`getPitch()` の式や `+power/-power` の組み合わせを入れ替えてきた。次の章でわかるが、**今の問題は符号ではない**。それを理論的に確信するために、この章が必要。

##### 理論

倒立振子は普通の振子と逆で、直立が自然に安定しない。少し前に傾くと重心はさらに前へ倒れようとする。そこで車輪を前へ動かし、**車輪という支点を重心の下へ入れる**ことで倒れないようにする。

```
前に傾く
  ↓
車輪を前へ動かす（支点を重心の下へ）
  ↓
重力モーメントが減る
  ↓
倒れる速度が減る
```

これは数式では `θ̈ = α·θ − β·ẍ_wheel` と書ける（理論編 1.3 節参照）。**車輪加速度 ẍ で重力項 αθ を打ち消す**のが本質。

> **📘 補足：ここに出てくる用語**
>
> - **θ（シータ）**: 直立からの傾き角。直立=0、前傾でプラス（または符号は実装次第）。
> - **θ̈（シータ・ツードット）**: 角加速度。「傾きが**どれくらいの勢いで**さらに傾こうとしているか」。
> - **ẍ_wheel（エックス・ツードット）**: 車輪の加速度。「車輪を**どれくらい強く加速**したか」。
> - **α・β**: ロボットの質量・長さ・慣性モーメントから決まる正の定数。重要なのは **α が正である**こと、つまり「傾けば傾くほど、さらに倒れる方向の力が出る」という**不安定性そのもの**を表している点。
> - **重力モーメント**: 重心が支点の真上から外れたときに、倒そうとする回転力。
>
> 数式 `θ̈ = α·θ − β·ẍ_wheel` の意味は、「傾きはひとりでに大きくなる（`αθ`）。それを車輪を加速して打ち消す（`−β·ẍ`）」というだけ。**この打ち消しが間に合わなければ倒れる、間に合いすぎれば反対へ倒れる**。これが倒立振子のすべて。

##### 今回のログでの現れ方

「前にも後ろにも倒れる」「ある傾きから走りながら倒れる」は、**方向は合っているが、走る量・タイミング・モーターの効き方が合っていない**状態。だから次に疑うべきは符号ではなく、「どれだけ走らせるか（P）」「どこで止めるか（D）」「どれくらい実際に動くか（モーター特性）」になる。

---

#### 2. P項：起き上がる力。強すぎると振動になる

##### なぜ理解するか

`kp` を変えたときに何が起きるかを予測できれば、ログで `Angle` の振幅と周期を見るだけで、`kp` が大きすぎるか小さすぎるかを判定できる。やみくもに `kp=10 → 15 → 20 → 10` と往復する必要がなくなる。

##### 理論

P項は「今どれだけ傾いているか」に比例してモーター出力を出す。

```cpp
P = -kp * Angle
```

倒立振子の線形化モデルでは、P項のみを使うと閉ループの特性方程式は `s² + 0·s + (β·kp − α) = 0` になる。**減衰項がない**ため、`kp` を大きくしても振動が大きくなるだけで、勝手には止まらない。

> **📘 補足：ここに出てくる用語**
>
> - **閉ループ**: 「センサで測る → 計算する → モーターを動かす → 実機が動く → 再びセンサで測る」と**ぐるっと一周つながった制御の輪**のこと。倒立振子はこの輪が回り続けて初めて立つ。
> - **特性方程式 `s² + a·s + b = 0`**: その閉ループが**どんな振動の仕方をするか**を決める方程式。`a`（s の係数）が**減衰**、`b`（定数項）が**ばねの強さ**に対応する、と理解しておけばよい。
> - **減衰項がない**: 上の式で `s` の係数が0、つまりブレーキ役がいないということ。ばねだけのおもちゃのように、振動が永遠に続く（実機ではモーターが飽和して暴れる）。
> - **リミットサイクル振動**: ゲインを上げすぎてモーター出力が上限に張り付いたまま振動する状態。理論的には収束するはずでも、出力が飽和すると一定振幅で振動し続ける。

- `kp` が小さい: 重力モーメント `α·θ` を打ち消せず、そのまま倒れる
- `kp` が適度: 戻ろうとするが、減衰がないため通り過ぎる
- `kp` が大きい: 通り過ぎる量が大きくなり、振動の振幅と周期が決まる
- `kp` が大きすぎる: モーター出力が飽和し、リミットサイクル振動になる

つまり**P項単独では絶対に安定化できない**。必ずD項（次章）が要る。

##### 今回のログでの現れ方

「時間は伸びたが前後どちらにも倒れる」「振動しながらある傾きから走る」は、まさにP項中心で動いていて、D項のブレーキが足りていない状態。`Angle` が0を中心に行ったり来たりしながら振幅が広がっていれば、それが理論通りの**P項単独に近い挙動**。

---

#### 3. D項：ブレーキ。ただしノイズや遅れがあると振動源になる

##### なぜ理解するか

D項は安定化に必須だが、**増やせば増やすほど良いわけではない**。ノイズに反応すれば自分で振動を作り、位相が遅れていれば加速側に働いてしまう。「振動した→`kd` を下げる」も「振動した→`kd` を上げる」も、**両方とも正解になり得る**。どちらが正解かはログで見分ける必要があり、それが`kd` 調整の前にこの章を理解する理由。

##### 理論

D項は「傾きの速度」を見てブレーキをかける項。これを入れることで、特性方程式が `s² + β·kd·s + (β·kp − α) = 0` となり、**減衰項 `β·kd·s` が初めて現れる**。これがあって初めて閉ループが安定化する。

> **📘 補足：なぜD項がブレーキになるのか**
>
> 「角度が0度でも、後ろへ向かって速く動いているなら、もうすぐ後傾が深くなる」とわかる。D項はこの**「もうすぐ起きる傾きの変化」を先回りして打ち消す**。
> 自動車のブレーキも、止まっている車には効かないが、走っている車に対しては減速力を出す。これと同じで、D項は**動いているときだけ力を出す**性質がある。だから「ブレーキ」と呼ぶ。

```cpp
dAngle = gyro[0] - gyroOffset[0]
D = -kd * dAngle
```

ただし実機では、「微分」は次の3つの問題に弱い。

| 問題 | なぜ起きるか | 結果 |
|------|--------------|------|
| ノイズ増幅 | 微分は高周波成分を増幅する性質がある（伝達関数 `Kd·s` のゲインは周波数に比例） | ジャイロのノイズが `kd` 倍されて出力に出る |
| スケール不一致 | `Angle` は `acc[2]` 由来の独自スケール、`dAngle` は `gyro[0]` の deg/s。両者の単位が物理的に対応していない | ブレーキが強すぎたり弱すぎたりする |
| 位相遅れ | 角度側にローパスフィルタ（`fil_N=5`）がかかっていて遅れる一方、ジャイロは生で速い | D項が「倒れている向き」と逆を向く瞬間ができ、ブレーキではなく加速になる |

> **📘 補足：ここに出てくる用語**
>
> - **ノイズ増幅**: 微分という操作は、**速い変化ほど大きく**増幅する性質がある。ジャイロの細かいゆらぎ（高周波ノイズ）も「速い変化」なので、`kd` 倍されてモーター出力に出てしまう。
> - **deg/s（度毎秒）**: ジャイロが返す単位。「1秒間に何度回転しているか」。一方の `Angle` は今回 `acc[2]` 由来の独自スケールで、deg と一致していない可能性がある。**単位が揃っていない量を引き算しているようなもの**で、`kd` の最適値が見つけにくい原因になる。
> - **ローパスフィルタ**: 速い変化を削って遅い変化だけ通すフィルタ。`fil_N=5` は「直近5サンプルの平均を使う」という意味で、ノイズは消えるが、**実際に倒れ始めてから検出までに数サンプル分の遅れ**が生じる。
> - **位相遅れ**: 入力に対して出力が時間的にずれること。倒れ始めから検出までに遅れがあると、D項が「もう前に倒れてしまった後」に「後ろへブレーキ」を出すことがあり、**ブレーキではなく加速**になってしまう。

##### 今回のログでの現れ方

`failure_log_oscillation.csv` がこの典型。D項や出力が強く、`power` が ±500 付近まで飽和してから `ANGLE_LIMIT` を超えている。

これは「力不足」ではなく、**閉ループゲインが高すぎる、またはD項がノイズ/位相遅れに反応しすぎている**と読む。対処は `kd` を下げる、フィルタを見直す、`power_limit` を下げる、のいずれか。**「振動したから kd を上げる」ではない**。

逆に、`kd=0.25` にしたとき早く倒れたのは、減衰項が不足して `s² + β·kd·s + …` の解が振動的になり、止めきれずに `ANGLE_LIMIT` を越えてしまった、と読める。

---

#### 4. I項・速度項・位置項：走り続ける問題を止めるが、早く入れると暴れる

##### なぜ理解するか

「立つ時間は伸びたが結局走って倒れる」を解決したくて、I項や `kspd` を入れたくなる。しかし**順番を間違えると今より悪くなる**。実際、今回 `kspd=1.2` を入れたら激しい振動が出た。なぜ「正しいはずの追加」が悪化させるのかを理解しないと、また同じ罠を踏む。

##### 理論

角度のP+D制御は「今の傾きを0にする」だけを目的にしている。これだけでは、**車輪位置や車輪速度は何も拘束されない**。なので、ロボット全体がじわじわ移動し続ける（ドリフトする）。

これを止めるために：

- **I項（積分）**: 角度の小さなずれが長時間続くと積もって、その分の補正力を出す
- **kspd（速度フィードバック）**: 車輪が動き出したらブレーキをかける
- **kdst（位置フィードバック）**: 元の位置から離れたら戻す

理論的には、これらは「拡張された状態フィードバック制御 `u = -K·x`」の一部（理論編 4 節参照）。

しかしここに**カスケード制御の鉄則**がある。

> **📘 補足：カスケード制御とは**
>
> 制御ループが**入れ子**になっている構造のこと。今回の場合：
>
> - **内側ループ（速い）**: 「傾きを0にする」姿勢制御（P + D）
> - **外側ループ（遅い）**: 「位置・速度を0にする」位置/速度制御（kspd・kdst・I）
>
> 内側ループは10ms周期で「今すぐ倒れない」ことを担当し、外側ループはより長い時間スケールで「ドリフトしない」ことを担当する。
>
> **鉄則**: 内側が先に十分速く・安定に動いていることが前提。内側が振動している段階で外側を閉じると、外側の指令が内側の振動と干渉して**さらに悪化**する。家を建てる前に内装を始められないのと同じ。

> **内側ループ（姿勢）が安定する前に、外側ループ（位置・速度）を閉じてはいけない。**

姿勢制御がまだ振動している段階で位置/速度補正を足すと、

- 「前に傾いた→前に走る」（姿勢制御の正しい指令）
- 「前に走り出した→ブレーキ」（速度制御の指令）

の2つが**逆方向**に出力を要求し合い、結果として出力が振動する。さらに、車輪が止められないのにブレーキを強くすると、姿勢が崩れる。

##### 今回のログでの現れ方

`failure_log_latest.csv` は、`ki=0` にしたつもりでも `kspd/kdst` が残っていて、`Speed` 由来の項（`k_speed`）が出力に乗り、振動と走り込みが増幅していた。

そのため現在の方針は、

| 段階 | 設定 | 目的 |
|------|------|------|
| いま | `ki=0`, `kspd=0`, `kdst=0` | まずP+Dだけで姿勢を安定化 |
| 次 | 姿勢が数秒静止できたら、`kspd` を小さく入れる | 走り続けを抑える |
| その後 | `ki` と `kdst` を慎重に入れる | 定位置に戻す |

になる。**順番を守ることが理論的に意味がある**。

| 項 | 役割 | 今回の扱い |
|----|------|------------|
| P | 傾いた分だけ戻す主力 | 必要。ただし強すぎると振動 |
| D | 倒れる速度を止めるブレーキ | 必要。ただしノイズ・位相に注意 |
| I | 小さなずれの蓄積を消す | 姿勢が安定してから |
| kspd/kdst | 走り続け・位置ずれを抑える | 姿勢P+Dが安定してから |

---

#### 5. 「角度」は物理角度ではなく制御用の偏差

##### なぜ理解するか

`getPitch()` の式を理論通りの `atan2(-acc[1], acc[2])` に戻すべきか、現在の `(acc[2]-1.0)*57.3` のままで良いかを判断するため。**「教科書通りの物理角度」と「実機で前後分離が安定する量」は別物**だと理解しないと、また式を入れ替え続けることになる。

##### 理論

理想的には、加速度センサから物理的な傾き角は `θ = atan2(-acc[1], acc[2])`（rad）で求まる。

ただしこれが成り立つのは、

1. IMUが本体に対して理想的な向きで取り付けられている
2. 直立時に該当軸の重力成分が想定通り（`acc[2]≈1, acc[1]≈0`）
3. 動的加速度ノイズがフィルタで十分落ちている

の全てが成り立つときだけ。実機では取り付け向きや個体差で、これらが完全には成り立たない。

##### 今回のコードと、その意味

現在のコードでは：

```cpp
getPitch() = (acc[2] - 1.0) * 57.3
```

これは**`acc[2]` の直立値 1.0 からの偏差を線形に角度っぽくスケールしただけの量**。物理角度ではない。

それでも制御には使える。なぜなら、PIDが必要としているのは「物理的に正しい角度」ではなく、

1. 前に傾けたときと後ろに傾けたときで**符号が反対になる**こと
2. 傾きが大きくなるほど**値も単調に大きくなる**こと
3. その値に対してモーターが**正しい方向へ動く**こと

の3つだけだから（線形制御の理論で言えば、状態と入力の符号関係が保たれていれば、ゲインで吸収できる）。

> **📘 補足：「ゲインで吸収できる」とは**
>
> 例えば「実際の傾き5度」が `Angle=10` と出てきても、`Angle=2.5` と出てきても、PIDから見れば違いはない。`kp` を半分にしたり倍にしたりすれば、**最終的にモーターへ出る出力は同じ**にできる。
>
> 大事なのは「実際の傾きと `Angle` が**符号が一致**し**単調に増える**」こと。スケールがズレていても `kp` で吸収できる。これが「物理的に正しい角度でなくても制御できる」と言える理由。
>
> ただし、**スケールがズレたまま `Angle` と `dAngle` を別の式から作る**と、両者の比が物理的に正しい比から外れる。これがこの先の節で出てくる「`kd` の最適値が狭い」問題の根本。

##### 残っている不明点

ただし、

- 角度は `acc[2]` 由来（線形偏差）
- D項は `gyro[0]` 由来（角速度 deg/s）

なので、**角度とD項のスケール・位相が完全には一致していない**。これが「kd の最適値が見つけにくい」「振動と過小制動の境目が狭い」原因の可能性が高い。

→ 次回、`Angle` の差分から作る `dAngle_from_angle` を追加して比較するのは、この食い違いを実際に測るため。

---

#### 6. 後ろへ逃げる挙動は、符号逆ではなく「片方向の効き不足」

##### なぜ理解するか

「後ろへ走って倒れる」を見ると、本能的に「符号が逆だ」と思いたくなる。実際この4日間、その仮説で何度も `getPitch()` や `+power/-power` を入れ替えた。**それでも直らない理由**を理論的に説明できるようになるのが、この章。

##### 理論

「符号が逆」なら、ログでは次のいずれかが見えるはず。

- 前に傾いたとき `power < 0`（後ろへ動こうとする）
- 後ろに傾いたとき `power > 0`（前へ動こうとする）

しかし `failure_log_backward_run.csv` では、

- `Angle` は小さい範囲（数度）にいる
- `power` は負側（-30〜-80程度）に偏り続ける
- 飽和もしていない
- それでも実機は後ろへ走って倒れる

つまり**制御器は正しい方向に正しい大きさの指令を出している**。にもかかわらず実機が止まらないのは、**指令と実出力の間（モーター・機械系）に問題がある**ということ。

| 原因 | 説明 |
|------|------|
| FS90Rの不感帯 | 1500μs 周辺の小さいPWM差ではサーボが動かない、または片輪だけ動く |
| 前進/後退の非対称 | 同じ絶対値の `power` でも、前進側と後退側で実際の速度が違う（ギア・摩擦・電源電圧降下） |
| 左右差 | 左右サーボのニュートラル点や摩擦が違い、合計トルクが片寄って直進にならない |
| 段差出力の副作用 | `minp=80` のような最低出力は不感帯を越えるが、`Angle` が小さい範囲で出力が±に跳ね、リミットサイクル振動を作る |

`pneg` を上げると立つ時間が伸びた一方、上げすぎると小刻み振動が増えた。**片方向だけのループゲインを上げる＝閉ループの位相余裕が片方向だけ減る**ので、安定だった領域が狭くなる、という古典制御で説明できるトレードオフ。

> **📘 補足：ここに出てくる用語**
>
> - **不感帯（ふかんたい / dead zone）**: 入力を入れても出力が出ない領域のこと。FS90Rの場合、1500μs前後の小さなPWM差では**サーボが反応しない**。倒立振子は小さい角度で細かく補正したいので、不感帯にぶつかると「指令は出ているのに実機は動かない」現象が起きる。
> - **ループゲイン**: 閉ループ全体（センサ→計算→モーター→実機→センサ）を一周したときの**信号の倍率**。これが大きいと反応は速いが、大きすぎると振動・発散する。
> - **位相余裕（Phase Margin）**: 閉ループが「あと何度位相が遅れたら振動を始めてしまうか」の余白。古典制御の安定性指標。`pneg` を上げる＝後退側だけループゲインが大きくなる＝**後退側だけ位相余裕が減る**ので、後退方向で振動しやすくなる。
>
> つまり「後ろへ逃げるから `pneg` を上げよう」は短期的には立つ時間を伸ばすが、**安定だった範囲を犠牲にしている**ことを理解しておく必要がある。だから本来の解決は `pneg` をひたすら上げることではなく、**不感帯そのものへの対処**（最低出力やサーボのトリム）になる。

##### 今回のログでの含意

ここを理解しているか否かで、次の判断が変わる。

| 理論なし | 理論あり |
|----------|----------|
| 「後ろへ倒れる→符号を疑う」 | `power` の符号を見て、合っていれば符号は触らない |
| 「pneg を 1.45 → 1.70 → 1.90 と上げ続ける」 | 振動が出始めた時点で、上げる方向ではなく**不感帯対策（最低出力やオフセット）**へ切り替える |

---

#### 7. ログの読み方

今回追加した `X,...` ログでは、次のように切り分ける。

| ログで見るもの | 何がわかるか |
|----------------|--------------|
| `Angle` | 本体がどちらへどれだけ傾いているか |
| `dAngle` | 倒れる速さ。D項の元データ |
| `P/I/D` | どの項が出力を支配しているか |
| `power` | 制御器が最終的に命令したモーター出力 |
| `powerL/powerR` | 左右サーボへ実際に送ったPWM |
| `accZ/gyroX` | 角度式とジャイロD項の整合 |
| `Angle vs dAngle` | 振動が内側へ収束しているか、外側へ発散しているか |
| `Angle vs Power` | 傾きに対して正しい方向・十分な量の出力が出ているか |

症状ごとの読み方は以下。

| 実機の症状 | 理論的な読み方 | 次に見るポイント |
|------------|----------------|------------------|
| 小刻みに振動して倒れる | P/Dが強すぎる、D項がノイズに反応、または出力飽和で位相余裕が足りない | `D`, `power`, `power_limit` |
| 振動は少ないが早く倒れる | PまたはDが不足し、倒れる速度に追いつけていない | `Angle` が増え続けるか、`power` が弱いか |
| 後ろへ走って倒れる | 符号逆ではなく、後退側の実効出力不足・不感帯・左右差の可能性 | `power < 0` 時の `powerL/powerR` と実機の動き |
| 前後どちらにも倒れる | 安定点に入っておらず、補正量とブレーキのバランスが未調整 | `Angle vs dAngle` が外向きに広がるか |
| `minp` で激しくなる | 不感帯対策の段差出力がリミットサイクルを作っている | `power` が小角度で急に跳ぶか |
| `kspd/kdst` で走り込みが増える | 姿勢制御が安定する前に位置/速度制御が干渉している | `Speed`, `k_speed`, `I` 相当の蓄積 |

次回はこの表に沿って、感覚で符号やゲインを入れ替えず、1回の転倒ログから「補正不足」「補正過大」「D項の位相ずれ」「モーター不感帯」のどれかに分類してから修正する。

### 進捗ログ・実験ログ

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

#### 2026-05-01: 前後符号の再検証と「後ろへ逃げる」問題のログ化

4/30深夜〜5/1にかけて、前後の角度符号・PID・モーター出力を実機ログで再検証した。最終的に、**acc[2] の偏差を使う線形角度**で前後の符号が分かれることを確認した。

##### 今日わかったこと

| 項目 | 結論 |
|------|------|
| 角度軸 | `acc[2]` の変化が前後の分離に効く。`acc[1]` は前後どちらでも同方向に動く場面があり、単独では不安定 |
| 角度式 | 現在は `getPitch() = (acc[2] - 1.0) * 57.3`。物理角度そのものではないが、制御用の符号・偏差として利用 |
| ゼロ基準 | `on` / AボタンON時に `Pitch_offset = Pitch_filter` として、今の姿勢を自動で `Angle=0` にする |
| 詳細ログ | `X,...` 行を追加し、`Angle,dAngle,P,I,D,Speed,power,L/R,acc,gyro` をCSV保存できるようにした |
| I項・速度項 | `ki=0` でも `kspd/kpower` を入れると激しく振動しやすい。現時点ではOFF |
| 最低出力 | `minp=80` はFS90R不感帯対策として試したが、段差的に効いて振動源になりやすい。現時点ではOFF |
| 前後非対称 | 後ろへ逃げやすいため、`pneg`（後退補正）を前進側より強くする方向は効果あり |

##### 保存した転倒ログ

以下のCSVは、次回の解析用に保存した。

```
tools/data/failure_log_latest.csv        # ki=0 でも kdst/kspd が残って走り込みを増幅したログ
tools/data/failure_log_oscillation.csv   # D項・出力飽和が強く、小刻み振動→ANGLE_LIMIT超過したログ
tools/data/failure_log_backward_run.csv  # 後ろへ逃げるが、飽和は少なく補正不足寄りだったログ
```

##### 転倒ログの可視化（PNG表示）

`tools/visualize_root.py` を詳細ログ形式に対応させ、各ログからPNGを生成した。READMEではクリックしなくても見えるように、PNG画像を横幅いっぱいに表示する。

```
python3 tools/visualize_root.py tools/data/failure_log_*.csv
```

###### 後ろへ逃げるログ

`failure_log_backward_run.csv`

Angle は小さい範囲に留まる一方、power が負側に偏り続ける。補正方向は合っているが、後退方向の補正不足またはサーボ低速域の効き不足が疑われる。

<p align="center">
  <img src="tools/data/failure_log_backward_run.png" alt="後ろへ逃げるログの可視化" width="100%">
</p>

###### kspd/kdst が残って走り込むログ

`failure_log_latest.csv`

`ki=0` でも速度・距離系が残り、走り込みを増幅していた時期のログ。

<p align="center">
  <img src="tools/data/failure_log_latest.png" alt="速度距離系が残って走り込むログの可視化" width="100%">
</p>

###### 小刻み振動・飽和ログ

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

##### 現在のファームウェア設定（次回の出発点）

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

##### 実機での観察

- 設定によっては **3〜4秒程度立つ** ところまで改善。
- ただし安定点には入らず、少し振動しながら、ある傾きから前または後ろへ走りながら倒れる。
- 後ろへ逃げる傾向が比較的多い。
- `kspd=1.2` や `minp=80` など段差・速度系を入れると激しい振動が増えた。
- `pneg=1.70` は立つ時間を伸ばしたが、完全には止まらない。

##### 次回の候補

1. `failure_log_backward_run.csv` を可視化し、後ろへ逃げる直前の `Angle/dAngle/P/D/power` の位相を確認する。
2. `pneg` を大きくする前に、`power_neg_scale` ではなく **後退側だけの小さな `bias`** または **左右/前後のサーボオフセット**を試す。
3. `gyro[0]` のD項ではなく、`Angle` の差分から作る `dAngle` 比較モードを追加し、D項の位相ズレを確認する。
4. `fil_N=5` の遅れを疑い、`fil_N=3` をログ付きで比較する。
5. 自動PIDスイープは、現在の「線形 `acc[2]` + 自動ゼロ合わせ + 前後非対称補正」を基準にやり直す。

#### 2026-05-02: チューニングUIの解析強化・ブート演出・GitHub Pagesデモ・どこでも起動

実機のハードはそのままで、**ソフト側を一気に整えた日**。前半はチューニングUIの解析機能と演出、後半は持ち運び対応とウェブデモ化。

##### 1. チューニングUIの統計解析強化（STATS LAB）

> 「期待姿勢が緑（|∠|≤5°）を保つための最適なパラメータの特定」を統計的に出せるようにしたい、という要件。

- **GREEN ZONE バナー** — 全解析結果のサマリにベスト構成を抽出表示
- **STATS LAB パネル** — 2D ヒストグラム / 3D ヒストグラム / KP×KD 散布図 / 複数パラメータ組合せ解析
- **エラーバー / 信頼区間**入りのパラメータ感度プロット
- **score（0〜100）** を新設：
  - `duration·3`（最大30）+ `green%·50`（最大50）+ `max(0, 20−mean|∠|·2)`（最大20）
  - 1テスト1スコア。緑帯滞在時間 + 倒れない時間 + 振れ幅の小ささを統合
- **`⇅ APPLY` ボタン**：過去の好スコア構成のパラメータを **ワンクリックで実機に再投入**（`/api/c?q=k=v` を順に push）
- セッション一覧を `score` 降順で並べてベスト設定が一目でわかる

##### 2. マスコット演出とi18n

| マスコット | 役割 |
|---|---|
| **Mona**（GIF） | リアルタイムATTITUDE表示。安定時はゆったり、転倒の瞬間に高速アニメ。`+/-` の角度に合わせて顔の向きが追従 |
| **Copilot**（GIF） | 解析画面遷移時のヒーロー演出 |
| **Ducky**（GIF） | 言語切替時に画面中央に大きく登場（モード切替の明示） / 通信ロスト時の見守りバナー |

> 途中で「倒れた時のDuckyは要らない」というフィードバックがあり、PWM飽和バッジ・QUICK FALLトースト・診断パネルのリーダー切替など、**転倒に関わるDucky表示はすべて削除**。Duckyは「通信・モード切替」専用キャラに役割整理。

##### 3. ブート演出（SEED風スタートアップ診断）

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

##### 4. GitHub Pages デモ化（`docs/demo/`）

ハードを持っていない人にもUIを触ってもらえるように、Flaskなしで動く**完全静的版**をビルド。

- `tools/build_demo.py` で `tools/templates/index.html` から自動生成
- `window.IPS_DEMO = true` フラグを注入し、`fetch` をシムが横取り：
  - `GET /api/s` → シミュレートされた50Hzテレメトリ（PIDパラメータ反映つき）
  - `GET /api/c?q=k=v` → シム内の状態を実際に書き換え（スライダーがちゃんと効く）
  - `GET /api/sessions` → バンドルされた `data/sessions/index.json`
  - `GET /api/sessions/<id>` → 同 `data/sessions/<id>.json`
- 5本の合成セッション（`baseline / kp_too_high / better_kd / pneg_tuned / winner`）を内包し、score 13〜32 の比較が見える
- 左下に紫の **`DEMO MODE`** バッジで明示

##### 5. どこでも起動できるようにする — マルチSSID + APフォールバック

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

##### 6. 通信まわりの技術解説（このプロジェクトでの実装）

> 上の機能を理解しておくと、自分で拡張しやすい。

###### (1) 制御UIとM5の通信モデル

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

###### (2) WiFiの3モード — STA / AP / STA+AP

| モード | 役割 | このプロジェクトでの使いどころ |
|---|---|---|
| **STA**（Station） | 既存ルータの**子機**として参加。インターネットアクセス可、ブラウザ→M5は同一LAN必要 | 家・オフィスで普通に使う。`WiFi.begin(ssid, pass)` |
| **AP**（Access Point） | M5自身が**親機**になる。ルータ不要だがインターネットなし | フォールバック。社外プレゼンや電車内など、信用できないWiFi下で完全独立動作 |
| **STA+AP**（Hybrid） | 両方同時。STAでルータに繋ぎつつ、自分もAP（中継機のような動き） | このプロジェクトでは未使用。将来 mDNS応答+遠隔管理を一緒にやりたい時の選択肢 |

切替は `WiFi.mode(WIFI_STA / WIFI_AP / WIFI_AP_STA)`。本プロジェクトはまずSTAで全SSID試行 → 全敗時に `WIFI_AP` に切替 + `WiFi.softAP(SSID, PASS)` を実施。

###### (3) `WiFi.scanNetworks()` で「圏外なのに繋ぎに行く」を回避

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

###### (4) ソフトAPモードの仕様（ESP32）

- パスワードは **8文字以上必須**（短いと `softAP()` が `false` を返す）
- デフォルトIPは **`192.168.4.1`**（DHCPサーバとして 192.168.4.2〜 を配布）
- **インターネットアクセスは無し** — つまりブラウザ側がスマホ等の「モバイルデータ」を併用していない限り外には出られない
- ノートPC側からは「IPS-CTRL」というSSIDが普通のWiFiとして見える

###### (5) `fetch` シム（DEMOモード）の仕組み

GitHub Pagesに置くとFlaskがいないので `/api/*` が全部404になる。ブラウザの `window.fetch` を**起動時に差し替え**て、URLパターンに応じて：

- `/api/s` → JSオブジェクトを `Response` として返す（実際のHTTPは飛ばない）
- `/api/c?q=...` → JS変数を書き換える
- `/api/sessions/...` → 隣接の静的JSONを `_origFetch` で読みに行く

これは**Service Workerなしのモック**。Service Workerを使うとオフラインキャッシュ等もできるが、デモ用途には1ファイルで完結するこの方式が手軽。

###### (6) 遠隔運用 (Tailscale など)

「オフィスから自宅のM5を操作したい」場合：

1. 自宅PCで `python3 tools/server.py` を常駐
2. 自宅PCとオフィスPCに **Tailscale**（無料枠）をインストール、同じTailnetに参加
3. オフィスPCのブラウザで `http://<自宅PCのTailscale IP>:5000/` を開く

ポート開放やDDNS設定なしで**P2P VPN**が張れる。Flaskが薄いプロキシなので、自宅LAN→M5のWiFi通信はそのまま、外側だけ暗号トンネルになる構成。

##### 7. 今日のコミット

```
4210dac  Mona パレード filter 修正（白すぎ問題）
9184b19  per-session score + APPLY ボタン + 転倒時Ducky除去
c45f4a7  ブート画面: ライブ診断シーケンス
e4a80ce  ブート画面サブタイトルから "SEED" 表記削除
(latest) GitHub Pages デモ + マルチSSID + APフォールバック + 通信解説
```

##### 8. 明日以降の小ネタ候補

- `mDNS` 応答（`pendulum.local`）でIP固定不要に
- BLE経由のワイヤレスシリアル（Wi-Fiが完全にダメな場所用）
- GitHub Actions でデモを自動再ビルド + Pages デプロイ
- セッションのスコア順ランキングをCSVエクスポート



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
| New to PID control | `docs/pid_guide.md` | README experiment notes → comments in `inverted_pendulum.ino` |
| Control theory background | `docs/pid_theory.md` | README experiment log → `inverted_pendulum.ino` source code |
| Just want to run it | `README.md` (Usage section) | `servo_test.ino` to verify → flash `inverted_pendulum.ino` → progress log |

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

### 📚 PID Control Guide and Theory

The learning materials are placed before the experiment logs so the theory can be read first.

| Resource | Contents | Audience |
|----------|----------|----------|
| **[docs/pid_guide.md](docs/pid_guide.md)** | Beginner-friendly explanation of P/I/D terms, broom-balancing analogy, inverted pendulum application, and practical tuning steps | Newcomers to PID control |
| **[docs/pid_theory.md](docs/pid_theory.md)** | Equations of motion, state-space representation, Kalman filter derivation, stability analysis, LQR, and related control theory | Readers who want the mathematical background |

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

### References

- [Interface Magazine article](https://interface.cqpub.co.jp/202509st/)
- [n_shinichi's inverted pendulum blog post](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903) — Technical guide and sample sketches for M5StickC Plus / Plus2 by the creator of the Interface Magazine kit
