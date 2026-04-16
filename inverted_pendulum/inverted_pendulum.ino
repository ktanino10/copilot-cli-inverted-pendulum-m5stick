/*
 * 倒立振子 (Inverted Pendulum) メイン制御ファームウェア
 * for M5StickC Plus2 + FS90R サーボ × 2
 *
 * Hardware:
 *   - M5StickC Plus2 (ESP32, IMU: MPU6886)
 *   - FS90R 連続回転サーボ × 2
 *   - Servo1 PWM: G0,  Servo2 PWM: G26
 *
 * Usage:
 *   1. 電源ON → ジャイロ自動キャリブレーション (静止させること)
 *   2. ロボットを直立させ M5ボタン長押し → 倒立制御スタート
 *   3. もう一度長押し → 制御ストップ
 *
 * Libraries (Arduino IDE):
 *   - M5StickCPlus2  (Board: M5Stack)
 *   - Kalman         (TKJElectronics/KalmanFilter)
 *   - ESP32Servo
 *
 * Based on Interface Magazine 2025/09
 */

#include <M5StickCPlus2.h>
#include <Kalman.h>
#include <ESP32Servo.h>

// ============================================================
//  ハードウェア設定
// ============================================================
#define SERVO1_PIN  0
#define SERVO2_PIN  26

// ============================================================
//  PID パラメータ (Interface誌デフォルト値)
// ============================================================
float kpower = 0.001;
float kp     = 21.0;
float ki     = 7.0;
float kd     = 1.6;
float kdst   = 0.07;   // 位置補正ゲイン
float kspd   = 2.5;    // 速度補正ゲイン

// ============================================================
//  制御パラメータ
// ============================================================
#define ANGLE_LIMIT       45.0    // 緊急停止角度 [deg]
#define INTEGRAL_LIMIT   100.0    // 積分項上限 (anti-windup)
#define MOTOR_LIMIT       90      // サーボ出力上限 (90±MOTOR_LIMIT)
#define CONTROL_HZ       100     // 制御ループ周波数 [Hz]
#define DISPLAY_HZ        10     // 画面更新周波数 [Hz]
#define GYRO_CAL_SAMPLES 200     // ジャイロキャリブレーション回数
#define LONG_PRESS_MS   1000     // 長押し判定 [ms]

// IMU軸設定 — 取り付け向きに応じて変更
// GYRO_SIGN: ジャイロの符号 (ロボットが逆に動く場合 -1 にする)
// MOTOR_SIGN: モーター回転方向 (逆走する場合 -1 にする)
#define GYRO_SIGN   1
#define MOTOR_SIGN  1

// ============================================================
//  グローバル変数
// ============================================================
// タイミング定数
const unsigned long CONTROL_INTERVAL = 1000 / CONTROL_HZ;
const unsigned long DISPLAY_INTERVAL = 1000 / DISPLAY_HZ;

// ハードウェアオブジェクト
Kalman kalmanFilter;
Servo servo1, servo2;

// 状態
enum State { IDLE, RUNNING };
State state = IDLE;

// IMU キャリブレーション
float gyroXOffset = 0.0;
float gyroYOffset = 0.0;
float gyroZOffset = 0.0;
float angleOffset = 0.0;   // 直立時の基準角度

// センサ値
float kalmanAngle   = 0.0;
float currentGyro   = 0.0;

// PID 内部状態
float pidIntegral   = 0.0;
float wheelPosition = 0.0;
float wheelSpeed    = 0.0;
int   motorOutput   = 0;

// タイミング
unsigned long prevControlTime = 0;
unsigned long prevDisplayTime = 0;

// ボタン
unsigned long btnDownTime = 0;
bool btnIsDown = false;

// ============================================================
//  IMU
// ============================================================

