# 🤖 copilot-cli-inverted-pendulum-m5stick

> **GitHub Copilot CLI** でファームウェア開発から書き込み・デバッグまで行う倒立振子プロジェクト

![Copilot CLI](https://img.shields.io/badge/Built_with-Copilot_CLI-8b5cf6?style=for-the-badge&labelColor=0a0a0c)
![M5StickC Plus](https://img.shields.io/badge/M5StickC-Plus-orange?style=for-the-badge)
![PID Control](https://img.shields.io/badge/PID-Control-blue?style=for-the-badge)

<p align="center">
  <a href="https://www.youtube.com/watch?v=MdE459HKGWA">
    <img src="https://img.youtube.com/vi/MdE459HKGWA/maxresdefault.jpg"
         alt="🎥 倒立振り子が実際に自立している様子 — クリックで YouTube 再生"
         width="720">
  </a>
  <br>
  <sub>🎥 <strong><a href="https://www.youtube.com/watch?v=MdE459HKGWA">▶︎ Watch on YouTube</a></strong> — M5StickC Plus が実際に自立し、ブラウザ UI に角度・PID 出力をリアルタイム表示している成功動画 / Real-world success: M5StickC Plus self-balancing while the browser dashboard streams live telemetry</sub>
</p>

<p align="center">
  <a href="https://ktanino10.github.io/copilot-cli-inverted-pendulum-m5stick/">
    <img src="https://img.shields.io/badge/%E2%96%B6%EF%B8%8E%20LAUNCH%20DEMO-Open%20in%20Browser-22c55e?style=for-the-badge&labelColor=0a0a0c&logo=github"
         alt="Launch the live demo on GitHub Pages"
         height="48">
  </a>
  &nbsp;
  <a href="https://ktanino10.github.io/copilot-cli-inverted-pendulum-m5stick/">
    <img src="https://img.shields.io/badge/PAGES-ktanino10.github.io-b866ff?style=for-the-badge&labelColor=0a0a0c"
         alt="GitHub Pages site URL"
         height="48">
  </a>
</p>

> 🚀 **▶︎ [Launch Demo](https://ktanino10.github.io/copilot-cli-inverted-pendulum-m5stick/)** — ハードがなくてもブラウザだけで触れるシミュレータ＋過去のテスト解析が見られます。詳しくは [§GitHub Pages デモを開く](#-github-pages-デモを開く)。

**[日本語](#日本語) | [English](#english)**

---

<a id="日本語"></a>
## 日本語

### 🚀 GitHub Pages デモを開く

| やりたいこと | リンク | 認証 |
|---|---|---|
| **🎮 デモを試す**（推奨スタート地点） | **[▶︎ Launch Demo](https://ktanino10.github.io/copilot-cli-inverted-pendulum-m5stick/)** | 不要 |
| 📊 同梱サンプルセッションを見る | デモ起動後、`📑 Sessions` パネル | 不要 |

> デモは「実機がいない」ことを `DEMO MODE` バッジで明示します。実機を持っていなくても、UI と過去テストの解析画面を一通り体験できます。「こういうインターフェースで操作していました」という見学用です。

### 概要

M5StickC Plus を使った倒立振子（自立バランスロボット）のプロジェクトです。
ファームウェアの作成、コンパイル、書き込み、デバッグまで、すべて **GitHub Copilot CLI** との対話で進めています。

### リポジトリ構成

このリポジトリの全体像です。**何がどこにあるか**、**どこから読めばいいか**を示します。

```
copilot-cli-inverted-pendulum-m5stick/
│
├── README.md                          ← 📌 今読んでいるファイル（概要・使い方・リンク集）
│
├── inverted_pendulum/
│   └── inverted_pendulum.ino          ← 🎯 メインファームウェア（PID制御 + カルマンフィルタ + WebUI）
│
├── servo_test/
│   └── servo_test.ino                 ← 🔧 サーボ動作確認用テストスケッチ
├── servo_diag/
│   └── servo_diag.ino                 ← 🔍 GPIO全ピン自動スキャン診断スケッチ
├── servo_calibrate/
│   └── servo_calibrate.ino            ← 🔧 サーボキャリブレーションツール
├── motor_dir_test/
│   └── motor_dir_test.ino             ← 🔧 モーター方向テスト / IMU軸ビューアー
├── octocat_display/
│   └── octocat_display.ino            ← 🐙 GitHub ステッカースライドショー（息抜き）
│
├── tools/                             ← 🛠️ ホストPC側ツール群
│   ├── server.py                      ← 🌐 Flask プロキシサーバ（PC ⇄ M5、ダッシュボード配信）
│   ├── build_demo.py                  ← 🏗️ docs/demo/ をテンプレートから生成
│   ├── monitor.py                     ← 📊 リアルタイムPIDモニター（シリアル）
│   ├── auto_tune.py                   ← 🔬 自動PIDパラメータスイープ
│   ├── collect_data.py                ← 📝 シリアルデータ収集
│   ├── visualize_root.py              ← 📈 CERN ROOT / matplotlib可視化
│   ├── upload.sh                      ← ⬆️ arduino-cli でファーム書き込み
│   ├── run_server.sh                  ← ▶️ server.py 起動ヘルパ
│   ├── start_tunnel.sh                ← 🛡️ Tailscale トンネル起動ヘルパ
│   ├── local_config.example.py        ← ⚙️ 個人設定のテンプレート（IP / パスワード等）
│   ├── requirements.txt               ← 🐍 Python 依存
│   ├── templates/                     ← 🎨 ダッシュボードのソース (index.html / dash.html / _demo_shim.js)
│   │                                     build_demo.py が docs/demo/ にビルドする元
│   └── data/                          ← 📂 収集データ・ヒートマップ画像・実機セッション記録（gitignore）
│       └── sessions/                  ← 🎮 倒立試行ごとのスコア＆波形 JSON/CSV
│
├── docs/                              ← 📚 ドキュメントと公開デモ
│   ├── pid_guide.md                   ← 🎓 PID制御 初心者ガイド
│   ├── pid_theory.md                  ← 📐 PID制御 理論編（上級者向け）
│   ├── glossary.md                    ← 📔 制御工学用語集
│   ├── wifi_communication.md          ← 📶 通信モデル・WiFi設計の解説
│   └── demo/                          ← 🌍 GitHub Pages 公開デモ（build_demo.py の出力）
│       ├── index.html                 ←   ガジェット風ホーム
│       ├── dash.html                  ←   STATS LAB / TEST ARCHIVE ダッシュボード
│       ├── _demo_shim.js              ←   /api/* をブラウザ内でモックする層
│       ├── data/sessions/             ←   実機キャプチャから生成された9件のセッション
│       └── assets/                    ←   GIF・画像
│
└── log/                               ← 📜 詳細な日次実験ログ（README から切り出し）
    └── progress-log.md                ← 試行錯誤の歴史（日本語＋English バイリンガル）
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

> 詳細な日次実験ログ（半田付け→ファーム移植→ハードウェアトラブル→IMU軸特定→PIDチューニング→GitHub Pages デモ化、2026-04-13〜2026-05-02）は **[`log/progress-log.md`](log/progress-log.md#japanese)** に切り出しました。

主なマイルストーン:

| 日付 | 内容 |
|------|------|
| 2026-04-13 | 半田付け完了・サーボ動作確認、Plus 故障 |
| 2026-04-16 | M5StickC Plus2 で復活・ファーム移植完了 |
| 2026-04-18 | pulse_drive で両輪駆動成功・IMU軸特定・42パターン PID スイープ |
| 2026-04-25 | n_shinichi 氏コード完全準拠リライト・atan2 境界問題解消 |
| 2026-05-01 | 「後ろへ逃げる」問題のログ化、転倒分類表の確立 |
| 2026-05-02 | チューニングUIの解析強化、マルチSSID + APフォールバック、Pages デモ公開 |

→ 詳細は **[`log/progress-log.md`](log/progress-log.md#japanese)** へ

### 🙏 謝辞

このプロジェクトは、以下の方々の先行された素晴らしいお仕事のおかげで成立しています。心より感謝申し上げます。

- **[しんさん (n_shinichi)](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903)** — Interface 誌の倒立振子キットの制作者であり、本リポジトリのファームウェアの土台となったサンプルスケッチと丁寧な解説記事を公開してくださっています。本プロジェクトの IMU 軸の取り扱い・PD 制御の構造・パルス駆動の考え方は、すべて n_shinichi 氏の公開コードと記事を参照して理解・移植したものです。
- **[Interface 誌 / CQ 出版](https://interface.cqpub.co.jp/202509st/)** — 「自分で動くハードウェア」を初学者でも組み立てて動かせる形に落とし込んだ素晴らしいキットと、その背景にある制御理論を読みやすくまとめてくださった編集・執筆チームの皆様。本キットがなければ、この学習体験そのものが存在しませんでした。

> 本リポジトリは上記の素晴らしい先行成果に対する敬意を持って、個人の学習・実験記録としてオープンにしているものです。商用流用や再配布を意図したものではなく、オリジナルの記事・キットの購入と参照を強く推奨します。

### 参考資料

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
├── README.md                          ← 📌 You are here (overview, usage, links)
│
├── inverted_pendulum/
│   └── inverted_pendulum.ino          ← 🎯 Main firmware (PID + Kalman filter + WebUI)
│
├── servo_test/
│   └── servo_test.ino                 ← 🔧 Servo test sketch
├── servo_diag/
│   └── servo_diag.ino                 ← 🔍 GPIO auto-scan diagnostic sketch
├── servo_calibrate/
│   └── servo_calibrate.ino            ← 🔧 Servo calibration tool
├── motor_dir_test/
│   └── motor_dir_test.ino             ← 🔧 Motor direction test / IMU axis viewer
├── octocat_display/
│   └── octocat_display.ino            ← 🐙 GitHub sticker slideshow (fun break)
│
├── tools/                             ← 🛠️ Host-side tools
│   ├── server.py                      ← 🌐 Flask proxy (PC ⇄ M5, dashboard host)
│   ├── build_demo.py                  ← 🏗️ Builds docs/demo/ from templates
│   ├── monitor.py                     ← 📊 Real-time PID monitor (serial)
│   ├── auto_tune.py                   ← 🔬 Automatic PID parameter sweep
│   ├── collect_data.py                ← 📝 Serial data collection
│   ├── visualize_root.py              ← 📈 CERN ROOT / matplotlib visualization
│   ├── upload.sh                      ← ⬆️ Flash firmware via arduino-cli
│   ├── run_server.sh                  ← ▶️ Helper to start server.py
│   ├── start_tunnel.sh                ← 🛡️ Helper to start Tailscale tunnel
│   ├── local_config.example.py        ← ⚙️ Template for personal config (IP / password)
│   ├── requirements.txt               ← 🐍 Python dependencies
│   ├── templates/                     ← 🎨 Dashboard source (index.html / dash.html / _demo_shim.js)
│   │                                     build_demo.py compiles these into docs/demo/
│   └── data/                          ← 📂 Captures, heatmaps, real-device sessions (gitignored)
│       └── sessions/                  ←   Per-attempt JSON/CSV with score & waveform
│
├── docs/                              ← 📚 Documentation & published demo
│   ├── pid_guide.md                   ← 🎓 PID Control — Beginner's Guide
│   ├── pid_theory.md                  ← 📐 PID Control — Theory (Advanced)
│   ├── glossary.md                    ← 📔 Control engineering glossary
│   ├── wifi_communication.md          ← 📶 Communication model & WiFi design
│   └── demo/                          ← 🌍 GitHub Pages public demo (output of build_demo.py)
│       ├── index.html                 ←   Gadget-styled landing page
│       ├── dash.html                  ←   STATS LAB / TEST ARCHIVE dashboard
│       ├── _demo_shim.js              ←   In-browser /api/* mock layer
│       ├── data/sessions/             ←   9 sessions generated from real captures
│       └── assets/                    ←   GIFs & images
│
└── log/                               ← 📜 Detailed daily experiment log (extracted from README)
    └── progress-log.md                ← History of trial-and-error (Japanese + English bilingual)
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

> The detailed daily experiment log (soldering → firmware port → hardware troubles → IMU axis identification → PID tuning → GitHub Pages demo, 2026-04-13 to 2026-05-02) has been moved to **[`log/progress-log.md`](log/progress-log.md#english)**.

Key milestones:

| Date | Summary |
|------|---------|
| 2026-04-13 | Soldering complete · servo test · Plus unit failed |
| 2026-04-16 | Recovered with M5StickC Plus2 · firmware ported |
| 2026-04-18 | pulse_drive both-wheels success · IMU axis identified · 42-pattern PID sweep |
| 2026-04-25 | Clean rewrite aligned to n_shinichi's code · atan2 boundary fix |
| 2026-05-01 | "Drifting backward" issue logged · fall classification table |
| 2026-05-02 | Tuning UI analytics · multi-SSID + AP fallback · GitHub Pages demo |

→ See **[`log/progress-log.md`](log/progress-log.md#english)** for full detail

### 🙏 Acknowledgments

This project would not exist without the wonderful prior work of the following creators. I am deeply grateful.

- **[n_shinichi (しんさん)](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903)** — Creator of the original Interface Magazine inverted pendulum kit, and author of the sample sketches and detailed blog post that this repository's firmware is built upon. The IMU axis handling, the PD control structure, and the pulse-drive approach used in this project are all derived from studying and porting n_shinichi's published code and articles.
- **[Interface Magazine / CQ Publishing](https://interface.cqpub.co.jp/202509st/)** — The editorial and writing team who packaged "hardware you can actually build and move" into a kit accessible to beginners, and who explained the underlying control theory in such a readable form. None of this learning experience would exist without this kit.

> This repository is a personal learning and experimentation record, made open with great respect for the prior work cited above. It is not intended for commercial reuse or redistribution. Please support the original by purchasing the magazine and the kit.

### References

- [Interface Magazine article](https://interface.cqpub.co.jp/202509st/)
- [n_shinichi's inverted pendulum blog post](https://n-shinichi.hatenablog.com/entry/2025/08/31/163903) — Technical guide and sample sketches for M5StickC Plus / Plus2 by the creator of the Interface Magazine kit
