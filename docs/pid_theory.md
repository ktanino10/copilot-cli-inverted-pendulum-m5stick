# 📐 PID制御 理論編 — 倒立振子の数理モデルと制御設計
# PID Control Theory — Mathematical Modeling and Control Design for the Inverted Pendulum

> 上級者・制御工学を学ぶ方向けの技術文書です。数式・状態空間表現・カルマンフィルタの導出を含みます。
>
> Advanced reference for engineers and control theory students. Includes mathematical derivations, state-space representation, and Kalman filter formulation.

**[日本語](#日本語) | [English](#english)**

> 📖 **専門用語がわからない方へ / Unfamiliar with the terminology?**
> **[docs/glossary.md](glossary.md)** — すべての専門用語を平易な言葉で解説した補足資料集 / Plain-language glossary for every technical term used in this document

---

<a id="日本語"></a>
## 日本語

---

### 1. 倒立振子の運動方程式

#### 1.1 モデルの定義

台車型倒立振子（cart-inverted pendulum）を一自由度に簡略化して考える。

| 記号 | 意味 | 単位 |
|------|------|------|
| θ | 振子の鉛直からの傾斜角（直立 = 0） | rad |
| θ̇ | 角速度 | rad/s |
| θ̈ | 角加速度 | rad/s² |
| x | 台車（車輪）の水平位置 | m |
| m | 振子の質量 | kg |
| l | 重心までの長さ | m |
| g | 重力加速度（≈ 9.81） | m/s² |
| F | 車輪が台車に与える水平力 | N |
| I | 振子の慣性モーメント | kg·m² |

#### 1.2 ラグランジュ方程式からの導出

系のラグランジアン ℒ = T − V について：

- 運動エネルギー T：


<div align="center"><img src="https://latex.codecogs.com/svg.image?T%20%3D%20%5Cfrac%7B1%7D%7B2%7DI%5Cdot%7B%5Ctheta%7D%5E2%20%2B%20%5Cfrac%7B1%7D%7B2%7Dm%5Cdot%7Bx%7D%5E2%20%2B%20ml%5Cdot%7Bx%7D%5Cdot%7B%5Ctheta%7D%5Ccos%5Ctheta" alt="T = \frac{1}{2}I\dot{\theta}^2 + \frac{1}{2}m\dot{x}^2 + ml\..." /></div>


- 位置エネルギー V：


<div align="center"><img src="https://latex.codecogs.com/svg.image?V%20%3D%20mgl%5Ccos%5Ctheta" alt="V = mgl\cos\theta..." /></div>


オイラー＝ラグランジュ方程式を適用すると、非線形運動方程式が得られる：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%28I%20%2B%20ml%5E2%29%5Cddot%7B%5Ctheta%7D%20-%20mgl%5Csin%5Ctheta%20%3D%20-ml%5Cddot%7Bx%7D%5Ccos%5Ctheta" alt="(I + ml^2)\ddot{\theta} - mgl\sin\theta = -ml\ddot{x}\cos\th..." /></div>


#### 1.3 小角度近似による線形化

倒立状態（θ ≈ 0）付近で sinθ ≈ θ, cosθ ≈ 1 と近似すると：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%28I%20%2B%20ml%5E2%29%5Cddot%7B%5Ctheta%7D%20-%20mgl%5Ctheta%20%3D%20-ml%5Cddot%7Bx%7D" alt="(I + ml^2)\ddot{\theta} - mgl\theta = -ml\ddot{x}..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cddot%7B%5Ctheta%7D%20%3D%20%5Cfrac%7Bmgl%7D%7BI%20%2B%20ml%5E2%7D%5Ctheta%20-%20%5Cfrac%7Bml%7D%7BI%20%2B%20ml%5E2%7D%5Cddot%7Bx%7D" alt="\ddot{\theta} = \frac{mgl}{I + ml^2}\theta - \frac{ml}{I + m..." /></div>


ここで α = mgl/(I + ml²), β = ml/(I + ml²) とおくと：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cddot%7B%5Ctheta%7D%20%3D%20%5Calpha%5Ctheta%20-%20%5Cbeta%20u" alt="\ddot{\theta} = \alpha\theta - \beta u..." /></div>


u = ẍ は制御入力（車輪による加速度）に対応する。

---

### 2. 状態空間表現

#### 2.1 状態ベクトルの定義

状態変数を以下のように定義する：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cmathbf%7Bx%7D%20%3D%20%5Cbegin%7Bbmatrix%7D%20%5Ctheta%20%5C%5C%20%5Cdot%7B%5Ctheta%7D%20%5C%5C%20x_w%20%5C%5C%20%5Cdot%7Bx%7D_w%20%5Cend%7Bbmatrix%7D" alt="\mathbf{x} = \begin{bmatrix} \theta \\ \dot{\theta} \\ x_w \..." /></div>


ここで x_w は車輪の位置、ẋ_w は車輪の速度。

#### 2.2 線形状態方程式


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cdot%7B%5Cmathbf%7Bx%7D%7D%20%3D%20A%5Cmathbf%7Bx%7D%20%2B%20Bu" alt="\dot{\mathbf{x}} = A\mathbf{x} + Bu..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?A%20%3D%20%5Cbegin%7Bbmatrix%7D%0A0%20%26%201%20%26%200%20%26%200%20%5C%5C%0A%5Calpha%20%26%200%20%26%200%20%26%200%20%5C%5C%0A0%20%26%200%20%26%200%20%26%201%20%5C%5C%0A0%20%26%200%20%26%200%20%26%200%0A%5Cend%7Bbmatrix%7D%2C%20%5Cquad%0AB%20%3D%20%5Cbegin%7Bbmatrix%7D%0A0%20%5C%5C%0A-%5Cbeta%20%5C%5C%0A0%20%5C%5C%0A1%0A%5Cend%7Bbmatrix%7D" alt="A = \begin{bmatrix}
0 & 1 & 0 & 0 \\
\alpha & 0 & 0 & 0 \\
0..." /></div>


観測方程式（IMU で θ と θ̇ を測定可能）：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cmathbf%7By%7D%20%3D%20C%5Cmathbf%7Bx%7D%2C%20%5Cquad%0AC%20%3D%20%5Cbegin%7Bbmatrix%7D%0A1%20%26%200%20%26%200%20%26%200%20%5C%5C%0A0%20%26%201%20%26%200%20%26%200%0A%5Cend%7Bbmatrix%7D" alt="\mathbf{y} = C\mathbf{x}, \quad
C = \begin{bmatrix}
1 & 0 & ..." /></div>


#### 2.3 可制御性・可観測性

可制御性行列 𝒞 = [B | AB | A²B | A³B] がフルランク（rank 4）であることを確認すれば、**状態フィードバックにより任意の極配置が可能**。

可観測性行列 𝒪 = [Cᵀ | (CA)ᵀ | (CA²)ᵀ | (CA³)ᵀ]ᵀ もフルランクであれば、**オブザーバ（カルマンフィルタ等）で全状態を推定可能**。

---

### 3. PID制御の数理

#### 3.1 連続時間PID制御則

角度偏差 e(t) = θ_ref − θ(t) に対して（θ_ref = 0）：


<div align="center"><img src="https://latex.codecogs.com/svg.image?u%28t%29%20%3D%20K_p%20e%28t%29%20%2B%20K_i%20%5Cint_0%5Et%20e%28%5Ctau%29%5C%2Cd%5Ctau%20%2B%20K_d%20%5Cfrac%7Bde%28t%29%7D%7Bdt%7D" alt="u(t) = K_p e(t) + K_i \int_0^t e(\tau)\,d\tau + K_d \frac{de..." /></div>


#### 3.2 伝達関数表現

ラプラス変換を適用すると：


<div align="center"><img src="https://latex.codecogs.com/svg.image?C%28s%29%20%3D%20K_p%20%2B%20%5Cfrac%7BK_i%7D%7Bs%7D%20%2B%20K_d%20s%20%3D%20%5Cfrac%7BK_d%20s%5E2%20%2B%20K_p%20s%20%2B%20K_i%7D%7Bs%7D" alt="C(s) = K_p + \frac{K_i}{s} + K_d s = \frac{K_d s^2 + K_p s +..." /></div>


PID制御器は s = 0 に極を持つ（定常偏差を除去する積分動作）。

#### 3.3 離散化（差分方程式）

制御周期 Δt で離散化する。本プロジェクトでは Δt = 10 ms（100 Hz）。

| PID項 | 連続時間 | 離散時間 |
|-------|---------|---------|
| P | Kp × e(t) | Kp × e[n] |
| I | Ki × ∫e dt | Ki × Σe[k]Δt |
| D | Kd × de/dt | Kd × (e[n] − e[n−1]) / Δt |

#### 3.4 実装上の注意点

**積分器ワインドアップ (Integral Windup)**

積分項が飽和出力下で蓄積し続けると、復帰時にオーバーシュートが発生する。対策としてクランプ方式を採用：


<div align="center"><img src="https://latex.codecogs.com/svg.image?I%5Bn%5D%20%3D%20%5Coperatorname%7Bclamp%7D%5Cleft%28%5Csum%20e%5Bk%5D%5CDelta%20t%2C%5C%20-I_%7B%5Cmax%7D%2C%5C%20I_%7B%5Cmax%7D%5Cright%29" alt="I[n] = \operatorname{clamp}\left(\sum e[k]\Delta t,\ -I_{\ma..." /></div>


本ファームウェアでは `INTEGRAL_LIMIT = 100.0`。

**微分キック (Derivative Kick) の回避**

目標値の急変時に微分項が発散する問題。対策として **プロセス変数の微分**を使う：


<div align="center"><img src="https://latex.codecogs.com/svg.image?D%5Bn%5D%20%3D%20K_d%20%5Ccdot%20%5Cleft%28-%5Cdot%7B%5Ctheta%7D%5Bn%5D%5Cright%29" alt="D[n] = K_d \cdot \left(-\dot{\theta}[n]\right)..." /></div>


↑ ジャイロの値を直接使用

これは目標値 θ_ref ではなく、測定値 θ の変化率をフィードバックすることで微分キックを防ぐ。本ファームウェアの `kd * (-gyroRate)` がこれに該当する。

---

### 4. 拡張PID — 位置・速度補償

#### 4.1 ドリフト問題

角度PIDのみでは、系の不安定平衡点付近でバランスは取れるが、台車が一方向に走り続ける（定常偏差が位置に蓄積）。

#### 4.2 位置・速度フィードバックの追加

制御出力を拡張する（姿勢制御 ＋ 位置・速度補償）：


<div align="center"><img src="https://latex.codecogs.com/svg.image?u%5Bn%5D%20%3D%20%28K_p%20e_%5Ctheta%20%2B%20K_i%20%5Csum%20e_%5Ctheta%20%5CDelta%20t%20%2B%20K_d%28-%5Cdot%7B%5Ctheta%7D%29%29%20%2B%20%28K_%7Bds%7D%20%5Ccdot%20x_w%20%2B%20K_%7Bspd%7D%20%5Ccdot%20%5Cdot%7Bx%7D_w%29" alt="u[n] = (K_p e_\theta + K_i \sum e_\theta \Delta t + K_d(-\do..." /></div>


これは **状態フィードバック制御** u = −K**x** の一形態と見なせる：


<div align="center"><img src="https://latex.codecogs.com/svg.image?K%20%3D%20%5B-K_p%2C%5C%20-K_d%2C%5C%20-K_%7Bds%7D%2C%5C%20-K_%7Bspd%7D%5D" alt="K = [-K_p,\ -K_d,\ -K_{ds},\ -K_{spd}]..." /></div>


（Ki は内部モデル原理による積分拡大系の一部）

#### 4.3 本ファームウェアの実装パラメータ

```cpp
kpower = 0.001;   // 全体スケーリング
kp     = 21.0;    // 比例ゲイン
ki     = 7.0;     // 積分ゲイン
kd     = 1.6;     // 微分ゲイン
kdst   = 0.07;    // 位置フィードバックゲイン
kspd   = 2.5;     // 速度フィードバックゲイン
```

K_power はPID出力をサーボPWM値にマッピングするスケーリング係数。

---

### 5. カルマンフィルタ

#### 5.1 なぜカルマンフィルタが必要か

| センサ | 長所 | 短所 |
|-------|------|------|
| 加速度センサ | 長期的に安定（ドリフトなし） | 振動・衝撃に弱い（高周波ノイズ） |
| ジャイロスコープ | 高速応答、ノイズに強い | 長期ドリフト（バイアス累積） |

**加速度センサの低周波精度** と **ジャイロの高周波精度** を最適に融合するのがカルマンフィルタの役割。

#### 5.2 システムモデル

状態ベクトル：**x**_k = [θ_k, θ̇_k, b_k]ᵀ

ここで b_k はジャイロバイアス（ドリフト項）。

**予測ステップ (Prediction)**：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D%20%3D%20F%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk-1%7Ck-1%7D%20%2B%20Bu_k" alt="\hat{\mathbf{x}}_{k|k-1} = F\hat{\mathbf{x}}_{k-1|k-1} + Bu_..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?F%20%3D%20%5Cbegin%7Bbmatrix%7D%0A1%20%26%20%5CDelta%20t%20%26%20-%5CDelta%20t%20%5C%5C%0A0%20%26%201%20%26%200%20%5C%5C%0A0%20%26%200%20%26%201%0A%5Cend%7Bbmatrix%7D%2C%20%5Cquad%0AB%20%3D%20%5Cbegin%7Bbmatrix%7D%0A0%20%5C%5C%201%20%5C%5C%200%0A%5Cend%7Bbmatrix%7D" alt="F = \begin{bmatrix}
1 & \Delta t & -\Delta t \\
0 & 1 & 0 \\..." /></div>


u_k = ω_gyro · Δt （ジャイロ入力）

共分散の予測：


<div align="center"><img src="https://latex.codecogs.com/svg.image?P_%7Bk%7Ck-1%7D%20%3D%20FP_%7Bk-1%7Ck-1%7DF%5ET%20%2B%20Q" alt="P_{k|k-1} = FP_{k-1|k-1}F^T + Q..." /></div>


Q はプロセスノイズ共分散行列：


<div align="center"><img src="https://latex.codecogs.com/svg.image?Q%20%3D%20%5Cbegin%7Bbmatrix%7D%0AQ_%5Ctheta%20%26%200%20%26%200%20%5C%5C%0A0%20%26%20Q_%7B%5Cdot%7B%5Ctheta%7D%7D%20%26%200%20%5C%5C%0A0%20%26%200%20%26%20Q_b%0A%5Cend%7Bbmatrix%7D" alt="Q = \begin{bmatrix}
Q_\theta & 0 & 0 \\
0 & Q_{\dot{\theta}}..." /></div>


**更新ステップ (Update)**：

観測値 z_k = θ_acc（加速度センサから計算した角度）に対して：


<div align="center"><img src="https://latex.codecogs.com/svg.image?H%20%3D%20%5Cbegin%7Bbmatrix%7D%201%20%26%200%20%26%200%20%5Cend%7Bbmatrix%7D" alt="H = \begin{bmatrix} 1 & 0 & 0 \end{bmatrix}..." /></div>


イノベーション：


<div align="center"><img src="https://latex.codecogs.com/svg.image?y_k%20%3D%20z_k%20-%20H%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D" alt="y_k = z_k - H\hat{\mathbf{x}}_{k|k-1}..." /></div>


カルマンゲイン：


<div align="center"><img src="https://latex.codecogs.com/svg.image?S_k%20%3D%20HP_%7Bk%7Ck-1%7DH%5ET%20%2B%20R" alt="S_k = HP_{k|k-1}H^T + R..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?K_k%20%3D%20P_%7Bk%7Ck-1%7DH%5ET%20S_k%5E%7B-1%7D" alt="K_k = P_{k|k-1}H^T S_k^{-1}..." /></div>


R は観測ノイズ分散（加速度センサのノイズ特性）。

状態更新：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck%7D%20%3D%20%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D%20%2B%20K_k%20y_k" alt="\hat{\mathbf{x}}_{k|k} = \hat{\mathbf{x}}_{k|k-1} + K_k y_k..." /></div>


共分散更新：


<div align="center"><img src="https://latex.codecogs.com/svg.image?P_%7Bk%7Ck%7D%20%3D%20%28I%20-%20K_k%20H%29P_%7Bk%7Ck-1%7D" alt="P_{k|k} = (I - K_k H)P_{k|k-1}..." /></div>


#### 5.3 相補フィルタとの比較

簡易な代替手法として**相補フィルタ**がある：


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Ctheta_%7Best%7D%20%3D%20%5Calpha%28%5Ctheta_%7Best%2Cprev%7D%20%2B%20%5Cdot%7B%5Ctheta%7D_%7Bgyro%7D%20%5Ccdot%20%5CDelta%20t%29%20%2B%20%281%20-%20%5Calpha%29%5Ctheta_%7Bacc%7D" alt="\theta_{est} = \alpha(\theta_{est,prev} + \dot{\theta}_{gyro..." /></div>


α ∈ [0, 1]（典型的には 0.96〜0.98）

| | 相補フィルタ | カルマンフィルタ |
|---|---|---|
| 計算量 | ◎ 極めて軽い | ○ やや重い（行列演算） |
| チューニング | α 1つだけ | Q, R 複数パラメータ |
| 最適性 | × ヒューリスティック | ◎ 線形系で最適推定 |
| バイアス補正 | × なし | ◎ 自動推定 |

本プロジェクトでは **TKJElectronics の Kalman ライブラリ**を使用。ESP32 の計算能力であれば、カルマンフィルタの演算負荷は問題にならない。

---

### 6. 安定性解析

#### 6.1 開ループ不安定性

線形化モデルの特性方程式 det(sI − A) = 0 を解くと：


<div align="center"><img src="https://latex.codecogs.com/svg.image?s%5E2%20-%20%5Calpha%20%3D%200%20%5Cimplies%20s%20%3D%20%5Cpm%5Csqrt%7B%5Calpha%7D" alt="s^2 - \alpha = 0 \implies s = \pm\sqrt{\alpha}..." /></div>


α > 0（重力の影響）であるため、**開ループ系は不安定**（正の実数極を持つ）。

#### 6.2 閉ループ安定条件

PD制御 u = −Kp·θ − Kd·θ̇ を適用した閉ループ系の特性方程式：


<div align="center"><img src="https://latex.codecogs.com/svg.image?s%5E2%20%2B%20%5Cbeta%20K_d%20s%20%2B%20%28%5Cbeta%20K_p%20-%20%5Calpha%29%20%3D%200" alt="s^2 + \beta K_d s + (\beta K_p - \alpha) = 0..." /></div>


安定条件（フルビッツの判定法）：

1. β·Kd > 0 （減衰項が正）
2. β·Kp − α > 0 すなわち Kp > α/β = g/l

つまり、**比例ゲインは g/l を超えなければ安定化できない**。

#### 6.3 位相余裕とゲイン余裕

ボード線図解析により、ロバスト安定性を評価できる。実用的には：

- **位相余裕 > 30°** を確保する（急激な位相遅れを避ける）
- **ゲイン余裕 > 6 dB** を目安とする

ただし、本プロジェクトのようなRCサーボ駆動系では、サーボ自体の応答遅れ（デッドバンド、PWM更新レート）が支配的なため、古典制御理論の余裕指標は参考値に留まる。

---

### 7. 実装固有の制約と対策

#### 7.1 RCサーボの非線形特性

| 特性 | 影響 | 対策 |
|------|------|------|
| デッドバンド（±5°付近） | 微小制御入力が無視される | `kpower` でスケーリング調整 |
| 速度飽和 | 大傾斜時に復帰不可 | `ANGLE_LIMIT` で早期停止 |
| 非対称応答 | 左右で異なる挙動 | `MOTOR_SIGN` で符号反転 |
| バックラッシュ | ガタつきによる振動 | D項の増加で抑制 |

#### 7.2 IMU特性（MPU6886）

| パラメータ | 典型値 | 備考 |
|-----------|-------|------|
| ジャイロノイズ密度 | 0.01 °/s/√Hz | カルマンフィルタの Q 設定に影響 |
| 加速度ノイズ | 100 μg/√Hz | R 設定に影響 |
| ジャイロドリフト | ±1 °/s（温度依存） | 起動時キャリブレーションで補正 |
| サンプルレート | 最大 1 kHz | 100 Hz 制御には十分 |

#### 7.3 制御周期と計算予算

100 Hz 制御ループの場合、1サイクルあたりの計算予算は **10 ms**。

| 処理 | 所要時間（ESP32） |
|------|------------------|
| IMU読取 (I²C) | ~1 ms |
| カルマンフィルタ | ~0.1 ms |
| PID演算 | < 0.01 ms |
| サーボPWM出力 | < 0.01 ms |
| LCD描画 (10 Hz) | ~5 ms |

ESP32 @ 240 MHz であれば、100 Hz ループは余裕を持って実行可能。LCD描画は10 Hzに間引くことで制御ループへの影響を最小化。

---

### 8. パラメータ設計の指針

#### 8.1 ジーグラー・ニコルズ法（Z-N法）

古典的なチューニング手法。限界ゲイン K_u（持続振動を起こす最小のPゲイン）と限界周期 T_u を実験的に求める。


<div align="center"><img src="https://latex.codecogs.com/svg.image?K_p%20%3D%200.6%20K_u%2C%20%5Cquad%20K_i%20%3D%20%5Cfrac%7B1.2%20K_u%7D%7BT_u%7D%2C%20%5Cquad%20K_d%20%3D%200.075%20K_u%20T_u" alt="K_p = 0.6 K_u, \quad K_i = \frac{1.2 K_u}{T_u}, \quad K_d = ..." /></div>


ただし倒立振子は**開ループ不安定系**であり、持続振動を安全に観察することが困難なため、Z-N法はそのまま適用しにくい。

#### 8.2 実用的アプローチ（手動チューニング）

本プロジェクトで推奨する手順：

1. **Kd から始める**：小さな Kp で手で支えながら、Kd を上げてダンピングを確保
2. **Kp を上げる**：手を離して倒れかけたときに復帰力が出るまで
3. **Ki を微調整**：定常偏差（ゆっくりした傾き）を除去
4. **Kds, Kspd を追加**：位置ドリフトを抑制

#### 8.3 パラメータの物理的解釈

| パラメータ | 物理的意味 | 大きすぎると | 小さすぎると |
|-----------|-----------|------------|------------|
| Kp | 復元力の強さ（バネ定数に相当） | 高周波振動 | 倒れる |
| Ki | 定常偏差の除去速度 | 低周波発振・不安定 | ゆっくり傾いていく |
| Kd | 減衰力（ダンパに相当） | 鈍重な応答 | 振動が止まらない |
| Kds | 位置復帰力 | 位置の振動 | 走り続ける |
| Kspd | 速度ブレーキ力 | 位置応答が鈍い | 加速が止まらない |

---

### 9. 発展的話題

#### 9.1 LQR（線形二次レギュレータ）

PIDの代わりに最適制御を用いる方法。コスト関数：


<div align="center"><img src="https://latex.codecogs.com/svg.image?J%20%3D%20%5Cint_0%5E%5Cinfty%20%28%5Cmathbf%7Bx%7D%5ET%20Q%20%5Cmathbf%7Bx%7D%20%2B%20u%5ET%20R%20u%29%5C%2Cdt" alt="J = \int_0^\infty (\mathbf{x}^T Q \mathbf{x} + u^T R u)\,dt..." /></div>


を最小化するフィードバックゲイン K をリッカチ方程式から求める。


<div align="center"><img src="https://latex.codecogs.com/svg.image?A%5ET%20P%20%2B%20PA%20-%20PBR%5E%7B-1%7DB%5ET%20P%20%2B%20Q%20%3D%200" alt="A^T P + PA - PBR^{-1}B^T P + Q = 0..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?K%20%3D%20R%5E%7B-1%7DB%5ET%20P" alt="K = R^{-1}B^T P..." /></div>


PIDと異なり、**全状態を同時に最適化**できるため、角度・位置・速度のバランスが自動的に決まる。

#### 9.2 モデル予測制御（MPC）

有限時間先の挙動を予測し、制約付き最適化を毎ステップ解く方法。制約（角度制限、モータ飽和）を明示的に扱えるが、ESP32での実時間実装には工夫が必要。

#### 9.3 強化学習

物理モデルを陽に使わず、試行錯誤で最適方策を学習する方法。シミュレータで学習した方策をマイコンに転移する（Sim-to-Real transfer）アプローチが研究されている。

---

<a id="english"></a>
## English

---

### 1. Equations of Motion

#### 1.1 Model Definition

Consider a simplified single-degree-of-freedom cart-inverted pendulum.

| Symbol | Meaning | Unit |
|--------|---------|------|
| θ | Pendulum tilt from vertical (upright = 0) | rad |
| θ̇ | Angular velocity | rad/s |
| θ̈ | Angular acceleration | rad/s² |
| x | Cart (wheel) horizontal position | m |
| m | Pendulum mass | kg |
| l | Length to center of mass | m |
| g | Gravitational acceleration (≈ 9.81) | m/s² |
| F | Horizontal force from wheels | N |
| I | Moment of inertia | kg·m² |

#### 1.2 Derivation from Lagrangian Mechanics

From the Lagrangian ℒ = T − V :

- Kinetic energy T :


<div align="center"><img src="https://latex.codecogs.com/svg.image?T%20%3D%20%5Cfrac%7B1%7D%7B2%7DI%5Cdot%7B%5Ctheta%7D%5E2%20%2B%20%5Cfrac%7B1%7D%7B2%7Dm%5Cdot%7Bx%7D%5E2%20%2B%20ml%5Cdot%7Bx%7D%5Cdot%7B%5Ctheta%7D%5Ccos%5Ctheta" alt="T = \frac{1}{2}I\dot{\theta}^2 + \frac{1}{2}m\dot{x}^2 + ml\..." /></div>


- Potential energy V :


<div align="center"><img src="https://latex.codecogs.com/svg.image?V%20%3D%20mgl%5Ccos%5Ctheta" alt="V = mgl\cos\theta..." /></div>


Applying the Euler–Lagrange equation yields the nonlinear equation of motion:


<div align="center"><img src="https://latex.codecogs.com/svg.image?%28I%20%2B%20ml%5E2%29%5Cddot%7B%5Ctheta%7D%20-%20mgl%5Csin%5Ctheta%20%3D%20-ml%5Cddot%7Bx%7D%5Ccos%5Ctheta" alt="(I + ml^2)\ddot{\theta} - mgl\sin\theta = -ml\ddot{x}\cos\th..." /></div>


#### 1.3 Linearization (Small-Angle Approximation)

Near the upright equilibrium (θ ≈ 0), with sinθ ≈ θ and cosθ ≈ 1:


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cddot%7B%5Ctheta%7D%20%3D%20%5Calpha%5Ctheta%20-%20%5Cbeta%20u" alt="\ddot{\theta} = \alpha\theta - \beta u..." /></div>


where α = mgl/(I + ml²), β = ml/(I + ml²), and u = ẍ is the control input.

---

### 2. State-Space Representation

#### 2.1 State Vector


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cmathbf%7Bx%7D%20%3D%20%5Cbegin%7Bbmatrix%7D%20%5Ctheta%20%5C%5C%20%5Cdot%7B%5Ctheta%7D%20%5C%5C%20x_w%20%5C%5C%20%5Cdot%7Bx%7D_w%20%5Cend%7Bbmatrix%7D" alt="\mathbf{x} = \begin{bmatrix} \theta \\ \dot{\theta} \\ x_w \..." /></div>


#### 2.2 Linear State Equation


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cdot%7B%5Cmathbf%7Bx%7D%7D%20%3D%20A%5Cmathbf%7Bx%7D%20%2B%20Bu" alt="\dot{\mathbf{x}} = A\mathbf{x} + Bu..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?A%20%3D%20%5Cbegin%7Bbmatrix%7D%0A0%20%26%201%20%26%200%20%26%200%20%5C%5C%0A%5Calpha%20%26%200%20%26%200%20%26%200%20%5C%5C%0A0%20%26%200%20%26%200%20%26%201%20%5C%5C%0A0%20%26%200%20%26%200%20%26%200%0A%5Cend%7Bbmatrix%7D%2C%20%5Cquad%0AB%20%3D%20%5Cbegin%7Bbmatrix%7D%0A0%20%5C%5C%0A-%5Cbeta%20%5C%5C%0A0%20%5C%5C%0A1%0A%5Cend%7Bbmatrix%7D" alt="A = \begin{bmatrix}
0 & 1 & 0 & 0 \\
\alpha & 0 & 0 & 0 \\
0..." /></div>


Observation equation (IMU measures θ and θ̇):


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Cmathbf%7By%7D%20%3D%20C%5Cmathbf%7Bx%7D%2C%20%5Cquad%0AC%20%3D%20%5Cbegin%7Bbmatrix%7D%0A1%20%26%200%20%26%200%20%26%200%20%5C%5C%0A0%20%26%201%20%26%200%20%26%200%0A%5Cend%7Bbmatrix%7D" alt="\mathbf{y} = C\mathbf{x}, \quad
C = \begin{bmatrix}
1 & 0 & ..." /></div>


#### 2.3 Controllability & Observability

The controllability matrix 𝒞 = [B | AB | A²B | A³B] must have full rank (= 4), enabling arbitrary pole placement via state feedback.

The observability matrix 𝒪 must also have full rank (= 4), enabling full state estimation via an observer (e.g., Kalman filter).

---

### 3. PID Control Mathematics

#### 3.1 Continuous-Time PID Law

For angle error e(t) = θ_ref − θ(t) where θ_ref = 0:


<div align="center"><img src="https://latex.codecogs.com/svg.image?u%28t%29%20%3D%20K_p%20e%28t%29%20%2B%20K_i%20%5Cint_0%5Et%20e%28%5Ctau%29%5C%2Cd%5Ctau%20%2B%20K_d%20%5Cfrac%7Bde%28t%29%7D%7Bdt%7D" alt="u(t) = K_p e(t) + K_i \int_0^t e(\tau)\,d\tau + K_d \frac{de..." /></div>


#### 3.2 Transfer Function

In the Laplace domain:


<div align="center"><img src="https://latex.codecogs.com/svg.image?C%28s%29%20%3D%20K_p%20%2B%20%5Cfrac%7BK_i%7D%7Bs%7D%20%2B%20K_d%20s%20%3D%20%5Cfrac%7BK_d%20s%5E2%20%2B%20K_p%20s%20%2B%20K_i%7D%7Bs%7D" alt="C(s) = K_p + \frac{K_i}{s} + K_d s = \frac{K_d s^2 + K_p s +..." /></div>


The integrator pole at s = 0 eliminates steady-state error.

#### 3.3 Discretization

At control period Δt = 10 ms (100 Hz):

| Term | Continuous | Discrete |
|------|-----------|----------|
| P | Kp × e(t) | Kp × e[n] |
| I | Ki × ∫e dt | Ki × Σe[k]Δt |
| D | Kd × de/dt | Kd × (e[n] − e[n−1]) / Δt |

#### 3.4 Implementation Considerations

**Integral Windup Prevention**


<div align="center"><img src="https://latex.codecogs.com/svg.image?I%5Bn%5D%20%3D%20%5Coperatorname%7Bclamp%7D%5Cleft%28%5Csum%20e%5Bk%5D%5CDelta%20t%2C%5C%20-I_%7B%5Cmax%7D%2C%5C%20I_%7B%5Cmax%7D%5Cright%29" alt="I[n] = \operatorname{clamp}\left(\sum e[k]\Delta t,\ -I_{\ma..." /></div>


This firmware uses `INTEGRAL_LIMIT = 100.0`.

**Derivative Kick Avoidance**

Instead of differentiating the error (which spikes on setpoint changes), differentiate the process variable:


<div align="center"><img src="https://latex.codecogs.com/svg.image?D%5Bn%5D%20%3D%20K_d%20%5Ccdot%20%28-%5Cdot%7B%5Ctheta%7D%5Bn%5D%29" alt="D[n] = K_d \cdot (-\dot{\theta}[n])..." /></div>


This corresponds to `kd * (-gyroRate)` in the firmware — using the gyroscope reading directly as the derivative term.

---

### 4. Extended PID — Position & Speed Compensation

#### 4.1 The Drift Problem

Angle-only PID stabilizes the pendulum but allows unbounded translation (the cart rolls indefinitely).

#### 4.2 Full Control Law

The full control law (attitude control + position/speed regulation):


<div align="center"><img src="https://latex.codecogs.com/svg.image?u%5Bn%5D%20%3D%20%28K_p%20e_%5Ctheta%20%2B%20K_i%20%5Csum%20e_%5Ctheta%20%5CDelta%20t%20%2B%20K_d%28-%5Cdot%7B%5Ctheta%7D%29%29%20%2B%20%28K_%7Bds%7D%20%5Ccdot%20x_w%20%2B%20K_%7Bspd%7D%20%5Ccdot%20%5Cdot%7Bx%7D_w%29" alt="u[n] = (K_p e_\theta + K_i \sum e_\theta \Delta t + K_d(-\do..." /></div>


This is equivalent to **state feedback** u = −K**x** where:


<div align="center"><img src="https://latex.codecogs.com/svg.image?K%20%3D%20%5B-K_p%2C%5C%20-K_d%2C%5C%20-K_%7Bds%7D%2C%5C%20-K_%7Bspd%7D%5D" alt="K = [-K_p,\ -K_d,\ -K_{ds},\ -K_{spd}]..." /></div>


(The integral term Ki extends this to an **augmented system** via the internal model principle.)

---

### 5. Kalman Filter

#### 5.1 Motivation

| Sensor | Strength | Weakness |
|--------|----------|----------|
| Accelerometer | Long-term stability (no drift) | Sensitive to vibration (high-freq noise) |
| Gyroscope | Fast response, low noise | Long-term drift (bias accumulation) |

The Kalman filter **optimally fuses** the accelerometer's low-frequency accuracy with the gyroscope's high-frequency accuracy.

#### 5.2 System Model

State: **x**_k = [θ_k, θ̇_k, b_k]ᵀ where b_k is the gyro bias.

**Prediction Step**:


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D%20%3D%20F%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk-1%7Ck-1%7D%20%2B%20Bu_k" alt="\hat{\mathbf{x}}_{k|k-1} = F\hat{\mathbf{x}}_{k-1|k-1} + Bu_..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?F%20%3D%20%5Cbegin%7Bbmatrix%7D%0A1%20%26%20%5CDelta%20t%20%26%20-%5CDelta%20t%20%5C%5C%0A0%20%26%201%20%26%200%20%5C%5C%0A0%20%26%200%20%26%201%0A%5Cend%7Bbmatrix%7D%2C%20%5Cquad%0AP_%7Bk%7Ck-1%7D%20%3D%20FP_%7Bk-1%7Ck-1%7DF%5ET%20%2B%20Q" alt="F = \begin{bmatrix}
1 & \Delta t & -\Delta t \\
0 & 1 & 0 \\..." /></div>


**Update Step**:

Measurement z_k = θ_acc, observation matrix H = [1  0  0]:


<div align="center"><img src="https://latex.codecogs.com/svg.image?K_k%20%3D%20P_%7Bk%7Ck-1%7DH%5ET%28HP_%7Bk%7Ck-1%7DH%5ET%20%2B%20R%29%5E%7B-1%7D" alt="K_k = P_{k|k-1}H^T(HP_{k|k-1}H^T + R)^{-1}..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck%7D%20%3D%20%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D%20%2B%20K_k%28z_k%20-%20H%5Chat%7B%5Cmathbf%7Bx%7D%7D_%7Bk%7Ck-1%7D%29" alt="\hat{\mathbf{x}}_{k|k} = \hat{\mathbf{x}}_{k|k-1} + K_k(z_k ..." /></div>



<div align="center"><img src="https://latex.codecogs.com/svg.image?P_%7Bk%7Ck%7D%20%3D%20%28I%20-%20K_kH%29P_%7Bk%7Ck-1%7D" alt="P_{k|k} = (I - K_kH)P_{k|k-1}..." /></div>


#### 5.3 Complementary Filter Comparison

A simpler alternative:


<div align="center"><img src="https://latex.codecogs.com/svg.image?%5Ctheta_%7Best%7D%20%3D%20%5Calpha%28%5Ctheta_%7Bprev%7D%20%2B%20%5Cdot%7B%5Ctheta%7D_%7Bgyro%7D%20%5CDelta%20t%29%20%2B%20%281-%5Calpha%29%5Ctheta_%7Bacc%7D" alt="\theta_{est} = \alpha(\theta_{prev} + \dot{\theta}_{gyro} \D..." /></div>


| | Complementary | Kalman |
|---|---|---|
| Computation | ◎ Minimal | ○ Moderate (matrix ops) |
| Tuning | Single α | Multiple Q, R params |
| Optimality | Heuristic | Optimal for linear-Gaussian |
| Bias estimation | None | Automatic |

---

### 6. Stability Analysis

#### 6.1 Open-Loop Instability

The characteristic equation det(sI − A) = 0 gives:


<div align="center"><img src="https://latex.codecogs.com/svg.image?s%5E2%20-%20%5Calpha%20%3D%200%20%5Cimplies%20s%20%3D%20%5Cpm%5Csqrt%7B%5Calpha%7D" alt="s^2 - \alpha = 0 \implies s = \pm\sqrt{\alpha}..." /></div>


Since α > 0, the **open-loop system is unstable** (positive real pole).

#### 6.2 Closed-Loop Stability (PD Control)

With u = −Kp·θ − Kd·θ̇, the closed-loop characteristic equation becomes:


<div align="center"><img src="https://latex.codecogs.com/svg.image?s%5E2%20%2B%20%5Cbeta%20K_d%20s%20%2B%20%28%5Cbeta%20K_p%20-%20%5Calpha%29%20%3D%200" alt="s^2 + \beta K_d s + (\beta K_p - \alpha) = 0..." /></div>


By the Hurwitz criterion, stability requires:

1. β·Kd > 0 (positive damping)
2. β·Kp > α → Kp > g/l (restoring force exceeds gravitational torque)

#### 6.3 Gain & Phase Margins

For robust stability: target **phase margin > 30°** and **gain margin > 6 dB**. However, in RC-servo-driven systems, the servo's own response lag (deadband, PWM update rate) dominates, making classical margin analysis approximate.

---

### 7. Implementation-Specific Constraints

#### 7.1 RC Servo Nonlinearities

| Characteristic | Impact | Mitigation |
|----------------|--------|------------|
| Deadband (~±5°) | Small commands ignored | Scale via `kpower` |
| Speed saturation | Cannot recover from large tilt | `ANGLE_LIMIT` for early shutoff |
| Asymmetric response | Left/right imbalance | `MOTOR_SIGN` for direction flip |
| Backlash | Mechanical oscillation | Increase D-term |

#### 7.2 IMU Specifications (MPU6886)

| Parameter | Typical Value | Relevance |
|-----------|--------------|-----------|
| Gyro noise density | 0.01 °/s/√Hz | Sets Q in Kalman filter |
| Accelerometer noise | 100 μg/√Hz | Sets R in Kalman filter |
| Gyro drift | ±1 °/s (temp-dependent) | Compensated by startup calibration |
| Max sample rate | 1 kHz | Sufficient for 100 Hz control |

#### 7.3 Computational Budget at 100 Hz

| Task | Time (ESP32 @ 240 MHz) |
|------|----------------------|
| IMU read (I²C) | ~1 ms |
| Kalman filter | ~0.1 ms |
| PID computation | < 0.01 ms |
| Servo PWM output | < 0.01 ms |
| LCD update (10 Hz) | ~5 ms |

Total budget: 10 ms/cycle — well within ESP32 capability. LCD rendering is decimated to 10 Hz to avoid jitter.

---

### 8. Tuning Guidelines

#### 8.1 Ziegler–Nichols Method

Classical approach using ultimate gain K_u and ultimate period T_u:


<div align="center"><img src="https://latex.codecogs.com/svg.image?K_p%20%3D%200.6K_u%2C%20%5Cquad%20K_i%20%3D%20%5Cfrac%7B1.2K_u%7D%7BT_u%7D%2C%20%5Cquad%20K_d%20%3D%200.075K_u%20T_u" alt="K_p = 0.6K_u, \quad K_i = \frac{1.2K_u}{T_u}, \quad K_d = 0...." /></div>


⚠️ Difficult to apply directly to inverted pendulums (open-loop unstable → cannot safely observe sustained oscillation).

#### 8.2 Practical Manual Tuning

1. Start with Kd: hand-hold the unit, increase Kd for damping
2. Raise Kp: until the unit attempts self-righting
3. Fine-tune Ki: eliminate slow drift
4. Add Kds, Kspd: suppress translational runaway

#### 8.3 Physical Interpretation

| Parameter | Physical Analogy | Too Large | Too Small |
|-----------|-----------------|-----------|-----------|
| Kp | Spring constant | High-frequency oscillation | Falls over |
| Ki | Drift corrector | Low-frequency instability | Slow lean |
| Kd | Viscous damper | Sluggish response | Undamped oscillation |
| Kds | Position spring | Position oscillation | Runs away |
| Kspd | Velocity damper | Sluggish translation | Unbounded acceleration |

---

### 9. Advanced Topics

#### 9.1 LQR (Linear Quadratic Regulator)

Minimizes the cost function:


<div align="center"><img src="https://latex.codecogs.com/svg.image?J%20%3D%20%5Cint_0%5E%5Cinfty%20%28%5Cmathbf%7Bx%7D%5ET%20Q%5Cmathbf%7Bx%7D%20%2B%20u%5ET%20Ru%29%5C%2Cdt" alt="J = \int_0^\infty (\mathbf{x}^T Q\mathbf{x} + u^T Ru)\,dt..." /></div>


via the algebraic Riccati equation. Unlike PID, LQR **optimizes all state variables simultaneously**.

#### 9.2 Model Predictive Control (MPC)

Solves a constrained optimization over a finite prediction horizon at each timestep. Handles actuator saturation and angle limits explicitly but requires significant computation.

#### 9.3 Reinforcement Learning

Learns control policy through trial-and-error without an explicit model. Sim-to-Real transfer (train in simulation, deploy on MCU) is an active research area.

---

## 📚 References / 参考文献

- Åström, K.J. & Murray, R.M. (2021). *Feedback Systems: An Introduction for Scientists and Engineers*, 2nd ed. Princeton University Press.
- Ogata, K. (2010). *Modern Control Engineering*, 5th ed. Prentice Hall.
- Welch, G. & Bishop, G. (2006). *An Introduction to the Kalman Filter*. UNC Chapel Hill TR 95-041.
- Interface誌 2025年9月号「モータ制御プログラミング入門」第3章 — 仁野新一 著
- TKJElectronics/KalmanFilter: https://github.com/TKJElectronics/KalmanFilter
