# Progress Log

> This file is the detailed experiment log extracted from README.md.
> A daily record of the M5StickC Plus2 inverted pendulum project from 2026-04-13 to 2026-05-02 — soldering → firmware port → hardware troubles → IMU axis identification → PID tuning → GitHub Pages demo, etc.
>
> 📖 [Back to README](../README.md) · 🇯🇵 [日本語版](progress-log.ja.md)

---


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
