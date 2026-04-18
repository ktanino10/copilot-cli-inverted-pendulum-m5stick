/*
 * 倒立振子 (Inverted Pendulum) メイン制御ファームウェア
 * for M5StickC Plus2 + FS90R サーボ × 2
 *
 * n_shinichi氏のオリジナル実装に基づくpulse_drive方式。
 * ESP32Servoライブラリを使わず、digitalWriteによる手動パルス生成で
 * G0のブートストラップピン問題を回避。
 *
 * Hardware:
 *   - M5StickC Plus2 (ESP32, IMU: MPU6886)
 *   - FS90R 連続回転サーボ × 2
 *   - Servo1 PWM: G0,  Servo2 PWM: G26
 *   - サーボ電源: BAT端子から給電（5V出力が弱いため）
 *
 * Usage:
 *   1. 電源ON → ジャイロ自動キャリブレーション (静止させること)
 *   2. BtnA (GPIO37) 押す → モーターON/OFFトグル
 *   3. BtnB (GPIO39) 押す → オフセット調整モード切替
 *
 * Based on Interface Magazine 2025/09, n_shinichi's Plus2 sketch
 */

#include <M5StickCPlus2.h>
#include <Kalman.h>

// ============================================================
//  ハードウェア設定
// ============================================================
#define MOTOR_PIN_L  0     // G0: 左モーター（手動パルスなのでブート問題なし）
#define MOTOR_PIN_R  26    // G26: 右モーター
#define BTN_A        37    // 正面ボタン（digitalRead直接）
#define BTN_B        39    // 側面ボタン（digitalRead直接）

// ============================================================
//  サーボ設定（マイクロ秒ベース）
// ============================================================
// FS90Rは1500μsで停止、500-2500μsで回転
#define MOTOR_NEUTRAL  1500    // 停止パルス幅 [μs]
#define MOTOR_MIN      500     // 最小パルス幅 [μs]
#define MOTOR_MAX      2500    // 最大パルス幅 [μs]

// サーボニュートラル補正（個体差、EEPROMで保存可能）
int motor_offsetL = 0;
int motor_offsetR = 0;

// ============================================================
//  PID パラメータ（チューニング用：初期値0、ボタンで調整）
// ============================================================
float kpower = 0.003;   // 全体スケール（固定）
float kp     = 20.0;    // P項: 大きめにして応答を確認
float ki     = 0.0;     // I項
float kd     = 0.0;     // D項
float kspd   = 0.0;     // 速度補正
float kdst   = 0.0;     // 位置補正

// チューニングUI
int tuneParam = 0;       // 0=kp, 1=kd, 2=ki, 3=kspd, 4=kdst
const char* paramNames[] = {"kp", "kd", "ki", "kspd", "kdst"};
float* paramPtrs[5];     // setup()で初期化
float paramStep = 0.1;   // 調整ステップ

// ============================================================
//  制御パラメータ
// ============================================================
#define ANGLE_LIMIT      45.0   // 緊急停止角度 [deg]
#define I_LIMIT         300.0   // 積分項上限 (anti-windup)
#define GYRO_CAL_SAMPLES 500    // ジャイロキャリブレーション回数
#define FIL_N            5      // ローパスフィルタ係数

// ============================================================
//  グローバル変数
// ============================================================
Kalman kalman;
long lastUs = 0;

// IMU
float acc[3], accOffset[3];
float gyro[3], gyroOffset[3];
float dAngle;

// 制御状態
float Pitch, Pitch_filter, Angle;
float Pitch_offset = 0;  // ONにした瞬間の角度を基準にする
float Speed, Power;
float P_Angle, I_Angle, D_Angle, k_speed;
int16_t power, powerL, powerR;

// UI状態
unsigned char motor_sw = 0;      // 0=OFF, 1=ON
int wait_count = 0;
float batt;
unsigned long ms10, ms100, ms1000;

// ============================================================
//  手動パルス駆動（n_shinichi方式）
//  ESP32Servoを使わず、digitalWriteでパルスを生成
//  これによりG0のブートストラップ問題を回避
// ============================================================
void pulse_drive(int16_t pL, int16_t pR) {
  pL = constrain(pL, MOTOR_MIN, MOTOR_MAX);
  pR = constrain(pR, MOTOR_MIN, MOTOR_MAX);
  bool doneL = false;
  bool doneR = false;
  uint32_t usec = micros();
  digitalWrite(MOTOR_PIN_L, HIGH);
  digitalWrite(MOTOR_PIN_R, HIGH);

  while (!doneL || !doneR) {
    uint32_t width = micros() - usec;
    if (width >= (uint32_t)pL) { digitalWrite(MOTOR_PIN_L, LOW); doneL = true; }
    if (width >= (uint32_t)pR) { digitalWrite(MOTOR_PIN_R, LOW); doneR = true; }
  }
}