// ジャイロオフセットをキャリブレーション (静止状態で呼ぶ)
void calibrateGyro() {
  float sumX = 0, sumY = 0, sumZ = 0;
  auto imu_update = StickCP2.Imu.getImuData();
  for (int i = 0; i < GYRO_CAL_SAMPLES; i++) {
    imu_update = StickCP2.Imu.getImuData();
    sumX += imu_update.gyro.x;
    sumY += imu_update.gyro.y;
    sumZ += imu_update.gyro.z;
    delay(5);
  }
  gyroXOffset = sumX / GYRO_CAL_SAMPLES;
  gyroYOffset = sumY / GYRO_CAL_SAMPLES;
  gyroZOffset = sumZ / GYRO_CAL_SAMPLES;
}

float getAccAngle() {
  auto data = StickCP2.Imu.getImuData();
  return atan2(data.accel.y, data.accel.z) * RAD_TO_DEG;
}

// ============================================================
//  モーター制御
// ============================================================

void setMotors(int output) {
  output = constrain(output, -MOTOR_LIMIT, MOTOR_LIMIT);
  motorOutput = output;
  int cmd = MOTOR_SIGN * output;
  // 左右サーボは鏡像配置のため逆回転
  servo1.write(90 + cmd);
  servo2.write(90 - cmd);
}

void stopMotors() {
  servo1.write(90);
  servo2.write(90);
  motorOutput = 0;
}

// ============================================================
//  PID 制御
// ============================================================

void resetPID() {
  pidIntegral   = 0.0;
  wheelPosition = 0.0;
  wheelSpeed    = 0.0;
  motorOutput   = 0;
}

int computePID(float angle, float gyroRate, float dt) {
  float error = -angle;  // 目標角度 = 0° (直立)

  // --- P ---
  float P = kp * error;

  // --- I (anti-windup) ---
  pidIntegral += error * dt;
  pidIntegral = constrain(pidIntegral, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);
  float I = ki * pidIntegral;

  // --- D (ジャイロを直接使用 = 微分ノイズ低減) ---
  float D = kd * (-gyroRate);

  // --- 位置/速度補正 (ドリフト防止) ---
  float posComp = kdst * wheelPosition + kspd * wheelSpeed;

  // --- 出力 ---
  float power = P + I + D + posComp;
  int output = (int)(kpower * power);
  output = constrain(output, -MOTOR_LIMIT, MOTOR_LIMIT);

  // 車輪位置推定を更新
  wheelSpeed = (float)output;
  wheelPosition += wheelSpeed * dt;
  wheelPosition = constrain(wheelPosition, -1000.0f, 1000.0f);

  return output;
}

// ============================================================
//  ディスプレイ
// ============================================================

void updateDisplay() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);

  // ステータス
  StickCP2.Display.setCursor(5, 5);
  if (state == RUNNING) {
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.print("RUNNING");
  } else {
    StickCP2.Display.setTextColor(ORANGE);
    StickCP2.Display.print("IDLE");
  }

  // バッテリー電圧
  int bat = StickCP2.Power.getBatteryLevel();
  StickCP2.Display.setTextColor(bat > 20 ? WHITE : RED);
  StickCP2.Display.setCursor(170, 5);
  StickCP2.Display.printf("%d%%", bat);

  // 区切り線
  StickCP2.Display.drawFastHLine(0, 28, 240, DARKGREY);

  // 傾斜角
  float angle = kalmanAngle - angleOffset;
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(5, 35);
  StickCP2.Display.printf("Ang:%+6.1f", angle);

  // モーター出力
  StickCP2.Display.setCursor(5, 58);
  StickCP2.Display.printf("Out:%+4d", motorOutput);

  // ジャイロ
  StickCP2.Display.setCursor(5, 81);
  StickCP2.Display.printf("Gyr:%+6.1f", currentGyro);

  // 操作説明
  StickCP2.Display.drawFastHLine(0, 105, 240, DARKGREY);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(5, 112);
  StickCP2.Display.print("[M5] Long press: Start / Stop");
}

// ============================================================
//  制御の開始 / 停止
// ============================================================

