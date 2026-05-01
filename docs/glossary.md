# 📖 用語集 / Glossary — 倒立振子 PID 制御
# Glossary — Inverted Pendulum PID Control

> `pid_theory.md` に登場する専門用語を、素人にも分かりやすく解説した補足資料集です。
>
> Supplementary reference explaining every technical term in `pid_theory.md` in plain language, suitable for complete beginners through intermediate learners.

**[日本語](#日本語) | [English](#english)**

---

<a id="日本語"></a>
## 日本語

---

### 目次

1. [物理・力学の基礎用語](#1-物理力学の基礎用語)
2. [制御理論の基礎](#2-制御理論の基礎)
3. [PID 制御の数学](#3-pid-制御の数学)
4. [状態空間表現](#4-状態空間表現)
5. [カルマンフィルタ・センサ融合](#5-カルマンフィルタセンサ融合)
6. [安定性解析](#6-安定性解析)
7. [ハードウェア・実装用語](#7-ハードウェア実装用語)
8. [発展的制御手法](#8-発展的制御手法)

---

### 1. 物理・力学の基礎用語

---

#### 倒立振子（とうりつふりこ）*Inverted Pendulum*

**一言で言うと**: 「逆さまに立てた棒をバランスさせるシステム」

通常の振り子は重心が支点より下にあるため、手を放しても自然に安定します。
「倒立振子」は重心が支点より**上**にある状態で、手を放すと必ず倒れます。
自転車に乗る、ほうきを手の上でバランスさせる、セグウェイが走るのはすべて倒立振子の原理です。

このプロジェクトでは、M5StickC を乗せた小型ロボットが「棒」で、
車輪を回してバランスを保つ様子がそのまま倒立振子です。

```
    ┌──────┐  ← M5StickC（重心）
    │      │
    │      │
    └──────┘
       │
    ───┼───  ← 支点（車輪軸）
      / \
```

> 📐 理論編の該当箇所: [Section 1. 倒立振子の運動方程式](pid_theory.md#1-倒立振子の運動方程式)

---

#### 運動方程式 *Equations of Motion*

**一言で言うと**: 「力と動きの関係を数式で書いたもの」

「押したら動く」「重力があれば落ちる」という物理の法則を数式にしたものです。
ニュートンの法則（F = ma）がもっとも有名な例です。

倒立振子では「棒がどのように傾くか」を表す方程式が必要で、
重力・慣性・車輪の動きをすべて含む複雑な式になります。

---

#### ラグランジュ方程式 / ラグランジアン *Lagrangian Mechanics*

**一言で言うと**: 「エネルギーの観点から運動方程式を導く方法」

ニュートン力学は「力」に注目しますが、ラグランジュ力学は
「運動エネルギー（動いている物体が持つエネルギー）」と
「位置エネルギー（高い場所にある物体が持つエネルギー）」の差（**ラグランジアン** <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathcal%7BL%7D%20%3D%20T%20-%20V" alt="\mathcal{L} = T - V" />）に注目します。

複雑なシステムでも、エネルギーの釣り合いを考えるだけで運動方程式が自動的に導けるのが特徴です。

> 📐 理論編の該当箇所: [Section 1.2 ラグランジュ方程式からの導出](pid_theory.md#12-ラグランジュ方程式からの導出)

---

#### 小角度近似（しょうかくどきんじ）*Small-Angle Approximation*

**一言で言うと**: 「傾きが小さいとき、sin(θ) ≈ θ と近似する」

倒立振子の運動方程式には <img src="https://latex.codecogs.com/svg.image?\inline%20%5Csin%5Ctheta" alt="\sin\theta" /> などの非線形な項が含まれており、
そのままでは計算が非常に難しくなります。

ロボットがほぼ直立しているとき（角度が小さいとき）は
<img src="https://latex.codecogs.com/svg.image?\inline%20%5Csin%5Ctheta%20%5Capprox%20%5Ctheta" alt="\sin\theta \approx \theta" />、<img src="https://latex.codecogs.com/svg.image?\inline%20%5Ccos%5Ctheta%20%5Capprox%201" alt="\cos\theta \approx 1" /> と近似しても誤差が非常に小さいです。
この近似によって方程式が大幅に単純化され、後の制御設計が容易になります。

| 角度 | <img src="https://latex.codecogs.com/svg.image?\inline%20%5Csin%5Ctheta" alt="\sin\theta" /> の真値 | 近似値 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" /> (rad) | 誤差 |
|------|-------------------|----------------------|------|
| 5°   | 0.0872            | 0.0873               | <0.1% |
| 10°  | 0.1736            | 0.1745               | 0.5% |
| 20°  | 0.3420            | 0.3491               | 2% |

> 📐 理論編の該当箇所: [Section 1.3 小角度近似による線形化](pid_theory.md#13-小角度近似による線形化)

---

#### 慣性モーメント <img src="https://latex.codecogs.com/svg.image?\inline%20I" alt="I" /> *Moment of Inertia*

**一言で言うと**: 「回転しにくさを表す量」

直線運動における「質量（重さ）」に相当するのが、回転運動における「慣性モーメント」です。
重い物体ほど動かしにくいのと同様に、慣性モーメントが大きいほど回転させにくく、止めにくいです。

棒のような形状では、重心から遠いほど慣性モーメントが大きくなります（<img src="https://latex.codecogs.com/svg.image?\inline%20I%20%3D%20ml%5E2" alt="I = ml^2" /> が典型例）。

---

#### 角速度 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" />、角加速度 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cddot%7B%5Ctheta%7D" alt="\ddot{\theta}" /> *Angular Velocity / Angular Acceleration*

**一言で言うと**: 「回転の速さ」「回転の加速度」

- **角速度 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" />**: 1 秒間に何ラジアン（rad）回転するか。時計の針が回る速さのイメージ。単位は rad/s。
- **角加速度 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cddot%7B%5Ctheta%7D" alt="\ddot{\theta}" />**: 角速度の変化率（加速・減速）。単位は rad/s²。
- ドット記号（˙）は「時間による微分（変化率）」を意味します。

---

#### ラジアン *Radian (rad)*

**一言で言うと**: 「角度の単位（°の代わりに使う数学・工学の標準単位）」

日常では「度（°）」を使いますが、数学・工学では弧長を半径で割った「ラジアン」を使います。
<img src="https://latex.codecogs.com/svg.image?\inline%20%5Cpi" alt="\pi" /> ラジアン ＝ 180°　なので、1 rad ≈ 57.3° です。

---

### 2. 制御理論の基礎

---

#### フィードバック制御 *Feedback Control*

**一言で言うと**: 「センサで現在の状態を測り、目標とのズレを修正し続ける仕組み」

```
目標値 ──→ [比較] ──→ [制御器] ──→ [アクチュエータ] ──→ システム
              ↑                                              |
              └──────────────── [センサ] ←──────────────────┘
```

人間がほうきをバランスさせるとき、目（センサ）で傾きを見て、
手（アクチュエータ）で修正するのが自然なフィードバック制御です。

---

#### 開ループ系 / 閉ループ系 *Open-Loop / Closed-Loop System*

**一言で言うと**: 「センサによる測定なし」 vs 「センサによる測定あり」

| 種類 | センサフィードバック | 例 |
|------|------------------|-----|
| 開ループ | なし | 電子レンジで「3分」セットして待つ |
| 閉ループ | あり | エアコンが温度を測りながら自動調節 |

倒立振子は**閉ループ制御が必須**です。センサなしでは一瞬で倒れてしまいます。

---

#### 制御入力 <img src="https://latex.codecogs.com/svg.image?\inline%20u" alt="u" /> *Control Input*

**一言で言うと**: 「システムへの命令（例: モータへの電力量）」

制御器が計算した「こうしろ」という値です。
倒立振子では「車輪を左右どちらに、どれくらいの速さで回せ」という命令に相当します。

---

#### フィードフォワード / フィードバック *Feedforward / Feedback*

- **フィードバック**: センサで現在の状態を測って修正する（事後対応）
- **フィードフォワード**: 予測に基づいてあらかじめ入力する（事前対応）

PID 制御はフィードバック制御の代表例です。

---

### 3. PID 制御の数学

---

#### PID 制御 *PID Control*

**一言で言うと**: 「誤差の現在・過去・未来を見て制御量を決める手法」

<div align="center"><img src="https://latex.codecogs.com/svg.image?u%28t%29%20%3D%20K_p%20e%28t%29%20%2B%20K_i%20%5Cint%20e%5C%2Cdt%20%2B%20K_d%20%5Cfrac%7Bde%7D%7Bdt%7D" alt="u(t) = K_p e(t) + K_i \int e\,dt + K_d \frac{de}{dt}" /></div>

| 記号 | 名前 | 役割 | 直感的なイメージ |
|------|------|------|----------------|
| <img src="https://latex.codecogs.com/svg.image?\inline%20K_p" alt="K_p" /> | 比例ゲイン | 今の誤差に比例した出力 | 傾いた分だけ修正 |
| <img src="https://latex.codecogs.com/svg.image?\inline%20K_i" alt="K_i" /> | 積分ゲイン | 誤差の蓄積（過去）を補正 | 少しずつ蓄積した傾きを解消 |
| <img src="https://latex.codecogs.com/svg.image?\inline%20K_d" alt="K_d" /> | 微分ゲイン | 誤差の変化速度（未来予測）にブレーキ | 倒れそうな速さを感知して先に手を打つ |

> 📐 理論編の該当箇所: [Section 3. PID制御の数理](pid_theory.md#3-pid制御の数理)

---

#### 比例項（P項）*Proportional Term*

**一言で言うと**: 「今どれだけズレているかに比例した修正力」

誤差が 2 倍なら出力も 2 倍。シンプルで直感的ですが、
P だけでは「行き過ぎ（オーバーシュート）」や「残留誤差（定常偏差）」が残ることがあります。

---

#### 積分項（I項）*Integral Term*

**一言で言うと**: 「過去の誤差をすべて足し合わせて、じわじわ残った誤差を消す」

例: 毎回 0.1° ずつ傾いていても P だけでは完全に補正できない場合、
I 項がその傾きを蓄積して「全体として正の誤差がある」と判断して補正します。

**積分器ワインドアップ（Integral Windup）**

積分値が際限なく大きくなる問題。例えばモータが飽和（最大出力）しているとき、
積分値だけが増え続け、飽和から回復したときに急激なオーバーシュートを引き起こします。
対策: 積分値の上下限をクランプ（制限）します（本プロジェクトでは `INTEGRAL_LIMIT = 100.0`）。

---

#### 微分項（D項）*Derivative Term*

**一言で言うと**: 「誤差がどれだけ速く変化しているかを見てブレーキをかける」

ブレーキの先読み機能です。「今は小さいズレでも、急激に大きくなっているなら早めに対応」します。
ジャイロ（角速度センサ）の値を直接使うと、D 項の計算が非常に簡単になります。

**微分キック（Derivative Kick）**

目標値（<img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta_%7B%5Ctext%7Bref%7D%7D" alt="\theta_{\text{ref}}" />）が突然変わったとき、誤差 <img src="https://latex.codecogs.com/svg.image?\inline%20e%20%3D%20%5Ctheta_%7B%5Ctext%7Bref%7D%7D%20-%20%5Ctheta" alt="e = \theta_{\text{ref}} - \theta" /> が瞬間的に急変し、
微分値 <img src="https://latex.codecogs.com/svg.image?\inline%20de%2Fdt" alt="de/dt" /> が無限大（スパイク）になる問題。
対策: **プロセス変数の微分**（測定値 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" /> の変化率 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" /> をジャイロから読む）を使います。

---

#### 離散化 *Discretization*

**一言で言うと**: 「連続時間の数式を、コンピュータが使えるデジタル式に変換すること」

マイコンは連続した時間を扱えません。決まった周期（例: 10 ms ごと）で計算します。
積分は「面積の合計（Σ）」に、微分は「差分（<img src="https://latex.codecogs.com/svg.image?\inline%20%5Cfrac%7Be%5Bn%5D-e%5Bn-1%5D%7D%7B%5CDelta%20t%7D" alt="\frac{e[n]-e[n-1]}{\Delta t}" />）」に置き換えます。

---

#### ゲイン *Gain*

**一言で言うと**: 「増幅率・感度」

<img src="https://latex.codecogs.com/svg.image?\inline%20K_p%20%3D%2010" alt="K_p = 10" /> なら「誤差 1 に対して出力 10」。大きいほど敏感な制御になります。
大きすぎると振動し、小さすぎると反応が遅くなります。

---

#### 定常偏差（ていじょうへんさ）*Steady-State Error*

**一言で言うと**: 「制御が落ち着いた後も残るズレ」

P 制御だけだと、目標にほぼ到達しても少しズレが残ることがあります。
I 制御（積分）がこの残留ズレを徐々に除去します。

---

#### 伝達関数 *Transfer Function*

**一言で言うと**: 「入力と出力の関係を周波数領域で表した関数」

ラプラス変換を使って、制御系の「動的な性質（速い・遅い、振動するかどうか）」を
数式一つで表現します。PID の伝達関数は:

<div align="center"><img src="https://latex.codecogs.com/svg.image?C%28s%29%20%3D%20K_p%20%2B%20%5Cfrac%7BK_i%7D%7Bs%7D%20%2B%20K_d%20s" alt="C(s) = K_p + \frac{K_i}{s} + K_d s" /></div>

- <img src="https://latex.codecogs.com/svg.image?\inline%20s" alt="s" /> はラプラス演算子（複素数の周波数変数）
- <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cfrac%7BK_i%7D%7Bs%7D" alt="\frac{K_i}{s}" /> の部分が「<img src="https://latex.codecogs.com/svg.image?\inline%20s%20%3D%200" alt="s = 0" />（直流＝定常）で無限大の利得を持つ」→ 定常偏差がゼロになる直感的な説明

---

#### ラプラス変換 *Laplace Transform*

**一言で言うと**: 「時間領域の微分方程式を、代数的に解きやすい形に変換する数学的ツール」

<img src="https://latex.codecogs.com/svg.image?\inline%20%5Cfrac%7Bd%7D%7Bdt%7D" alt="\frac{d}{dt}" /> が <img src="https://latex.codecogs.com/svg.image?\inline%20s" alt="s" />（掛け算）に変わるため、微分方程式が代数方程式になり計算が楽になります。
電気回路や制御工学で広く使われます。日常の類似例: 対数を使って掛け算を足し算に変換するのと同じ発想。

---

### 4. 状態空間表現

---

#### 状態空間表現 *State-Space Representation*

**一言で言うと**: 「システムの "状態（今の様子）" をベクトルで表し、行列を使って動きを記述する方法」

PID の伝達関数と並ぶ、もう一つの制御系記述法です。複数の変数を同時に扱えるのが特徴。

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cdot%7B%5Cmathbf%7Bx%7D%7D%20%3D%20A%5Cmathbf%7Bx%7D%20%2B%20Bu%2C%20%5Cquad%20%5Cmathbf%7By%7D%20%3D%20C%5Cmathbf%7Bx%7D" alt="\dot{\mathbf{x}} = A\mathbf{x} + Bu, \quad \mathbf{y} = C\ma..." /></div>

- <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathbf%7Bx%7D" alt="\mathbf{x}" />: **状態ベクトル**（システムの今の様子を記述する変数の組）
- <img src="https://latex.codecogs.com/svg.image?\inline%20A" alt="A" />: **システム行列**（状態がどう変化するかを記述）
- <img src="https://latex.codecogs.com/svg.image?\inline%20B" alt="B" />: **入力行列**（制御入力がどう影響するか）
- <img src="https://latex.codecogs.com/svg.image?\inline%20C" alt="C" />: **出力行列**（センサで観測できる量を選ぶ）

> 📐 理論編の該当箇所: [Section 2. 状態空間表現](pid_theory.md#2-状態空間表現)

---

#### 状態ベクトル <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathbf%7Bx%7D" alt="\mathbf{x}" /> *State Vector*

**一言で言うと**: 「システムの現在の "状態" を記述するすべての変数をひとまとめにしたもの」

倒立振子の場合:

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cmathbf%7Bx%7D%20%3D%20%5Cbegin%7Bbmatrix%7D%20%5Ctheta%20%5C%5C%20%5Cdot%7B%5Ctheta%7D%20%5C%5C%20x_w%20%5C%5C%20%5Cdot%7Bx%7D_w%20%5Cend%7Bbmatrix%7D%0A%3D%20%5Cbegin%7Bbmatrix%7D%20%5Ctext%7B%E5%82%BE%E6%96%9C%E8%A7%92%7D%20%5C%5C%20%5Ctext%7B%E8%A7%92%E9%80%9F%E5%BA%A6%7D%20%5C%5C%20%5Ctext%7B%E8%BB%8A%E8%BC%AA%E4%BD%8D%E7%BD%AE%7D%20%5C%5C%20%5Ctext%7B%E8%BB%8A%E8%BC%AA%E9%80%9F%E5%BA%A6%7D%20%5Cend%7Bbmatrix%7D" alt="\mathbf{x} = \begin{bmatrix} \theta \\ \dot{\theta} \\ x_w \..." /></div>

これら 4 つを知れば、次の瞬間にシステムがどう動くかが計算できます。

---

#### 可制御性（かせいぎょせい）*Controllability*

**一言で言うと**: 「制御入力だけで、どんな状態にも持っていけるか？」

可制御性が確認できれば、「倒立振子をどんな角度・位置からでも直立に戻せる（理論上）」ことが保証されます。
可制御性行列 <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathcal%7BC%7D%20%3D%20%5BB%5C%20%7C%5C%20AB%5C%20%7C%5C%20A%5E2B%5C%20%7C%5C%20A%5E3B%5D" alt="\mathcal{C} = [B\ |\ AB\ |\ A^2B\ |\ A^3B]" /> のランクを確認して判定します。

---

#### 可観測性（かかんそくせい）*Observability*

**一言で言うと**: 「センサで計測できる量から、すべての状態が推定できるか？」

倒立振子の IMU は <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" />（角度）と <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" />（角速度）しか直接測れません。
可観測性が確認できれば、その観測値だけから <img src="https://latex.codecogs.com/svg.image?\inline%20x_w" alt="x_w" />（車輪位置）も推定できます。

---

#### 極配置（きょくはいち）*Pole Placement*

**一言で言うと**: 「閉ループ系の応答特性（速さ・振動）を自由に設計する手法」

系の「極（特性方程式の根）」を、設計者が望む場所に配置するようなフィードバックゲインを計算します。
可制御性があれば、理論上どんな場所にも極を置けます。

---

#### オブザーバ（状態推定器）*Observer / State Estimator*

**一言で言うと**: 「センサで直接測れない状態変数を数学的に推定する仕組み」

車輪の位置 <img src="https://latex.codecogs.com/svg.image?\inline%20x_w" alt="x_w" /> は加速度センサからは直接わかりませんが、
系のモデルと IMU の測定値を組み合わせることで推定できます。
カルマンフィルタはオブザーバの一種です。

---

### 5. カルマンフィルタ・センサ融合

---

#### カルマンフィルタ *Kalman Filter*

**一言で言うと**: 「ノイズのある複数のセンサを最適に組み合わせて、真値を推定するアルゴリズム」

GPS と加速度計を組み合わせてスマートフォンの位置を求めるのにも使われる技術です。
「予測」と「更新」を繰り返し、確率的に最も確からしい状態を推定します。

**予測ステップ**: 前の状態から今の状態を予測（ジャイロを使って角度を積分）

**更新ステップ**: 実際のセンサ値（加速度センサの角度）で予測を修正

> 📐 理論編の該当箇所: [Section 5. カルマンフィルタ](pid_theory.md#5-カルマンフィルタ)

---

#### 共分散行列 <img src="https://latex.codecogs.com/svg.image?\inline%20P" alt="P" /> *Covariance Matrix*

**一言で言うと**: 「推定の不確かさ（誤差の広がり）を表す行列」

値が大きいほど「推定値への自信がない」ことを意味します。
カルマンフィルタは、予測と観測の不確かさを比較して、
自信のある情報により多くの重みをつけて推定値を更新します。

---

#### プロセスノイズ <img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" /> / 観測ノイズ <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" /> *Process Noise / Measurement Noise*

**一言で言うと**: 「モデルのズレ」と「センサの誤差」を確率的に表したもの」

- **<img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" />（プロセスノイズ）**: 「モデル通りには動かない」という不確かさ（例: 路面のでこぼこ）
- **<img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" />（観測ノイズ）**: センサ自身の測定誤差（例: 加速度センサのノイズ）

<img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" /> が大きい → モデルよりセンサを信じる  
<img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" /> が大きい → センサより予測モデルを信じる

---

#### カルマンゲイン <img src="https://latex.codecogs.com/svg.image?\inline%20K_k" alt="K_k" /> *Kalman Gain*

**一言で言うと**: 「予測とセンサどちらをどれだけ信じるかを決める重み」

<img src="https://latex.codecogs.com/svg.image?\inline%20K_k" alt="K_k" /> が 0 に近い → 予測値をそのまま使う（センサを信じない）  
<img src="https://latex.codecogs.com/svg.image?\inline%20K_k" alt="K_k" /> が 1 に近い → センサ値をそのまま使う（予測を信じない）

毎ステップ、<img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" /> と <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" /> の比率に基づいて自動的に計算されます。

---

#### イノベーション *Innovation*

**一言で言うと**: 「予測値とセンサ実測値のズレ（新しく得られた情報）」

<div align="center"><img src="https://latex.codecogs.com/svg.image?y_k%20%3D%20z_k%20-%20H%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D" alt="y_k = z_k - H\hat{\mathbf{x}}_{k|k-1}" /></div>

これがゼロなら「予測通り」、大きければ「予測が外れた」ことを意味します。
カルマンフィルタはこのズレをゲイン <img src="https://latex.codecogs.com/svg.image?\inline%20K_k" alt="K_k" /> で重み付けして状態を修正します。

---

#### ジャイロバイアス <img src="https://latex.codecogs.com/svg.image?\inline%20b" alt="b" /> *Gyro Bias*

**一言で言うと**: 「ジャイロが静止しているのに出てしまう誤った角速度値」

安価な MEMS ジャイロは、温度や経年変化によりゼロのはずの値がずれます（ドリフト）。
カルマンフィルタはこのバイアスも状態変数として推定・補正します。

---

#### 相補フィルタ *Complementary Filter*

**一言で言うと**: 「加速度計とジャイロを単純に重み付け合成する、カルマンの簡易版」

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Ctheta_%7B%5Ctext%7Best%7D%7D%20%3D%20%5Calpha%28%5Ctheta_%7B%5Ctext%7Bprev%7D%7D%20%2B%20%5Cdot%7B%5Ctheta%7D_%7B%5Ctext%7Bgyro%7D%7D%20%5Ccdot%20%5CDelta%20t%29%20%2B%20%281%20-%20%5Calpha%29%5Ctheta_%7B%5Ctext%7Bacc%7D%7D" alt="\theta_{\text{est}} = \alpha(\theta_{\text{prev}} + \dot{\th..." /></div>

- <img src="https://latex.codecogs.com/svg.image?\inline%20%5Calpha" alt="\alpha" /> は 0 〜 1 の重み（例: 0.98 = ジャイロを 98% 信頼）
- 計算が軽いが、バイアス補正は自動ではなく、チューニングが必要

| | 相補フィルタ | カルマンフィルタ |
|---|---|---|
| 計算量 | 極めて軽い | やや重い（行列演算） |
| 精度 | 実用的に十分 | 理論的に最適 |
| 実装難易度 | 簡単 | やや複雑 |

---

#### IMU（慣性計測ユニット）*IMU — Inertial Measurement Unit*

**一言で言うと**: 「加速度とジャイロを一体化したセンサモジュール」

スマートフォンが縦横を検知したり、ドローンが姿勢を保ったりするのに使われます。
このプロジェクトでは M5StickC 内蔵の **MPU6886** を使用します。

| センサ | 測定量 | 長所 | 短所 |
|--------|--------|------|------|
| 加速度センサ | <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" />（重力から角度を計算） | 長期安定（ドリフトなし） | 振動・衝撃に弱い |
| ジャイロスコープ | <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" />（角速度を直接測定） | 高速・ノイズに強い | 長期ドリフトあり |

---

### 6. 安定性解析

---

#### 安定性 *Stability*

**一言で言うと**: 「外から少し乱されても元に戻ろうとする性質があるかどうか」

倒立振子は制御なし（開ループ）では**不安定**（少し傾けると倒れる）。
適切な PID 制御（閉ループ）にすれば**安定**（元の直立状態に戻る）になります。

---

#### 特性方程式 *Characteristic Equation*

**一言で言うと**: 「システムの動き（応答特性）を決定する方程式」

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cdet%28sI%20-%20A%29%20%3D%200" alt="\det(sI - A) = 0" /></div>

この方程式の根（**極**）が複素平面の左半分にあれば安定、右半分にあれば不安定です。

---

#### 極（きょく）*Poles*

**一言で言うと**: 「システムの振動・減衰特性を決める特性値（特性方程式の解）」

| 極の位置 | 挙動 |
|---------|------|
| 負の実数（左半平面） | 安定（指数減衰） |
| 正の実数（右半平面） | 不安定（指数発散） |
| 純虚数（虚軸上） | 振動し続ける（境界） |
| 複素数（実部が負） | 減衰振動（安定） |

---

#### フルビッツの安定判別法 *Hurwitz Stability Criterion*

**一言で言うと**: 「特性方程式の係数を見るだけで、実際に解を求めずに安定かどうかを判定する方法」

全係数が正であることなどが安定の必要条件となります。
倒立振子の PD 制御では:
- <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cbeta%20K_d%20%3E%200" alt="\beta K_d > 0" />（減衰係数が正）
- <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cbeta%20K_p%20%3E%20%5Calpha" alt="\beta K_p > \alpha" />（比例ゲインが重力の影響 <img src="https://latex.codecogs.com/svg.image?\inline%20g%2Fl" alt="g/l" /> を上回る）

---

#### 位相余裕（いそうよゆう）*Phase Margin*

**一言で言うと**: 「あとどれだけ遅延（位相遅れ）が増えると不安定になるかの余裕」

実用的な目安: **30° 以上**を確保する。
低いと、センサの遅延やサーボの応答遅れで簡単に不安定になります。

---

#### ゲイン余裕 *Gain Margin*

**一言で言うと**: 「あとどれだけゲインを上げると不安定になるかの余裕（dB 単位）」

実用的な目安: **6 dB 以上**（2 倍の余裕）を確保する。
倒立振子の実機では RCサーボの非線形性が支配的で、これらは参考値にとどまります。

---

#### ボード線図 *Bode Plot*

**一言で言うと**: 「周波数ごとのゲイン（大きさ）と位相を表したグラフ」

どの周波数の外乱に強く、どこで不安定になりやすいかを視覚的に把握できます。
横軸が周波数（対数）、縦軸がゲイン (dB) / 位相 (°) の 2 枚のグラフで表示されます。

---

### 7. ハードウェア・実装用語

---

#### PWM（パルス幅変調）*PWM — Pulse Width Modulation*

**一言で言うと**: 「パルスのON時間の幅を変えることでモータへの電力量を制御する方法」

完全にON/OFFする信号のON時間の割合（**デューティ比**）を変えることで、
平均電圧を変化させます。スイッチを秒間 100 回入切して、そのうち 70 回 ON なら 70% の出力相当になります。

RCサーボでは **1000〜2000 μs のパルス幅** で中立・正転・逆転を指定します。

---

#### RCサーボ / 連続回転サーボ *RC Servo / Continuous Rotation Servo*

**一言で言うと**: 「ラジコン用の小型モータ内蔵アクチュエータ」

通常のRCサーボは角度制御（0〜180°）ですが、本プロジェクトの **FS90R** は連続回転型で、
PWM パルス幅で**回転速度と方向**を制御します。

| パルス幅 | 動作 |
|---------|------|
| 1500 μs | 停止（中立） |
| 1000 μs | 一方向に最高速回転 |
| 2000 μs | 逆方向に最高速回転 |

---

#### デッドバンド *Deadband*

**一言で言うと**: 「入力が小さすぎてアクチュエータが反応しない不感帯」

モータや弁などの機械部品は、入力が一定の閾値を超えないと動きません。
サーボの場合 ±5° 付近がデッドバンドで、この範囲内の制御指令は無視されます。
`kpower` パラメータでPID出力をスケーリングしてデッドバンドを超えるようにします。

---

#### バックラッシュ *Backlash*

**一言で言うと**: 「歯車のかみ合わせのガタつき（遊び）」

歯車が反転するとき、ガタの分だけ空走が生じます。これが振動の原因になります。
D 項（微分ゲイン <img src="https://latex.codecogs.com/svg.image?\inline%20K_d" alt="K_d" />）を高めにすることで振動を抑制できます。

---

#### I²C *I²C (Inter-Integrated Circuit)*

**一言で言うと**: 「マイコンとセンサが通信するための 2 本線シリアル通信規格」

SDA（データ）と SCL（クロック）の 2 本だけで複数のデバイスと通信できます。
ESP32 から MPU6886（IMU）のデータを読み取るのに使用しています。
通信速度は 400 kHz が標準で、IMU の読み取りに約 1 ms かかります。

---

#### M5StickC Plus / ESP32 *Microcontroller Unit*

**一言で言うと**: 「頭脳となる小型コンピュータ（マイコンボード）」

- **ESP32**: Wi-Fi/BLE 搭載の高性能 MCU（240 MHz デュアルコア）。Arduino IDE で開発可能
- **M5StickC Plus**: ESP32 に IMU・LCD・バッテリを内蔵した小型ボード
- IMU として **MPU6886**（6 軸: 3 軸加速度 + 3 軸ジャイロ）を搭載

---

#### 制御周期 / サンプリング周期 <img src="https://latex.codecogs.com/svg.image?\inline%20%5CDelta%20t" alt="\Delta t" /> *Control Period / Sampling Period*

**一言で言うと**: 「センサを読んで制御計算して出力するまでのサイクルの繰り返し時間」

本プロジェクトでは <img src="https://latex.codecogs.com/svg.image?\inline%20%5CDelta%20t%20%3D%2010%5C%2C%5Ctext%7Bms%7D" alt="\Delta t = 10\,\text{ms}" />（100 Hz）。
短いほど制御精度が上がりますが、計算・通信の時間が必要です。
ESP32 では 100 Hz ループは余裕を持って動作します。

---

#### スケーリング係数 <img src="https://latex.codecogs.com/svg.image?\inline%20K_%7B%5Ctext%7Bpower%7D%7D" alt="K_{\text{power}}" /> *Scaling Factor*

**一言で言うと**: 「PID が計算した数値を、実際のサーボPWM値に変換する比率」

PID の出力（純粋な数値計算）をそのままサーボに送っても、
単位が合わないため使えません。`kpower = 0.001` はこの変換スケールです。

---

### 8. 発展的制御手法

---

#### LQR（線形二次レギュレータ）*LQR — Linear Quadratic Regulator*

**一言で言うと**: 「コスト関数を最小化するように自動的にゲインを決める最適制御手法」

PID では各ゲイン（<img src="https://latex.codecogs.com/svg.image?\inline%20K_p%2C%20K_d" alt="K_p, K_d" /> 等）を人間が試行錯誤で決めますが、
LQR ではコスト行列 <img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" />（状態偏差の重み）と <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" />（入力の重み）を設定すると、
リカッチ方程式を解くことで最適なフィードバックゲインが自動計算されます。

<div align="center"><img src="https://latex.codecogs.com/svg.image?J%20%3D%20%5Cint_0%5E%5Cinfty%20%28%5Cmathbf%7Bx%7D%5ET%20Q%20%5Cmathbf%7Bx%7D%20%2B%20u%5ET%20R%20u%29%5C%2Cdt%20%5Cquad%20%5Ctext%7B%E3%82%92%E6%9C%80%E5%B0%8F%E5%8C%96%7D" alt="J = \int_0^\infty (\mathbf{x}^T Q \mathbf{x} + u^T R u)\,dt ..." /></div>

> 📐 理論編の該当箇所: [Section 9.1 LQR](pid_theory.md#91-lqr線形二次レギュレータ)

---

#### モデル予測制御（MPC）*MPC — Model Predictive Control*

**一言で言うと**: 「将来の動きを予測しながら、毎ステップ最適な入力を計算する制御手法」

「今の入力が N ステップ後にどう影響するか」を毎回シミュレーションして、
最もコストが低くなる入力列を求めます。角度制限・モータ飽和などの制約を明示的に扱えます。
ただし計算量が多く、マイコンへの実装には工夫が必要です。

> 📐 理論編の該当箇所: [Section 9.2 モデル予測制御](pid_theory.md#92-モデル予測制御mpc)

---

#### ジーグラー・ニコルズ法 *Ziegler–Nichols Method*

**一言で言うと**: 「実験的に PID ゲインを決める古典的なチューニング手法」

P ゲインを徐々に上げて持続振動（限界ゲイン <img src="https://latex.codecogs.com/svg.image?\inline%20K_u" alt="K_u" />）を起こし、
その周期 <img src="https://latex.codecogs.com/svg.image?\inline%20T_u" alt="T_u" /> から P/I/D ゲインを経験則で計算します。

<div align="center"><img src="https://latex.codecogs.com/svg.image?K_p%20%3D%200.6%20K_u%2C%20%5Cquad%20K_i%20%3D%20%5Cfrac%7B1.2%20K_u%7D%7BT_u%7D%2C%20%5Cquad%20K_d%20%3D%200.075%20K_u%20T_u" alt="K_p = 0.6 K_u, \quad K_i = \frac{1.2 K_u}{T_u}, \quad K_d = ..." /></div>

⚠️ 倒立振子は開ループ不安定なため、「持続振動」を安全に観察できず、直接の適用は困難です。

---

#### リカッチ方程式 *Algebraic Riccati Equation*

**一言で言うと**: 「LQR の最適ゲインを求めるために解く行列方程式」

<div align="center"><img src="https://latex.codecogs.com/svg.image?A%5ET%20P%20%2B%20PA%20-%20PBR%5E%7B-1%7DB%5ET%20P%20%2B%20Q%20%3D%200" alt="A^T P + PA - PBR^{-1}B^T P + Q = 0" /></div>

この方程式の解 <img src="https://latex.codecogs.com/svg.image?\inline%20P" alt="P" /> から最適フィードバックゲイン <img src="https://latex.codecogs.com/svg.image?\inline%20K%20%3D%20R%5E%7B-1%7DB%5ET%20P" alt="K = R^{-1}B^T P" /> が得られます。
MATLAB や Python（scipy）の関数を使って数値的に解きます。

---

#### 強化学習 *Reinforcement Learning*

**一言で言うと**: 「試行錯誤を繰り返して、「報酬を最大化する行動」を学習するAI手法」

物理モデルを事前に用意せず、ロボットが転んだり立ったりを繰り返す中で
最適な制御方策を自動的に学習します。
「シミュレータで学習した方策を実機に転移する（Sim-to-Real）」手法が研究されています。

---

## 📚 関連ドキュメント

| ドキュメント | 内容 | 難易度 |
|------------|------|--------|
| [pid_guide.md](pid_guide.md) | 初心者向け PID 制御の直感的解説 | ⭐ |
| [pid_theory.md](pid_theory.md) | 数式・状態空間・カルマンフィルタの詳細理論 | ⭐⭐⭐ |
| **glossary.md**（本書） | 専門用語の平易な解説 | ⭐ → ⭐⭐⭐ |

---

<a id="english"></a>
## English

---

### Table of Contents

1. [Physics & Mechanics Basics](#1-physics--mechanics-basics)
2. [Control Theory Fundamentals](#2-control-theory-fundamentals)
3. [PID Control Mathematics](#3-pid-control-mathematics)
4. [State-Space Representation](#4-state-space-representation)
5. [Kalman Filter & Sensor Fusion](#5-kalman-filter--sensor-fusion)
6. [Stability Analysis](#6-stability-analysis)
7. [Hardware & Implementation Terms](#7-hardware--implementation-terms)
8. [Advanced Control Methods](#8-advanced-control-methods)

---

### 1. Physics & Mechanics Basics

---

#### Inverted Pendulum

**In one sentence**: A stick balanced upright on a moving base — unstable by nature, controlled by feedback.

Unlike a normal pendulum (stable at rest, center of mass *below* the pivot), an inverted pendulum has its center of mass **above** the pivot. Without active control it immediately falls over. Riding a bicycle, balancing a broom on your palm, or riding a Segway all use the same principle.

In this project, the M5StickC-mounted robot body is the "stick," and the wheels are driven to maintain balance.

> 📐 Theory reference: [Section 1. Equations of Motion](pid_theory.md#1-equations-of-motion)

---

#### Equations of Motion

**In one sentence**: Mathematical expressions that relate forces (and torques) to accelerations.

Newton's second law (<img src="https://latex.codecogs.com/svg.image?\inline%20F%20%3D%20ma" alt="F = ma" />) is the simplest example. For an inverted pendulum, the equation must account for gravity, inertia, and the horizontal force from the wheels — leading to a nonlinear differential equation.

---

#### Lagrangian Mechanics

**In one sentence**: A method to derive equations of motion from *energy* rather than forces.

The **Lagrangian** is defined as <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathcal%7BL%7D%20%3D%20T%20-%20V" alt="\mathcal{L} = T - V" /> (kinetic energy minus potential energy). Applying the Euler–Lagrange equation to <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathcal%7BL%7D" alt="\mathcal{L}" /> automatically yields the equations of motion — very convenient for complex, coupled systems.

> 📐 Theory reference: [Section 1.2](pid_theory.md#12-derivation-from-lagrangian-mechanics)

---

#### Small-Angle Approximation

**In one sentence**: Near vertical (<img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta%20%5Capprox%200" alt="\theta \approx 0" />), replace <img src="https://latex.codecogs.com/svg.image?\inline%20%5Csin%5Ctheta" alt="\sin\theta" /> with <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" /> and <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ccos%5Ctheta" alt="\cos\theta" /> with 1.

This approximation converts the nonlinear equation of motion into a *linear* one, enabling the use of powerful linear control techniques (state-space, LQR, Kalman filter). The error is less than 1% for angles below 10°.

| Angle | True <img src="https://latex.codecogs.com/svg.image?\inline%20%5Csin%5Ctheta" alt="\sin\theta" /> | Approx. <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" /> (rad) | Error |
|-------|-----------------|------------------------|-------|
| 5°    | 0.0872          | 0.0873                 | <0.1% |
| 10°   | 0.1736          | 0.1745                 | 0.5% |
| 20°   | 0.3420          | 0.3491                 | 2%   |

> 📐 Theory reference: [Section 1.3](pid_theory.md#13-linearization-small-angle-approximation)

---

#### Moment of Inertia <img src="https://latex.codecogs.com/svg.image?\inline%20I" alt="I" />

**In one sentence**: The rotational equivalent of mass — how hard it is to start or stop spinning.

A heavier or more spatially distributed mass increases <img src="https://latex.codecogs.com/svg.image?\inline%20I" alt="I" />, making angular acceleration harder. For a thin rod of mass <img src="https://latex.codecogs.com/svg.image?\inline%20m" alt="m" /> and length <img src="https://latex.codecogs.com/svg.image?\inline%20l" alt="l" /> pivoted at one end: <img src="https://latex.codecogs.com/svg.image?\inline%20I%20%3D%20%5Cfrac%7B1%7D%7B3%7Dml%5E2" alt="I = \frac{1}{3}ml^2" />.

---

#### Angular Velocity <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" /> & Angular Acceleration <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cddot%7B%5Ctheta%7D" alt="\ddot{\theta}" />

- **Angular velocity <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" />**: Rate of rotation (radians per second). A clock hand that completes one revolution per minute has <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D%20%5Capprox%200.105" alt="\dot{\theta} \approx 0.105" /> rad/s.
- **Angular acceleration <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cddot%7B%5Ctheta%7D" alt="\ddot{\theta}" />**: Rate of change of angular velocity (rad/s²).
- The dot notation (˙) denotes differentiation with respect to time.

---

#### Radian (rad)

**In one sentence**: The SI unit of angle, equal to the arc length divided by the radius.

<img src="https://latex.codecogs.com/svg.image?\inline%20%5Cpi" alt="\pi" /> rad = 180°, so 1 rad ≈ 57.3°. Radians are used in physics and engineering because they make calculus formulas cleaner (no conversion factors).

---

### 2. Control Theory Fundamentals

---

#### Feedback Control

**In one sentence**: Continuously measure the system output, compare with the target, and correct the error.

```
Reference ──→ [Error] ──→ [Controller] ──→ [Actuator] ──→ System
                ↑                                            |
                └──────────── [Sensor] ◄────────────────────┘
```

Human balance is the best analogy: your eyes (sensor) detect tilt, your brain (controller) calculates corrections, and your legs (actuators) execute them.

---

#### Open-Loop vs. Closed-Loop

| Type | Sensor Feedback | Example |
|------|----------------|---------|
| Open-loop | None | Setting a microwave timer and walking away |
| Closed-loop | Yes | A thermostat that measures room temperature |

An inverted pendulum **requires** closed-loop control — it will fall within milliseconds without it.

---

#### Control Input <img src="https://latex.codecogs.com/svg.image?\inline%20u" alt="u" />

**In one sentence**: The output of the controller — the "command" sent to the actuator.

For the inverted pendulum, <img src="https://latex.codecogs.com/svg.image?\inline%20u" alt="u" /> is the wheel acceleration (or equivalently the PWM duty cycle sent to the servos).

---

### 3. PID Control Mathematics

---

#### PID Control

**In one sentence**: Combine proportional (present error), integral (past error), and derivative (future trend) actions.

<div align="center"><img src="https://latex.codecogs.com/svg.image?u%28t%29%20%3D%20K_p%20e%28t%29%20%2B%20K_i%20%5Cint%20e%5C%2Cdt%20%2B%20K_d%20%5Cfrac%7Bde%7D%7Bdt%7D" alt="u(t) = K_p e(t) + K_i \int e\,dt + K_d \frac{de}{dt}" /></div>

| Symbol | Name | Role | Everyday analogy |
|--------|------|------|-----------------|
| <img src="https://latex.codecogs.com/svg.image?\inline%20K_p" alt="K_p" /> | Proportional gain | Output proportional to current error | Push as hard as the stick tilts |
| <img src="https://latex.codecogs.com/svg.image?\inline%20K_i" alt="K_i" /> | Integral gain | Correct accumulated past error | Gradually fix a persistent lean |
| <img src="https://latex.codecogs.com/svg.image?\inline%20K_d" alt="K_d" /> | Derivative gain | Brake based on rate of error change | Act early when falling speed is high |

> 📐 Theory reference: [Section 3. PID Control Mathematics](pid_theory.md#3-pid-control-mathematics)

---

#### Proportional Term (P)

Produces an output proportional to the current error. Intuitive but may leave a **steady-state error** and can **oscillate** if gain is too high.

---

#### Integral Term (I)

Accumulates past error over time — slowly but surely eliminates persistent offsets that P alone cannot correct.

**Integral Windup**: When the actuator saturates (hits its maximum output), the integral keeps accumulating but the actuator can't respond. When saturation ends, a large "catch-up" overshoot occurs. Clamping the integral value (e.g., `INTEGRAL_LIMIT = 100.0`) prevents this.

---

#### Derivative Term (D)

Reacts to the *rate of change* of the error — a predictive brake. Using the gyroscope reading directly as <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" /> avoids **derivative kick** (a spike caused by sudden changes in the setpoint).

---

#### Discretization

**In one sentence**: Convert continuous-time differential equations into difference equations that a microcontroller can execute.

A microcontroller runs at discrete time steps <img src="https://latex.codecogs.com/svg.image?\inline%20%5CDelta%20t" alt="\Delta t" /> (here 10 ms). Integrals become sums; derivatives become finite differences:

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cfrac%7Bde%7D%7Bdt%7D%20%5Capprox%20%5Cfrac%7Be%5Bn%5D%20-%20e%5Bn-1%5D%7D%7B%5CDelta%20t%7D" alt="\frac{de}{dt} \approx \frac{e[n] - e[n-1]}{\Delta t}" /></div>

---

#### Gain

**In one sentence**: An amplification factor that scales a signal.

<img src="https://latex.codecogs.com/svg.image?\inline%20K_p%20%3D%2010" alt="K_p = 10" /> means "for every 1 unit of error, apply 10 units of control force." Too high causes oscillation; too low gives sluggish response.

---

#### Steady-State Error

**In one sentence**: The residual offset remaining after the system settles.

P-only control often leaves a small but non-zero error. The I term integrates this error over time and eliminates it.

---

#### Transfer Function

**In one sentence**: A mathematical expression (in the Laplace domain) that fully describes the input-output relationship of a linear system.

<div align="center"><img src="https://latex.codecogs.com/svg.image?C%28s%29%20%3D%20K_p%20%2B%20%5Cfrac%7BK_i%7D%7Bs%7D%20%2B%20K_d%20s" alt="C(s) = K_p + \frac{K_i}{s} + K_d s" /></div>

The pole at <img src="https://latex.codecogs.com/svg.image?\inline%20s%20%3D%200" alt="s = 0" /> (from the <img src="https://latex.codecogs.com/svg.image?\inline%20K_i%2Fs" alt="K_i/s" /> term) ensures the integrator eliminates steady-state error.

---

#### Laplace Transform

**In one sentence**: A mathematical tool that converts differential equations into algebraic equations, making analysis much easier.

Differentiation (<img src="https://latex.codecogs.com/svg.image?\inline%20d%2Fdt" alt="d/dt" />) becomes multiplication by <img src="https://latex.codecogs.com/svg.image?\inline%20s" alt="s" />, turning differential equations into polynomials in <img src="https://latex.codecogs.com/svg.image?\inline%20s" alt="s" />. Analogous to using logarithms to turn multiplication into addition.

---

### 4. State-Space Representation

---

#### State-Space Representation

**In one sentence**: Describe a system by collecting all its "state variables" into a vector and writing the dynamics as matrix equations.

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cdot%7B%5Cmathbf%7Bx%7D%7D%20%3D%20A%5Cmathbf%7Bx%7D%20%2B%20Bu%2C%20%5Cqquad%20%5Cmathbf%7By%7D%20%3D%20C%5Cmathbf%7Bx%7D" alt="\dot{\mathbf{x}} = A\mathbf{x} + Bu, \qquad \mathbf{y} = C\m..." /></div>

- <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathbf%7Bx%7D" alt="\mathbf{x}" />: **State vector** — everything you need to know to predict future behavior
- <img src="https://latex.codecogs.com/svg.image?\inline%20A" alt="A" />: **System matrix** — how states evolve naturally
- <img src="https://latex.codecogs.com/svg.image?\inline%20B" alt="B" />: **Input matrix** — how the control input affects the states
- <img src="https://latex.codecogs.com/svg.image?\inline%20C" alt="C" />: **Output matrix** — which states are measured by sensors

> 📐 Theory reference: [Section 2. State-Space Representation](pid_theory.md#2-state-space-representation)

---

#### State Vector <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathbf%7Bx%7D" alt="\mathbf{x}" />

**In one sentence**: A column vector listing all variables that fully describe the system's current condition.

For the inverted pendulum:

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cmathbf%7Bx%7D%20%3D%20%5Cbegin%7Bbmatrix%7D%5Ctheta%20%5C%5C%20%5Cdot%7B%5Ctheta%7D%20%5C%5C%20x_w%20%5C%5C%20%5Cdot%7Bx%7D_w%5Cend%7Bbmatrix%7D%20%3D%20%5Cbegin%7Bbmatrix%7D%5Ctext%7Btilt%20angle%7D%20%5C%5C%20%5Ctext%7Bangular%20velocity%7D%20%5C%5C%20%5Ctext%7Bwheel%20position%7D%20%5C%5C%20%5Ctext%7Bwheel%20speed%7D%5Cend%7Bbmatrix%7D" alt="\mathbf{x} = \begin{bmatrix}\theta \\ \dot{\theta} \\ x_w \\..." /></div>

---

#### Controllability

**In one sentence**: "Can the control input steer the system from any initial state to any desired state?"

If the controllability matrix <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cmathcal%7BC%7D%20%3D%20%5BB%5C%20%7C%5C%20AB%5C%20%7C%5C%20A%5E2B%5C%20%7C%5C%20A%5E3B%5D" alt="\mathcal{C} = [B\ |\ AB\ |\ A^2B\ |\ A^3B]" /> has full rank (rank 4 here), then — at least in theory — any initial tilt or position can be corrected.

---

#### Observability

**In one sentence**: "Can all state variables be inferred from the sensor measurements alone?"

The IMU directly measures <img src="https://latex.codecogs.com/svg.image?\inline%20%5Ctheta" alt="\theta" /> and <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7B%5Ctheta%7D" alt="\dot{\theta}" />. If the system is observable, <img src="https://latex.codecogs.com/svg.image?\inline%20x_w" alt="x_w" /> and <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cdot%7Bx%7D_w" alt="\dot{x}_w" /> can be reconstructed mathematically from those readings.

---

#### Pole Placement

**In one sentence**: Design a state-feedback controller by choosing where the closed-loop eigenvalues (poles) should lie.

Poles on the left half of the complex plane give stable, decaying responses. By choosing their locations, you specify how fast and how smoothly the system responds.

---

#### Observer / State Estimator

**In one sentence**: A mathematical algorithm that estimates unmeasured state variables from available sensor readings.

The wheel position <img src="https://latex.codecogs.com/svg.image?\inline%20x_w" alt="x_w" /> is not directly measured, but can be estimated using the system model plus IMU data. The Kalman filter is an optimal observer.

---

### 5. Kalman Filter & Sensor Fusion

---

#### Kalman Filter

**In one sentence**: An algorithm that optimally fuses noisy sensor measurements with a system model to estimate the true state.

Used everywhere — GPS navigation, aircraft autopilots, robot arms. It alternates between **prediction** (use the model to guess the next state) and **update** (correct the guess using new sensor data, weighted by each source's reliability).

> 📐 Theory reference: [Section 5. Kalman Filter](pid_theory.md#5-kalman-filter)

---

#### Covariance Matrix <img src="https://latex.codecogs.com/svg.image?\inline%20P" alt="P" />

**In one sentence**: A matrix that quantifies how uncertain the current state estimate is.

Large diagonal values mean low confidence. The Kalman filter tracks this uncertainty automatically and uses it to weight predictions vs. measurements.

---

#### Process Noise <img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" /> / Measurement Noise <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" />

- **<img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" />**: Uncertainty in the system model (e.g., road bumps, unmodeled dynamics)
- **<img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" />**: Uncertainty in sensor readings (e.g., accelerometer noise floor)

High <img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" /> → trust sensor more; high <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" /> → trust model prediction more.

---

#### Kalman Gain <img src="https://latex.codecogs.com/svg.image?\inline%20K_k" alt="K_k" />

**In one sentence**: A per-step weighting factor that determines how much to trust the prediction vs. the measurement.

<img src="https://latex.codecogs.com/svg.image?\inline%20K_k%20%5Cto%200" alt="K_k \to 0" />: ignore sensor, keep prediction.  
<img src="https://latex.codecogs.com/svg.image?\inline%20K_k%20%5Cto%201" alt="K_k \to 1" />: discard prediction, use sensor reading.  
Computed automatically from <img src="https://latex.codecogs.com/svg.image?\inline%20P" alt="P" />, <img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" />, and <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" /> at every step.

---

#### Innovation

**In one sentence**: The difference between the actual sensor measurement and the predicted measurement.

<div align="center"><img src="https://latex.codecogs.com/svg.image?y_k%20%3D%20z_k%20-%20H%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D" alt="y_k = z_k - H\hat{\mathbf{x}}_{k|k-1}" /></div>

Zero innovation means the model predicted perfectly. Large innovation means the model missed something — the state estimate is updated proportionally.

---

#### Gyro Bias <img src="https://latex.codecogs.com/svg.image?\inline%20b" alt="b" />

**In one sentence**: A slow, temperature-dependent drift offset added to the gyroscope reading when the sensor is actually stationary.

Cheap MEMS gyroscopes output a non-zero rate even when still. The Kalman filter in this project treats <img src="https://latex.codecogs.com/svg.image?\inline%20b" alt="b" /> as a third state variable and estimates it on the fly.

---

#### Complementary Filter

**In one sentence**: A simple, lightweight alternative to the Kalman filter that blends gyroscope and accelerometer data with a fixed mixing coefficient <img src="https://latex.codecogs.com/svg.image?\inline%20%5Calpha" alt="\alpha" />.

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Ctheta_%7B%5Ctext%7Best%7D%7D%20%3D%20%5Calpha%28%5Ctheta_%7B%5Ctext%7Bprev%7D%7D%20%2B%20%5Cdot%7B%5Ctheta%7D_%7B%5Ctext%7Bgyro%7D%7D%20%5CDelta%20t%29%20%2B%20%281-%5Calpha%29%5Ctheta_%7B%5Ctext%7Bacc%7D%7D" alt="\theta_{\text{est}} = \alpha(\theta_{\text{prev}} + \dot{\th..." /></div>

| | Complementary | Kalman |
|---|---|---|
| Computation | Minimal | Moderate |
| Optimality | Heuristic | Theoretically optimal |
| Bias correction | None | Automatic |
| Tuning | Single <img src="https://latex.codecogs.com/svg.image?\inline%20%5Calpha" alt="\alpha" /> | Multiple <img src="https://latex.codecogs.com/svg.image?\inline%20Q" alt="Q" />, <img src="https://latex.codecogs.com/svg.image?\inline%20R" alt="R" /> |

---

#### IMU — Inertial Measurement Unit

**In one sentence**: A chip combining an accelerometer and a gyroscope (and often a magnetometer) to measure motion and orientation.

This project uses the **MPU6886** built into the M5StickC Plus: 3-axis accelerometer + 3-axis gyroscope, max sample rate 1 kHz.

| Sensor | Measures | Strength | Weakness |
|--------|---------|----------|---------|
| Accelerometer | Tilt angle from gravity | No long-term drift | Sensitive to vibration |
| Gyroscope | Angular rate directly | Fast, low noise | Drift (bias) builds up |

---

### 6. Stability Analysis

---

#### Stability

**In one sentence**: A system is stable if small disturbances cause only bounded, decaying responses.

Open-loop inverted pendulum: **unstable** (any tiny tilt grows exponentially).  
Closed-loop (with PID): **stable** (returns to upright after a push).

---

#### Characteristic Equation

**In one sentence**: The polynomial whose roots (poles) determine the system's natural behavior.

<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cdet%28sI%20-%20A%29%20%3D%200" alt="\det(sI - A) = 0" /></div>

Roots in the left half-plane → stable. Roots in the right half-plane → unstable.

---

#### Poles

**In one sentence**: The roots of the characteristic equation; they determine stability and transient response.

| Pole location | Response |
|---------------|----------|
| Negative real | Exponential decay (stable) |
| Positive real | Exponential growth (unstable) |
| Pure imaginary | Sustained oscillation |
| Complex, negative real part | Damped oscillation (stable) |

---

#### Hurwitz Stability Criterion

**In one sentence**: Check the coefficients of the characteristic polynomial — if they satisfy the Hurwitz conditions, all roots are in the left half-plane (stable).

For the closed-loop PD-controlled inverted pendulum (<img src="https://latex.codecogs.com/svg.image?\inline%20s%5E2%20%2B%20%5Cbeta%20K_d%20s%20%2B%20%28%5Cbeta%20K_p%20-%20%5Calpha%29%20%3D%200" alt="s^2 + \beta K_d s + (\beta K_p - \alpha) = 0" />):
1. <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cbeta%20K_d%20%3E%200" alt="\beta K_d > 0" /> — positive damping
2. <img src="https://latex.codecogs.com/svg.image?\inline%20%5Cbeta%20K_p%20%3E%20%5Calpha" alt="\beta K_p > \alpha" /> — restoring force exceeds gravity, i.e., <img src="https://latex.codecogs.com/svg.image?\inline%20K_p%20%3E%20g%2Fl" alt="K_p > g/l" />

---

#### Phase Margin

**In one sentence**: How much additional phase lag (delay) the loop can tolerate before going unstable.

Target: **> 30°**. Low phase margin means the system is sensitive to time delays introduced by sensors, actuators, and computation.

---

#### Gain Margin

**In one sentence**: How much the loop gain can be increased before going unstable (expressed in dB).

Target: **> 6 dB** (factor of 2). In practice, RC servo nonlinearities dominate and make these margins approximate.

---

#### Bode Plot

**In one sentence**: A pair of frequency-domain graphs showing how the loop gain and phase shift vary with frequency.

Used to assess gain margin, phase margin, and which frequency ranges are "dangerous" for instability. Horizontal axis is frequency (log scale); vertical axes are magnitude (dB) and phase (degrees).

---

### 7. Hardware & Implementation Terms

---

#### PWM — Pulse Width Modulation

**In one sentence**: Control average voltage (and thus motor speed) by rapidly switching power on and off, varying the on-time fraction (duty cycle).

For RC servos a **1000–2000 μs pulse** within a 20 ms period encodes the desired position or speed:

| Pulse width | Action |
|------------|--------|
| 1500 μs | Stop (neutral) |
| 1000 μs | Full speed one direction |
| 2000 μs | Full speed opposite direction |

---

#### RC Servo / Continuous Rotation Servo (FS90R)

**In one sentence**: A compact gearmotor with built-in speed/position control, commanded by a PWM pulse.

Standard servos rotate to a specific angle (0–180°). **Continuous rotation** servos like the FS90R instead use the pulse width to set **speed and direction** — no built-in position feedback.

---

#### Deadband

**In one sentence**: The range of input values within which an actuator produces no output.

RC servos ignore commands too close to neutral (approximately ±5°). The `kpower` scaling factor ensures the PID output is large enough to exit the deadband under normal conditions.

---

#### Backlash

**In one sentence**: Mechanical play ("slop") between meshing gears that causes a delay when direction reverses.

Backlash introduces a small dead zone when changing direction, which can excite oscillations. Increasing the D-gain helps damp these oscillations.

---

#### I²C (Inter-Integrated Circuit)

**In one sentence**: A two-wire serial communication protocol used between microcontrollers and peripheral chips.

SDA (data) + SCL (clock) allow multiple devices to share the same bus. The ESP32 reads IMU data from the MPU6886 over I²C at ~400 kHz, taking roughly 1 ms per read.

---

#### M5StickC Plus / ESP32

- **ESP32**: Dual-core 240 MHz MCU with Wi-Fi and Bluetooth. Programmed with the Arduino IDE.
- **M5StickC Plus**: A compact board packaging the ESP32, MPU6886 IMU, LCD, buttons, and battery into a fingertip-sized enclosure.

---

#### Control / Sampling Period <img src="https://latex.codecogs.com/svg.image?\inline%20%5CDelta%20t" alt="\Delta t" />

**In one sentence**: The fixed time interval between successive iterations of the sense–compute–actuate loop.

This project runs at <img src="https://latex.codecogs.com/svg.image?\inline%20%5CDelta%20t%20%3D%2010%5C%2C%5Ctext%7Bms%7D" alt="\Delta t = 10\,\text{ms}" /> (100 Hz). Faster sampling improves control fidelity but requires more processing and communication bandwidth. At 240 MHz, the ESP32 handles the 100 Hz loop with plenty of headroom.

---

### 8. Advanced Control Methods

---

#### LQR — Linear Quadratic Regulator

**In one sentence**: An optimal state-feedback controller that automatically computes gains by minimizing a weighted cost of state error and control effort.

<div align="center"><img src="https://latex.codecogs.com/svg.image?J%20%3D%20%5Cint_0%5E%5Cinfty%20%28%5Cmathbf%7Bx%7D%5ET%20Q%5Cmathbf%7Bx%7D%20%2B%20u%5ET%20Ru%29%5C%2Cdt%20%5Cquad%20%5Ctext%7Bminimized%20via%20the%20Riccati%20equation%7D" alt="J = \int_0^\infty (\mathbf{x}^T Q\mathbf{x} + u^T Ru)\,dt \q..." /></div>

Unlike PID (where gains are tuned by trial and error), LQR balances all state variables (angle, angular rate, position, speed) simultaneously.

> 📐 Theory reference: [Section 9.1 LQR](pid_theory.md#91-lqr-linear-quadratic-regulator)

---

#### MPC — Model Predictive Control

**In one sentence**: At each timestep, solve a constrained optimization over a finite prediction horizon to find the best sequence of control actions.

MPC explicitly handles actuator limits, angle bounds, and other constraints — but demands significantly more computation than PID or LQR.

> 📐 Theory reference: [Section 9.2 MPC](pid_theory.md#92-model-predictive-control-mpc)

---

#### Ziegler–Nichols Method

**In one sentence**: A classical empirical PID tuning recipe based on the ultimate gain <img src="https://latex.codecogs.com/svg.image?\inline%20K_u" alt="K_u" /> and period <img src="https://latex.codecogs.com/svg.image?\inline%20T_u" alt="T_u" /> at the onset of sustained oscillation.

<div align="center"><img src="https://latex.codecogs.com/svg.image?K_p%20%3D%200.6K_u%2C%20%5Cquad%20K_i%20%3D%20%5Cfrac%7B1.2K_u%7D%7BT_u%7D%2C%20%5Cquad%20K_d%20%3D%200.075K_u%20T_u" alt="K_p = 0.6K_u, \quad K_i = \frac{1.2K_u}{T_u}, \quad K_d = 0...." /></div>

⚠️ Not directly applicable to open-loop-unstable systems (like inverted pendulums) because inducing safe sustained oscillation is difficult.

---

#### Algebraic Riccati Equation

**In one sentence**: The matrix equation solved to obtain the optimal LQR feedback gain.

<div align="center"><img src="https://latex.codecogs.com/svg.image?A%5ET%20P%20%2B%20PA%20-%20PBR%5E%7B-1%7DB%5ET%20P%20%2B%20Q%20%3D%200" alt="A^T P + PA - PBR^{-1}B^T P + Q = 0" /></div>

Solved numerically using tools like MATLAB's `lqr()` or Python's `scipy.linalg.solve_continuous_are()`.

---

#### Reinforcement Learning

**In one sentence**: An AI technique where a control policy is learned through trial and error, maximizing a reward signal — without needing an explicit mathematical model.

The robot falls, learns, and gradually improves. **Sim-to-Real transfer** (train in simulation, deploy on hardware) is a popular approach because real-world falls are slow and potentially damaging.

---

## 📚 Related Documents

| Document | Content | Level |
|----------|---------|-------|
| [pid_guide.md](pid_guide.md) | Intuitive beginner's guide to PID and the inverted pendulum | ⭐ |
| [pid_theory.md](pid_theory.md) | Full mathematical treatment: equations of motion, state-space, Kalman filter | ⭐⭐⭐ |
| **glossary.md** (this file) | Plain-language explanations of every technical term | ⭐ → ⭐⭐⭐ |

---

## 📚 References / 参考文献

- Åström, K.J. & Murray, R.M. (2021). *Feedback Systems: An Introduction for Scientists and Engineers*, 2nd ed. Princeton University Press.
- Ogata, K. (2010). *Modern Control Engineering*, 5th ed. Prentice Hall.
- Welch, G. & Bishop, G. (2006). *An Introduction to the Kalman Filter*. UNC Chapel Hill TR 95-041.
- Interface誌 2025年9月号「モータ制御プログラミング入門」第3章 — 仁野新一 著