// サーボ停止（パルスを出さない = 完全停止）
void servo_stop() {
  digitalWrite(MOTOR_PIN_L, LOW);
  digitalWrite(MOTOR_PIN_R, LOW);
  powerL = MOTOR_NEUTRAL;
  powerR = MOTOR_NEUTRAL;
}

// ============================================================
//  IMU
// ============================================================
void readIMU() {
  float gx, gy, gz, ax, ay, az;
  M5.Imu.getGyro(&gx, &gy, &gz);
  M5.Imu.getAccel(&ax, &ay, &az);
  gyro[0] = gx; gyro[1] = gy; gyro[2] = gz;
  acc[0] = ax;  acc[1] = ay;  acc[2] = az;
  dAngle = gyro[0] - gyroOffset[0];
}

void calibrateIMU() {
  float gyroSum[3] = {0}, accSum[3] = {0};
  for (int i = 0; i < GYRO_CAL_SAMPLES; i++) {
    readIMU();
    gyroSum[0] += gyro[0]; gyroSum[1] += gyro[1]; gyroSum[2] += gyro[2];
    accSum[0] += acc[0];   accSum[1] += acc[1];   accSum[2] += acc[2];
    delay(2);
  }
  for (int i = 0; i < 3; i++) {
    gyroOffset[i] = gyroSum[i] / GYRO_CAL_SAMPLES;
    accOffset[i] = accSum[i] / GYRO_CAL_SAMPLES;
  }
  accOffset[2] -= 1.0;  // 重力補正
}

void applyCalibration() {
  for (int i = 0; i < 3; i++) {
    gyro[i] -= gyroOffset[i];
    acc[i] -= accOffset[i];
  }
}

float getPitch() {
  return atan2(acc[1], acc[2]) * RAD_TO_DEG;
}

// ============================================================
//  姿勢角度の取得
// ============================================================
void get_Angle() {
  readIMU();
  applyCalibration();
  float dt = (micros() - lastUs) / 1000000.0;
  lastUs = micros();
  Pitch = kalman.getAngle(getPitch(), gyro[0], dt);
  Pitch_filter = (Pitch + Pitch_filter * (FIL_N - 1)) / FIL_N;
  Angle = Pitch - Pitch_offset;
}

// ============================================================
//  PID制御
// ============================================================
void PID_reset() {
  Power = Speed = I_Angle = power = 0;
  wait_count = 0;
}

void PID_ctrl() {
  // シンプルP制御のみ（まず方向確認用）
  power = (int16_t)(kp * Angle);
  power = constrain(power, -500, 500);

  if (motor_sw == 1) {
    powerL =  power + motor_offsetL + MOTOR_NEUTRAL;
    powerR = -power + motor_offsetR + MOTOR_NEUTRAL;
    pulse_drive(powerL, powerR);
  } else {
    digitalWrite(MOTOR_PIN_L, LOW);
    digitalWrite(MOTOR_PIN_R, LOW);
  }
}

// ============================================================
//  ディスプレイ
// ============================================================
void updateDisplay() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  
  // 状態
  StickCP2.Display.setCursor(0, 0);
  if (motor_sw == 1) {
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.printf("ON ");
  } else {
    StickCP2.Display.setTextColor(RED);
    StickCP2.Display.printf("OFF");
  }
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.printf(" A:%5.1f", Angle);
  
  // 現在のパラメータ（選択中はハイライト）
  StickCP2.Display.setTextSize(1);
  for (int i = 0; i < 5; i++) {
    StickCP2.Display.setCursor(0, 25 + i * 12);
    if (i == tuneParam) {
      StickCP2.Display.setTextColor(YELLOW);
      StickCP2.Display.printf("> ");
    } else {
      StickCP2.Display.setTextColor(DARKGREY);
      StickCP2.Display.printf("  ");
    }
    StickCP2.Display.printf("%s=%5.2f", paramNames[i], *paramPtrs[i]);
  }
  
  // 操作説明
  StickCP2.Display.setCursor(0, 90);
  StickCP2.Display.setTextColor(CYAN);
  StickCP2.Display.printf("[A]ON/OFF [B]%s+%.1f", paramNames[tuneParam], paramStep);
  StickCP2.Display.setCursor(0, 102);
  StickCP2.Display.printf("[A long]Param [B long]Step");
  
  // バッテリー
  StickCP2.Display.setCursor(0, 118);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.printf("%.1fV  L:%4d R:%4d", batt, powerL, powerR);
}

