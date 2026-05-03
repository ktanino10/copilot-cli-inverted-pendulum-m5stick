# 🎛️ 制御パラメータ完全リファレンス
# Control Parameter Complete Reference

> ダッシュボード (`tools/templates/index.html`) と M5 ファームウェア (`inverted_pendulum/inverted_pendulum.ino`) で調整できる **全制御パラメータの意味** と **おすすめチューニング順序** をまとめています。
>
> Complete reference for every tunable parameter exposed by the dashboard (`tools/templates/index.html`) and the M5 firmware (`inverted_pendulum/inverted_pendulum.ino`), plus a recommended tuning workflow.

**[日本語](#日本語) | [English](#english)**

---

<a id="日本語"></a>
## 日本語

### 1. パラメータ早見表

ダッシュボードや `?` シリアルコマンドで表示される全変数の意味です。

| 変数 | 既定値 | カテゴリ | 役割 | 一言で言うと |
|---|---|---|---|---|
| **`Pitch_offset`** (`po`) | 0.0 | 校正 | atan2 + offset で「直立 = 0°」を作る | センサーの **基準点** 校正 |
| `Pitch_offset2` (`po2`) | 0.0 | 校正 | 二次微調整 | 一次校正後の細かいズレ補正 |
| `targetBias` (`bias`) | 0.0 | 校正 | 目標姿勢ずらし | 故意に前/後ろへ少しだけ傾けたい時 |
| **`kp`** | 10.0 | PID | 角度誤差比例ゲイン | 「傾いた量」に比例した復元力 |
| **`kd`** | 0.35 | PID | 角速度比例ゲイン (微分項) | 「傾く速さ」へのブレーキ ＝ 振動止め |
| `ki` | 0.0 | PID | 角度誤差積分ゲイン | 定常偏差（常に片側へ倒れる）を消す |
| **`kspd`** | 0.15 | 速度抑制 | 速度フィードバック | 倒立してるのに車輪が走り続ける（暴走）抑制 |
| `kpower` | 0.0001 | 速度抑制 | power 積分の係数 | `kspd` と組で「車輪移動量」を生成 |
| `kdst` | 0.0 | 位置 | 累積位置を中央へ戻す | 普段 0 で OK、長距離走行抑制用 |
| `power_limit` (`plim`) | 350 | 出力 | power の絶対値上限 | サーボ飽和防止のキャップ |
| `power_pos_scale` (`ppos`) | 0.85 | 出力 | +方向出力スケール | 前進方向の出力倍率 |
| `power_neg_scale` (`pneg`) | 1.70 | 出力 | -方向出力スケール | 後退方向の出力倍率（個体差補正） |
| `min_drive_power` (`minp`) | 0 | 出力 | デッドゾーン突破最低出力 | 小角度で動き出すための最低出力 |
| `min_drive_angle` (`minang`) | 3.0 | 出力 | デッドゾーン適用角度 | 何度以上傾いたら `minp` を効かせるか |
| **`angle_limit`** (`alim`) | 20.0 | 安全 | PID 有効角度範囲 | これ超えたら LIMIT モード(モーター停止) |
| `fil_N` | 5 | フィルタ | Pitch ローパス窓サンプル数 | 大きいほど滑らかだが遅延 |

---

### 2. カテゴリ別の意味

#### 🎯 校正系（最初に必ず合わせる）
- **`Pitch_offset`** が **すべての出発点**。これがズレてるとどんな PID も無駄
- 「直立姿勢で `angle = 0°` になる」よう物理的・電気的に補正する値
- IMU の取り付け角度のずれ・電源ON時の重力ベクトルからの算出誤差を吸収する
- シリアル `zero` コマンドや、ダッシュボード `po=` 入力で設定

#### 🎚️ PID 三役
- **`kp`** （比例）: 傾きに反応した即時の復元力。**大きすぎ → 振動**、**小さすぎ → 倒れる**
- **`kd`** （微分）: 揺れの "速さ" にブレーキを当てて減衰。**大きすぎ → 高周波振動**、**小さすぎ → スイングし続ける**
- **`ki`** （積分）: 微小な定常偏差を時間をかけて消す。**強すぎ → オーバーシュート＆発振**、不要なら 0

#### 🚗 速度抑制系（暴走止め）
- 物理的には「振子が立ってても、車輪が振子の真下を追いかけて走り続ける」ことが起きる
- `kpower` で `power` を積分して **車輪の積算移動量** を作り、`kspd` で「中央に戻れ」というフィードバックをかける
- これが無いと、立ったまま部屋の端まで走っていく

#### ⚙️ 出力スケーリング
- **`power_pos_scale` / `power_neg_scale`**: サーボの個体差で前後の出力強度が違う場合の補正
- 「常に後ろに逃げる」→ `pneg` を上げる（後退の力を強める）
- 「常に前に突っ込む」→ `ppos` を上げる
- **`power_limit`**: サーボの応答可能領域を超える指令を抑制。物理的サーボの限界

#### 🛡️ 安全装置
- **`angle_limit`**: ±この角度を超えたら **PID を完全停止** してモーターも切る（LIMIT モード）
- 大きすぎ → 倒れた状態でも PID が大出力指令を出し続けて発散・サーボ破損
- 小さすぎ → 少し揺れただけで停止して立ち直れない

---

### 3. おすすめチューニング順序（実証済みフロー）

> ⚠ **必ず順序通り**に進めてください。後ろのステップを先にやると、前の効果が見えません。

#### Step 0 — 机上の前準備
- `ki = 0`, `kspd = 0` にリセット (P+D だけで攻める)
- 振子を **手で支えた状態** で `?` コマンドの `angle` を確認
- バッテリー残量を確認（電圧低下で挙動変わる）

#### Step 1 — `Pitch_offset` を合わせる 🎯
> ここがズレてると以降のチューニング全部無駄になる、最重要ステップ
- 振子を完全に **直立** で支え、シリアルから `zero` または `po=<値>` で `angle ≈ 0°` に
- ±0.5° 以内に追い込む
- IMU の取り付け状態を変えたら毎回必要

#### Step 2 — `kp` を上げる（復元力）
- `kp = 5` から始めて、自立試行
- すぐ倒れる → `kp` を上げる（5 → 8 → 12 …）
- ガクガク振動し始める手前まで上げる
- 振動 = "もう少しでバランス取れるけど、修正力が強すぎてオーバーシュート" の状態

#### Step 3 — `kd` で減衰（揺れ止め）
- 目安: `kd ≈ kp / 25` （例: `kp = 10` なら `kd = 0.40` 前後）
- 振動が消えるまで `kd` を増やす
- 大きすぎるとセンサーノイズを拾って "高周波振動" → 動画で見ると微振動

#### Step 4 — `kspd` で暴走を止める（一番分かりにくい）
- 立ってるが **車輪が同方向にだんだん走り続ける** → `kspd` 必要
- 0.10 → 0.15 → 0.20 と少しずつ上げる
- 多すぎると「前後にスイング」、少なすぎると「スーッと逃げる」

#### Step 5 — 左右非対称を見つける（重要）
- 動画撮影 or 観察: **どっち方向に偏って倒れる**？
- 後ろに逃げるなら `power_neg_scale` を上げる（既定 1.70 — 既に補正済の場合あり）
- 前に行くなら `power_pos_scale` を上げる
- 片側だけ補正、もう片側は 1.0 が基本

#### Step 6 — `ki` は最後の最後
- 上記 5 ステップで「**ほぼ立ってるが少しだけ片側へドリフト**」する時のみ
- `ki = 0.5` → `1.0` と慎重に
- 大きすぎるとオーバーシュート＆発振 → 0 に戻して別の原因を疑う

---

### 4. 推奨「立ち上がり初期セット」

```
po       = (実測校正値、個体ごと違う)
kp       = 8.0     # 控えめスタート
kd       = 0.30    # kp の 1/27
ki       = 0.0     # 最後に追加
kspd     = 0.10    # 控えめ
ppos     = 1.0     # 個体差確認のため対称スタート
pneg     = 1.0     # 偏り見て後で調整
minp     = 0       # デッドゾーン補償は無効から
alim     = 20.0    # PID 範囲
plim     = 350     # 出力上限
```

ここから **`kp` → `kd` → `kspd` → 左右補正 → `ki`** の順に育てる。

> **既知の良好値**（n_shinichi 氏 + 本プロジェクト調整）:
> `kp=10.0, kd=0.35, ki=0.0, kspd=0.15, ppos=0.85, pneg=1.70, alim=20`

---

### 5. よくある罠と対処

| 症状 | 怪しい変数 | 対処 |
|---|---|---|
| 立ってもすぐ倒れる | `Pitch_offset` ズレ | `zero` で再校正 |
| 微振動が止まらない | `kd` 高すぎ or `fil_N` 低い | `kd` ↓ または `fil_N` を 7-9 に |
| 立つけど大きくスイング | `kp` 高すぎ | `kp` ↓ |
| 立つが片側に走り続ける | `kspd` 不足 | `kspd` ↑（0.05 ずつ） |
| 常に後ろへ逃げる | `power_neg_scale` 不足 | `pneg` ↑ |
| 常に前へ突っ込む | `power_pos_scale` 不足 | `ppos` ↑ |
| いきなり倒れる(LIMIT) | `angle_limit` 超え | `alim` 一時的に↑、根本は `kp/kd` 見直し |
| 起動直後だけ揺れて落ち着く | バッテリー or `kpower` | バッテリー充電・`kpower` 再確認 |
| 電圧下がると挙動変わる | `power_limit` の効きが変わる | `batt` 表示を見ながら定期的に再チューン |

---

### 6. ダッシュボードの自動チューニング機能

- **42パターン PID スイープ + ヒートマップ**: `kp` × `kd` の組み合わせを自動試行し、安定時間を可視化
- **セッション記録**: 各試行を `tools/sessions/` に JSON + CSV で保存、後で解析可能
- **解析ダッシュボード** (`/dash`): セッション間で `score` (角度安定性指標) を比較、傾向把握
- **🔗 LINK MONA**: チューニング中の状態を M5 LCD 上で視覚フィードバック（緑/黄/赤/黒）

詳細は `progress-log.md` の `2026-04-18: 42パターン自動PIDスイープ` 節を参照。

---

### 7. パラメータ変更方法

| 経路 | 永続性 | 用途 |
|---|---|---|
| ダッシュボード入力欄 | リブートで消える | 試行錯誤 |
| シリアルコマンド (`kp=12`) | リブートで消える | デバッグ・遠隔調整 |
| ファーム書き換え (`inverted_pendulum.ino`) | 永続 | 確定値の保存 |
| Preferences (NVS) 保存 | 永続 | 未実装、将来追加候補 |

---

<a id="english"></a>
## English

### 1. Parameter Cheat Sheet

Every variable exposed by the dashboard or shown by the `?` serial command:

| Variable | Default | Category | Role | In one phrase |
|---|---|---|---|---|
| **`Pitch_offset`** (`po`) | 0.0 | Calibration | Adds offset to atan2 so "upright = 0°" | The sensor **zero point** calibration |
| `Pitch_offset2` (`po2`) | 0.0 | Calibration | Secondary fine-tune | Tweak after initial calibration |
| `targetBias` (`bias`) | 0.0 | Calibration | Target tilt offset | Intentionally lean fwd/back slightly |
| **`kp`** | 10.0 | PID | Angle-error proportional gain | Restoring force ∝ tilt amount |
| **`kd`** | 0.35 | PID | Angular-velocity gain (derivative) | Brake on tilt speed = vibration damper |
| `ki` | 0.0 | PID | Angle-error integral gain | Erases steady drift to one side |
| **`kspd`** | 0.15 | Drift control | Velocity feedback | Stops "running away" while balanced |
| `kpower` | 0.0001 | Drift control | Coefficient on power integral | Pairs with `kspd` to estimate wheel travel |
| `kdst` | 0.0 | Position | Center-pull on accumulated position | Usually 0; long-drive suppression |
| `power_limit` (`plim`) | 350 | Output | Absolute power cap | Servo-saturation guard |
| `power_pos_scale` (`ppos`) | 0.85 | Output | +direction output scale | Forward gain multiplier |
| `power_neg_scale` (`pneg`) | 1.70 | Output | -direction output scale | Reverse gain (compensates servo asymmetry) |
| `min_drive_power` (`minp`) | 0 | Output | Dead-zone breakthrough power | Minimum push for small tilts |
| `min_drive_angle` (`minang`) | 3.0 | Output | Dead-zone activation angle | Below this angle `minp` doesn't fire |
| **`angle_limit`** (`alim`) | 20.0 | Safety | PID active range | Beyond this → LIMIT mode (motors off) |
| `fil_N` | 5 | Filter | Pitch low-pass window size | Bigger = smoother but more delay |

---

### 2. Meaning by Category

#### 🎯 Calibration (always tune first)
- **`Pitch_offset`** is **the foundation**. If it's off, no PID setting will save you.
- Adjusts so the sensor reads `angle = 0°` when the pendulum is physically upright
- Compensates for IMU mounting tilt and the gravity-vector-derivation error at boot
- Set via `zero` serial command or `po=` from the dashboard

#### 🎚️ PID Trio
- **`kp`** (Proportional): Immediate restoring force responding to tilt. **Too big → vibration**, **too small → falls**
- **`kd`** (Derivative): Brakes on the rate of tilt for damping. **Too big → high-frequency jitter**, **too small → keeps swinging**
- **`ki`** (Integral): Slowly erases tiny steady-state errors over time. **Too strong → overshoot & oscillation**; leave at 0 if not needed

#### 🚗 Drift Suppression (anti-runaway)
- Physically: the pendulum can stay balanced while the wheels keep racing across the room chasing the body's center of mass
- `kpower` integrates `power` to build a **virtual wheel-position estimate**, and `kspd` feeds back "return to center"
- Without this, you'll find the robot at the far wall, still vertical

#### ⚙️ Output Scaling
- **`power_pos_scale` / `power_neg_scale`**: compensate when the servo's forward and reverse strength differ
- "Always escapes backward" → raise `pneg` (more reverse force)
- "Always lunges forward" → raise `ppos`
- Adjust one side; keep the other at 1.0 as baseline

#### 🛡️ Safety
- **`angle_limit`**: when |tilt| exceeds this, **PID stops entirely** and motors cut (LIMIT mode)
- Too big → keeps pumping huge commands while fallen → diverges, may damage servos
- Too small → trips on minor wobble and can never recover

---

### 3. Recommended Tuning Workflow (battle-tested)

> ⚠ **Follow the order strictly.** Skipping ahead hides the effect of earlier steps.

#### Step 0 — Bench prep
- Reset `ki = 0`, `kspd = 0` (work with P+D only)
- Hold the pendulum upright by hand and verify `angle` via `?` command
- Check battery voltage (low batt changes behavior)

#### Step 1 — Calibrate `Pitch_offset` 🎯
> Most critical step — every later step depends on this being correct.
- Hold the pendulum perfectly upright, run `zero` or `po=<value>` until `angle ≈ 0°`
- Get within ±0.5°
- Re-run any time the IMU mounting changes

#### Step 2 — Raise `kp` (restoring force)
- Start at `kp = 5`, attempt self-balance
- Falls immediately → bump `kp` (5 → 8 → 12 …)
- Stop just before it starts rattling/vibrating
- Vibration here means "almost balanced, but the correction is overshooting"

#### Step 3 — Add `kd` (damping)
- Rule of thumb: `kd ≈ kp / 25` (e.g. `kp = 10` → `kd = 0.40` ish)
- Increase `kd` until the vibration disappears
- Too high → picks up sensor noise → "high-frequency jitter" visible in slow-mo video

#### Step 4 — Stop runaway with `kspd` (the trickiest knob)
- Balanced but **the wheels keep drifting in one direction** → need `kspd`
- Bump 0.10 → 0.15 → 0.20 in small steps
- Too much → fwd/back swinging; too little → silent escape

#### Step 5 — Find left/right asymmetry (important)
- Record video or watch carefully: **which direction does it favor falling/escaping?**
- Backward → raise `power_neg_scale` (default 1.70 — may already be tuned)
- Forward → raise `power_pos_scale`
- Bias one side, leave the other at 1.0

#### Step 6 — `ki` last (and lightly)
- Only after the above 5 steps if you still see "**balanced but slowly drifting to one side**"
- Try `ki = 0.5` → `1.0` cautiously
- Too much → overshoot & oscillation → reset to 0 and look for the real cause elsewhere

---

### 4. Recommended Starter Set

```
po       = (your measured calibration; differs per unit)
kp       = 8.0     # conservative start
kd       = 0.30    # ~kp/27
ki       = 0.0     # add last
kspd     = 0.10    # conservative
ppos     = 1.0     # symmetric until you measure asymmetry
pneg     = 1.0     # tune later
minp     = 0       # dead-zone comp off by default
alim     = 20.0    # PID range
plim     = 350     # output cap
```

Then evolve in order: **`kp` → `kd` → `kspd` → asymmetry → `ki`**.

> **Known-good set** (n_shinichi's values + this project's tuning):
> `kp=10.0, kd=0.35, ki=0.0, kspd=0.15, ppos=0.85, pneg=1.70, alim=20`

---

### 5. Common Pitfalls and Fixes

| Symptom | Suspect variable | Fix |
|---|---|---|
| Falls immediately on standup | `Pitch_offset` mis-calibrated | Re-run `zero` |
| Persistent micro-vibration | `kd` too high or `fil_N` too low | Lower `kd` or set `fil_N` to 7-9 |
| Stands but swings widely | `kp` too high | Lower `kp` |
| Stays vertical but drifts in one direction | Insufficient `kspd` | Bump `kspd` by 0.05 |
| Always escapes backward | `power_neg_scale` insufficient | Raise `pneg` |
| Always lunges forward | `power_pos_scale` insufficient | Raise `ppos` |
| Suddenly cuts out (LIMIT mode) | Exceeds `angle_limit` | Temporarily raise `alim`, root cause is `kp`/`kd` |
| Wobbles only at startup | Battery or `kpower` | Charge battery, recheck `kpower` |
| Behavior shifts as voltage drops | `power_limit` effective cap drifts | Watch the `batt` reading and re-tune periodically |

---

### 6. Dashboard Auto-Tuning Features

- **42-pattern PID sweep + heatmap**: auto-tries `kp` × `kd` combinations and visualizes settling time
- **Session recording**: each trial saved as JSON + CSV in `tools/sessions/` for post-analysis
- **Analytics dashboard** (`/dash`): compare `score` (angle stability metric) across sessions to spot trends
- **🔗 LINK MONA**: live visual feedback on the M5 LCD during tuning (green/amber/red/black)

See the `2026-04-18: 42-pattern automatic PID sweep` entry in `progress-log.md` for details.

---

### 7. How to Change Parameters

| Method | Persists? | Use case |
|---|---|---|
| Dashboard input field | No (gone on reboot) | Trial & error |
| Serial command (`kp=12`) | No (gone on reboot) | Debugging / remote tweaks |
| Edit firmware (`inverted_pendulum.ino`) | Yes (permanent) | Locking in confirmed values |
| Preferences (NVS) save | — | Not yet implemented (future work) |

---

> 📖 [README に戻る / Back to README](../README.md) ・ 📊 [PID 理論編 / PID Theory](pid_theory.md) ・ 🎓 [PID ガイド / PID Guide](pid_guide.md)
