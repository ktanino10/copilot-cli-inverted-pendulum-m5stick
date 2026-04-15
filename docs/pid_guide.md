# 🎓 PID制御ガイド — 倒立振子をゼロから理解する
# PID Control Guide — Understanding the Inverted Pendulum from Scratch

> Interface誌 2025年9月号「モータ制御プログラミング入門」第3章（仁野新一氏 著）の内容をもとに、初心者向けにまとめたガイドです。
>
> Based on Interface Magazine Sep 2025, Chapter 3: "Understanding PID Control with a Simple Inverted Pendulum" by Shinichi Nino.

**[日本語](#日本語) | [English](#english)**

---

<a id="日本語"></a>
## 日本語

### 1. PID制御って何？

PID制御は、「目標の状態」と「今の状態」のズレ（＝誤差）を使って、機械を動かす力を計算する方法です。3つの要素の頭文字を取って「PID」と呼ばれます。

| 要素 | 正式名 | 何をしているか | 日常の例 |
|------|--------|--------------|---------|
| **P** | 比例 (Proportional) | 誤差の大きさに比例した力を出す | 傾いた分だけ手を動かす |
| **I** | 積分 (Integral) | 小さな誤差の蓄積を補正する | ずっと少しだけ傾いている → じわじわ修正 |
| **D** | 微分 (Derivative) | 誤差の変化速度を見てブレーキをかける | 「倒れそう！」と思ったら早めに手を動かす |

### 2. 人間は無意識にPID制御している

PID制御は難しい数式なしでは説明できない技術ではありません。**人が直感的に行っている動作と非常に似ています。**

#### ほうきバランスの例え

手のひらの上にほうきを立てて、倒さないようにバランスを取る遊びを想像してください。

1. **ほうきが右に傾いた** → 手を右に動かす（**P**: 傾きに比例して動く）
2. **ほうきがずっと微妙に右に傾いている** → 少しずつ右に寄せていく（**I**: 誤差の蓄積を解消）
3. **ほうきが急に左に倒れそう** → 素早く左に手を動かす（**D**: 変化の速さに反応）

この「目（センサ）」「脳（制御器）」「手（アクチュエータ）」の関係が、そのままロボットのPID制御の構造になります。

```
[目標: ほうきを立てたまま]
        |
        v
  ┌─────────────┐
  │  傾き偏差    │──→ 手加減の調整 ──→ 手のひらを動かす ──→ ほうき
  │  位置偏差    │      P: 傾きに比例                        │
  └─────────────┘      D: 倒れ速度                          │
        ^              I: 位置ずれ蓄積                       │
        │                                                    │
        └──────── 目で見る（センサ） ──────────────────────────┘
```

#### 箱を押す例え

床の上の箱を目標の場所まで滑らせるとき：

- **遠い** → 力いっぱい押す（P）
- **近づいた** → 軽く押す（P）
- **通り過ぎそう** → ブレーキ（D）
- **ちょっとだけ届かない状態が続く** → じわじわ押す（I）

### 3. 倒立振子にPIDを使うとどうなる？

倒立振子は、**棒（ボディ）の上にマイコンを乗せ、車輪を回してバランスを取るロボット**です。人が乗る「セグウェイ」と同じ原理です。

#### 倒立振子 ＝ ほうきバランス

| ほうきバランス | 倒立振子 |
|--------------|---------|
| 目（見る） | IMUセンサ（傾きを測る） |
| 脳（考える） | マイコン（PID計算） |
| 手（動かす） | サーボモータ（車輪を回す） |
| ほうき | ロボットのボディ |

#### 制御の数式

```
Power = P項 + I項 + D項

P項 = kp × Angle（傾き角度）
I項 = ki × ΣAngle（角度の積分＝蓄積した傾き）
D項 = kd × dAngle（角速度＝傾きの変化速度）
```

**倒れそうな方向に車輪を回す** → 支点が重心の真下に来る → 倒れない

### 4. それだけじゃ足りない — 位置・速度の補正

PID で角度だけ制御すると、ロボットは倒れませんが **どんどん走り出してしまいます**。壁にぶつかるまで止まりません。

これを防ぐために、**位置と速度のフィードバック**を追加します：

```
Power = P.Angle + I.Angle + D.Angle + Kds×位置 + Kspd×速度
```

- **Kds（位置係数）**: 元の場所から離れたら戻す力
- **Kspd（速度係数）**: 走り出したらブレーキをかける力

```
制御ブロック図:

                  ┌─────────────────────────┐
  角度目標(0°) ──→│  姿勢制御               │
                  │  P(kp) + I(ki) + D(kd)  │──┐
                  └─────────────────────────┘  │
                                                ├──→ 駆動力 → モータ → ボディ
                  ┌─────────────────────────┐  │         ↑
  位置目標(0) ───→│  位置・速度制御          │──┘         │
                  │  Kds + Kspd             │        IMUセンサ
                  └─────────────────────────┘     (角度・角速度・位置)
```

### 5. なぜマイコンとPIDは相性がいいのか

- **計算が軽い**: 足し算・掛け算だけなので、ESP32のような安価なマイコンでも余裕で動く
- **試行錯誤しやすい**: 係数（kp, ki, kd）を変えるだけで動きが変わる。実機で試しながら調整できる
- **汎用性が高い**: 位置制御、トルク制御、速度制御、温度制御… あらゆる場面で使える

### 6. 理論と実践のギャップ

教科書通りにやっても、実際の機械はうまく動かないことが多いです。原因は：

| 理論では無視されがち | 実際には… |
|-------------------|---------|
| 部品は全部同じ性能 | ばらつきがある（同じモータでも回転数が違う） |
| 摩擦はゼロ | 静止摩擦 ≠ 動摩擦、温度でも変わる |
| センサは正確 | ノイズがある、遅延がある |
| 計算は瞬時 | メカの応答にはタイムラグがある |

**だから PID の係数は、理論値をスタート地点にして、実機で試行錯誤して決めます。** これは経験がものを言う世界です。

### 7. このプロジェクトのハードウェア構成

| 部品 | 型番・説明 | 役割 |
|------|---------|------|
| マイコン | M5StickC Plus | IMU（MPU6886）とバッテリ内蔵。Arduino IDEで開発 |
| サーボ | FS90R × 2 | 連続回転RCサーボ。PWM信号で速度制御。約500円/個 |
| フレーム | 3Dプリント製 | 筆者（仁野氏）が設計した専用ボディ |

#### 構成の特徴

- **安い**: 合計3万円以下で作れる（市販キットは数万円〜）
- **シンプル**: エンコーダなし、DCモータドライバなし。ソフトウェアで頑張る構成
- **簡易だが課題もある**: RCサーボはトルク・精度に限界があり、高速な制御が難しい。回転速度はPWMで直接制御するため、細かい調整には限界がある

### 8. PIDパラメータの調整手順

実際に倒立振子を立たせるための係数調整の流れです：

#### ステップ1: 初期化
全係数をゼロにし、本体を垂直に保持して電源を入れる。手で支えながら調整スタート。

#### ステップ2: P項（kp）の調整
手で支えた状態で kp を上げていく。
- 軽く手を離して倒れかけたとき、**起き上がろうとする力**が出るまで上げる
- 上げすぎると振動する → 少し戻す

#### ステップ3: D項（kd）の調整
kd を上げて、振動を抑える。
- D項は「ブレーキ」の役割。行き過ぎたときの揺れ戻しを小さくする
- 上げすぎると反応が鈍くなる

#### ステップ4: I項（ki）の調整
ゆっくり振動する（安定しない）ときに ki を上げる。
- 微小な傾きが蓄積して徐々にずれていくのを補正する
- 上げすぎると大きく振動して発散する

> **コツ**: きわめて微妙な調整です。数値を少しずつ変えて、何度も試すしかありません。

### 9. さらに学びたい方へ

Interface誌では続編として、以下の内容も解説されています：

- IMUのノイズ処理（相補フィルタ / カルマンフィルタ）
- M5StickCのIMU特性と補正
- ESP32のタイマーを使った正確な制御周期の実現
- BLE（Bluetooth Low Energy）による無線コントロール

詳しくは [Interface誌 2025年9月号](https://interface.cqpub.co.jp/202509st/) をご覧ください。

---

<a id="english"></a>
## English

### 1. What Is PID Control?

PID control calculates how much force to apply to a machine by measuring the gap ("error") between a **target state** and the **current state**. It's named after its three components:

| Term | Full Name | What It Does | Everyday Example |
|------|-----------|-------------|-----------------|
| **P** | Proportional | Output force proportional to current error | Move your hand as much as the stick tilts |
| **I** | Integral | Correct accumulated small errors over time | Stick keeps leaning slightly right → slowly nudge it |
| **D** | Derivative | Apply braking based on how fast the error changes | "It's about to fall!" → react quickly |

### 2. You Already Do PID Control Every Day

PID control is not some abstract mathematical concept. **It closely resembles what humans do instinctively.**

#### The Broom-Balancing Analogy

Imagine balancing a broom upright on your palm:

1. **Broom tilts right** → move hand right (P: respond proportionally to tilt)
2. **Broom keeps slightly leaning right** → gradually shift further right (I: correct accumulated drift)
3. **Broom suddenly starts falling left** → quickly move hand left (D: react to speed of change)

The relationship between your **eyes (sensor)**, **brain (controller)**, and **hand (actuator)** maps directly to the structure of a PID-controlled robot.

```
[Goal: keep the broom upright]
        |
        v
  ┌─────────────────┐
  │  Tilt error      │──→ Adjust hand motion ──→ Move palm ──→ Broom
  │  Position error  │      P: proportional to tilt              │
  └─────────────────┘      D: falling speed                     │
        ^                  I: accumulated drift                  │
        │                                                        │
        └────────── Eyes (Sensor) ───────────────────────────────┘
```

#### The Box-Pushing Analogy

Sliding a box across the floor to a target position:

- **Far away** → push hard (P)
- **Getting close** → push gently (P)
- **About to overshoot** → brake (D)
- **Stuck just short of the target** → keep nudging (I)

### 3. How PID Controls an Inverted Pendulum

An inverted pendulum is a **robot with a body (stick) balanced on wheels**. It's the same principle as a Segway.

#### Inverted Pendulum = Broom Balancing

| Broom Balancing | Inverted Pendulum |
|----------------|-------------------|
| Eyes (see) | IMU sensor (measure tilt) |
| Brain (think) | Microcontroller (PID math) |
| Hand (move) | Servo motors (spin wheels) |
| Broom | Robot body |

#### The Control Equation

```
Power = P_term + I_term + D_term

P_term = kp × Angle  (tilt angle)
I_term = ki × ΣAngle  (integral of angle = accumulated tilt)
D_term = kd × dAngle  (angular velocity = rate of tilt change)
```

**The wheels spin toward the direction of falling** → the support point moves under the center of gravity → the robot stays upright.

### 4. Angle Alone Isn't Enough — Position & Speed Correction

With angle-only PID, the robot won't fall over, but it **will keep rolling forward forever** until it hits a wall.

To prevent this, **position and speed feedback** are added:

```
Power = P.Angle + I.Angle + D.Angle + Kds × Position + Kspd × Speed
```

- **Kds (position gain)**: Pull back toward the starting position
- **Kspd (speed gain)**: Brake when it starts rolling

```
Control Block Diagram:

                  ┌──────────────────────────┐
  Angle target(0°)│  Attitude Control         │
                  │  P(kp) + I(ki) + D(kd)   │──┐
                  └──────────────────────────┘  │
                                                 ├──→ Motor Power → Motor → Body
                  ┌──────────────────────────┐  │         ↑
  Position(0) ───→│  Position/Speed Control   │──┘         │
                  │  Kds + Kspd              │        IMU Sensor
                  └──────────────────────────┘    (angle, angular velocity, position)
```

### 5. Why Microcontrollers and PID Are a Perfect Match

- **Low computation**: Just addition and multiplication — runs easily on cheap MCUs like ESP32
- **Easy to experiment**: Change coefficients (kp, ki, kd) and see immediate results on the real machine
- **Highly versatile**: Position control, torque control, speed control, temperature control… works everywhere

### 6. The Gap Between Theory and Practice

Textbook PID often doesn't work on real hardware. Here's why:

| Theory Assumes | Reality |
|---------------|---------|
| All components are identical | Parts vary (even same-model motors have different RPMs) |
| Zero friction | Static ≠ dynamic friction; changes with temperature |
| Perfect sensors | Noise, latency, drift |
| Instant computation | Mechanical response has time lag |

**That's why PID coefficients are tuned by trial and error on the actual machine.** Theory gives you a starting point; experience gets you the rest of the way.

### 7. Hardware Setup for This Project

| Component | Model / Details | Role |
|-----------|----------------|------|
| MCU | M5StickC Plus | Built-in IMU (MPU6886) and battery. Programmed via Arduino IDE |
| Servos | FS90R × 2 | Continuous-rotation RC servos. Speed controlled by PWM. ~$5 each |
| Frame | 3D-printed | Custom body designed by the article's author (Nino) |

#### Design Philosophy

- **Cheap**: Under $200 total (commercial kits cost much more)
- **Simple**: No encoders, no DC motor drivers. Software compensates for hardware limitations
- **Trade-offs**: RC servos have limited torque and precision; fine speed control is difficult with PWM-only

### 8. How to Tune PID Parameters

Step-by-step process to make the inverted pendulum actually balance:

#### Step 1: Initialize
Set all coefficients to zero. Hold the unit vertical and power on. Support it by hand while tuning.

#### Step 2: Tune P (kp)
Increase kp while holding the unit.
- When you slightly release it and it **tries to right itself**, you're in the ballpark
- Too high → oscillation. Back off slightly

#### Step 3: Tune D (kd)
Increase kd to dampen oscillation.
- D acts as a brake. It reduces overshoot and rebounds
- Too high → sluggish response

#### Step 4: Tune I (ki)
If the robot slowly drifts (low-frequency oscillation), increase ki.
- Corrects tiny accumulated errors that P alone misses
- Too high → large oscillations and instability

> **Tip**: This is an extremely delicate process. Change values in small increments and test repeatedly.

### 9. Further Reading

The Interface Magazine series continues with advanced topics:

- IMU noise processing (complementary filter / Kalman filter)
- M5StickC IMU characteristics and calibration
- Precise control timing using ESP32 hardware timers
- Wireless control via BLE (Bluetooth Low Energy)

See [Interface Magazine Sep 2025](https://interface.cqpub.co.jp/202509st/) for details.

---

---

> 📐 **上級者向けの理論編もあります / For the rigorous math, see:**
> **[docs/pid_theory.md](pid_theory.md)** — 運動方程式・状態空間・カルマンフィルタ・安定性解析

> 📖 **専門用語の解説 / Glossary of technical terms:**
> **[docs/glossary.md](glossary.md)** — 用語を平易な言葉で解説した補足資料集 / Plain-language explanations for every technical term

---

## 📚 Source / 出典

This guide is based on content from:

- **Interface誌 2025年9月号「モータ制御プログラミング入門」第3章** — 仁野 新一 著
- Published by CQ出版社: https://interface.cqpub.co.jp/202509st/
- Author's blog: https://n-shinichi.hatenablog.com/entry/2025/08/31/163903