// ============================================================
//  setup
// ============================================================
void setup() {
  auto cfg = M5.config();
  cfg.output_power = true;
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);
  Serial.begin(115200);

  // ボタン: GPIO直接読み（StickCP2.BtnA を使わない）
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);

  // モーターピン
  pinMode(MOTOR_PIN_L, OUTPUT);
  pinMode(MOTOR_PIN_R, OUTPUT);

  // IMU初期化
  M5.Imu.begin();
  
  // パラメータポインタ初期化
  paramPtrs[0] = &kp;
  paramPtrs[1] = &kd;
  paramPtrs[2] = &ki;
  paramPtrs[3] = &kspd;
  paramPtrs[4] = &kdst;

  // キャリブレーション画面
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(10, 20);
  StickCP2.Display.println("Calibrating...");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("Keep still!");

  calibrateIMU();

  // カルマンフィルタ初期化
  readIMU();
  kalman.setAngle(getPitch());
  lastUs = micros();
  
  // 初期角度を取得するために一度 get_Angle() を呼ぶ
  for (int i = 0; i < 100; i++) {
    get_Angle();
    delay(5);
  }
  // カルマンフィルタ + IMUキャリブレーション後は Pitch ≈ 0
  // Pitch_offset は不要（ONにした瞬間に再設定）
  Pitch_offset = 0;
  Serial.printf("Pitch=%.1f, Pitch_offset=%.1f\n", Pitch, Pitch_offset);

  // 画面初期化
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setCursor(0, 0);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.println("InvPendulum v2");

  servo_stop();

  ms10 = ms100 = ms1000 = millis();

  Serial.println("=== Inverted Pendulum Ready (pulse_drive + tuning) ===");
  Serial.println("CSV: DATA,time_ms,angle,dAngle,powerL,powerR,kp,kd,ki,kspd,kdst");
  Serial.println("[A short]ON/OFF [A long]Param [B short]+step [B long]Step size");
}

// ============================================================
//  loop
// ============================================================
void loop() {
  get_Angle();

  // 10ms制御ループ (100Hz)
  if (millis() > ms10) {
    if (motor_sw == 1) {
      if (-ANGLE_LIMIT < Angle && Angle < ANGLE_LIMIT) {
        wait_count++;
        if (wait_count > 50) {
          PID_ctrl();
        }
      } else {
        // 角度超過: PIDリセットだけ（モーターは止めない、自然に戻るのを待つ）
        PID_reset();
        servo_stop();
      }
    }
    ms10 += 10;
  }

  // 100ms 表示更新 + ボタン
  if (millis() > ms100) {
    updateDisplay();
    
    // シリアルCSVログ
    Serial.printf("DBG,A=%.1f,pw=%d,pL=%d,pR=%d,sw=%d\n",
      Angle, power, powerL, powerR, motor_sw);
    
    // BtnA短押し: モーターON/OFF、長押し: パラメータ切替
    static unsigned long btnA_down = 0;
    if (digitalRead(BTN_A) == 0) {
      if (btnA_down == 0) btnA_down = millis();
      if (millis() - btnA_down > 800) {
        // 長押し: パラメータ切替
        tuneParam = (tuneParam + 1) % 5;
        Serial.printf("PARAM: %s\n", paramNames[tuneParam]);
        btnA_down = 0;
        delay(300);
      }
    } else {
      if (btnA_down > 0 && millis() - btnA_down < 800) {
        // 短押し: モーターON/OFF
        motor_sw = !motor_sw;
        if (motor_sw == 1) {
          // ONにした瞬間の角度を基準にする
          Pitch_offset = Pitch_filter;
          PID_reset();
          Serial.printf("Motor: ON (offset=%.1f)\n", Pitch_offset);
        } else {
          PID_reset();
          servo_stop();
          Serial.println("Motor: OFF");
        }
      }
      btnA_down = 0;
    }
    
    // BtnB短押し: パラメータ+step、長押し: ステップ切替
    static unsigned long btnB_down = 0;
    if (digitalRead(BTN_B) == 0) {
      if (btnB_down == 0) btnB_down = millis();
      if (millis() - btnB_down > 800) {
        // 長押し: ステップ切替 (0.1 → 1.0 → 0.01 → 0.1)
        if (paramStep >= 1.0) paramStep = 0.01;
        else if (paramStep >= 0.1) paramStep = 1.0;
        else paramStep = 0.1;
        Serial.printf("STEP: %.2f\n", paramStep);
        btnB_down = 0;
        delay(300);
      }
    } else {
      if (btnB_down > 0 && millis() - btnB_down < 800) {
        // 短押し: パラメータ増加
        *paramPtrs[tuneParam] += paramStep;
        Serial.printf("SET %s=%.2f\n", paramNames[tuneParam], *paramPtrs[tuneParam]);
      }
      btnB_down = 0;
    }
    
    ms100 += 100;
  }

  // 1秒 バッテリー
  if (millis() > ms1000) {
    batt = M5.Power.getBatteryVoltage() / 1000.0;
    ms1000 += 1000;
  }
}