void startControl() {
  // 現在の角度を直立基準としてキャリブレーション
  angleOffset = getAccAngle();
  kalmanFilter.setAngle(angleOffset);
  kalmanAngle = angleOffset;
  resetPID();
  state = RUNNING;
  Serial.println(">>> Control STARTED");
}

void stopControl() {
  stopMotors();
  state = IDLE;
  Serial.println(">>> Control STOPPED");
}

// ============================================================
//  setup
// ============================================================

void setup() {
  auto cfg = M5.config();
  cfg.output_power = true;  // 5V外部出力を有効化
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);   // 横向き
  Serial.begin(115200);

  // --- キャリブレーション画面 ---
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(10, 20);
  StickCP2.Display.println("Calibrating...");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("Keep still!");

  // ジャイロキャリブレーション
  calibrateGyro();

  // カルマンフィルタ初期化
  float initAngle = getAccAngle();
  kalmanFilter.setAngle(initAngle);
  kalmanAngle  = initAngle;
  angleOffset  = initAngle;

  // サーボ初期化 (停止状態)
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  stopMotors();

  // タイミング初期化
  prevControlTime = millis();
  prevDisplayTime = millis();

  // Ready
  updateDisplay();

  Serial.println("=== Inverted Pendulum Ready ===");
  Serial.printf("Gyro offset: X=%.2f Y=%.2f Z=%.2f\n",
                gyroXOffset, gyroYOffset, gyroZOffset);
  Serial.printf("Angle offset: %.2f deg\n", angleOffset);
  Serial.println("CSV: angle, gyro, output");
}

// ============================================================
//  loop
// ============================================================

void loop() {
  StickCP2.update();
  unsigned long now = millis();

  // ---- ボタン処理 (M5 長押し) ----
  if (StickCP2.BtnA.wasPressed()) {
    btnDownTime = now;
    btnIsDown = true;
  }
  if (StickCP2.BtnA.wasReleased()) {
    if (btnIsDown && (now - btnDownTime >= LONG_PRESS_MS)) {
      if (state == IDLE) {
        startControl();
      } else {
        stopControl();
      }
    }
    btnIsDown = false;
  }

  // ---- 制御ループ (100 Hz) ----
  if (now - prevControlTime >= CONTROL_INTERVAL) {
    float dt = (now - prevControlTime) / 1000.0f;
    prevControlTime = now;

    // IMU 読み取り
    auto imu = StickCP2.Imu.getImuData();
    float ax = imu.accel.x, ay = imu.accel.y, az = imu.accel.z;
    float gx = imu.gyro.x,  gy = imu.gyro.y,  gz = imu.gyro.z;

    // ジャイロオフセット補正
    gx -= gyroXOffset;
    gy -= gyroYOffset;
    gz -= gyroZOffset;

    // 加速度センサからの角度 [deg]
    float accAngle = atan2(ay, az) * RAD_TO_DEG;

    // カルマンフィルタで姿勢推定
    float gyroInput = GYRO_SIGN * gx;
    kalmanAngle = kalmanFilter.getAngle(accAngle, gyroInput, dt);
    currentGyro = gyroInput;

    if (state == RUNNING) {
      float angle = kalmanAngle - angleOffset;

      // 安全停止
      if (fabs(angle) > ANGLE_LIMIT) {
        stopControl();
        Serial.println("!!! ANGLE LIMIT — emergency stop !!!");
        return;
      }

      // PID → モーター
      int output = computePID(angle, gyroInput, dt);
      setMotors(output);

      // シリアル出力 (Arduino Serial Plotter 対応)
      Serial.printf("%.2f,%.2f,%d\n", angle, gyroInput, output);
    }
  }

  // ---- ディスプレイ更新 (10 Hz) ----
  if (now - prevDisplayTime >= DISPLAY_INTERVAL) {
    prevDisplayTime = now;
    updateDisplay();
  }
}
