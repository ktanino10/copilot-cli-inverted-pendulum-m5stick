/*
 * 倒立振子 — n_shinichi氏準拠クリーン版
 * for M5StickC Plus2 + FS90R サーボ × 2
 *
 * n_shinichi氏のオリジナルコードに完全準拠。
 * 変更点は IMU軸（加速度Z軸、ジャイロX軸）と
 * モーター方向（+power/-power）のみ。
 *
 * シリアルコマンド対応（115200bps）:
 *   kp=6.3  kd=0.48  ki=1.4  po=1.0  on  off  ?
 */

#include <M5StickCPlus2.h>
#include <Kalman.h>

// ============================================================
//  ハードウェア設定
// ============================================================
#define MOTOR_PIN_L 0
#define MOTOR_PIN_R 26
#define BTN_A 37
#define BTN_B 39

// ============================================================
//  PIDパラメータ（n_shinichi氏デフォルト値）
// ============================================================
int motor_offsetL = 0, motor_offsetR = 0;
int16_t motor_init_L = 1500, motor_init_R = 1500;
// 速度・位置フィードバックを"極弱"で復活：純P+Dだけだと
// 車輪が振子の真下へ走り続けて止まらない（位置/速度ルーフ無し）。
// kpower で「power の積分(=車輪速度)」を生成し、kspd で抑える。
float kpower = 0.0001;
float kp = 10.0;
float ki = 0.0;
float kd = 0.35;
float kspd = 0.15;
float kdst = 0.0;
int16_t power_limit = 350;
float power_pos_scale = 0.85;  // +power = 前進補正
float power_neg_scale = 1.70;  // -power = 後退補正。後ろへ逃げるため強める
int16_t min_drive_power = 0;   // 振動源になるため既定では無効
float min_drive_angle = 3.0;   // 小刻み振動では最低出力を入れない
// PID有効角度範囲。広いと LIMIT 復帰時に大角度→飽和→限界サイクル振動の原因。
// ±20 に絞り、復帰時の出力を power_limit 内に抑える。
float angle_limit = 20.0;

// Pitch_offset: atan2 + この値で直立=0°
// n_shinichi氏は81を使用。Plus2での実測値に要調整。
float Pitch_offset = 0.0;  // 起動後にシリアルで設定
float Pitch_offset2 = 0.0;
float Pitch_power = 0.0;
float targetBias = 0.0;  // 正の値で目標姿勢を+側へずらす
int fil_N = 5;

// ============================================================
//  制御変数
// ============================================================
Kalman kalman;
long lastMs = 0;
float acc[3], accOffset[3];
float gyro[3], gyroOffset[3];
float Pitch, Pitch_filter, Angle;
float dAngle;
int wait_count;
unsigned char motor_sw = 0;
int16_t power, powerL, powerR;
unsigned long ms10, ms100, ms1000;
float Speed, P_Angle, I_Angle, D_Angle, k_speed;
float batt;

// ============================================================
//  IMU — n_shinichi氏と同じ構造
// ============================================================
void readGyro() {
  float gx, gy, gz, ax, ay, az;
  M5.Imu.getGyro(&gx, &gy, &gz);
  M5.Imu.getAccel(&ax, &ay, &az);
  gyro[0] = gx; gyro[1] = gy; gyro[2] = gz;
  acc[0] = ax;  acc[1] = ay;  acc[2] = az;
  // ジャイロX軸 = 前後の回転（実機テスト確認済み）
  dAngle = (gyro[0] - gyroOffset[0]);
}

void calibration() {
  float gyroSum[3] = {0}, accSum[3] = {0};
  for (int i = 0; i < 500; i++) {
    readGyro();
    for (int j = 0; j < 3; j++) {
      gyroSum[j] += gyro[j];
      accSum[j] += acc[j];
    }
    delay(2);
  }
  for (int j = 0; j < 3; j++) {
    gyroOffset[j] = gyroSum[j] / 500;
    accOffset[j] = accSum[j] / 500;
  }
  accOffset[2] -= 1.0;
}

void applyCalibration() {
  for (int i = 0; i < 3; i++) {
    gyro[i] -= gyroOffset[i];
    acc[i] -= accOffset[i];
  }
}

// acc[2]の偏差を線形にスケーリング（前後均等な応答）
float getPitch() {
  return (acc[2] - 1.0) * 57.3;
}

// ============================================================
//  角度取得 — n_shinichi氏と同じ構造
// ============================================================
void get_Angle() {
  readGyro();
  applyCalibration();
  float kalman_dt = (micros() - lastMs) / 1000000.0;
  lastMs = micros();
  // カルマンフィルタ: ジャイロX軸（n_shinichi氏と同じ）
  Pitch = kalman.getAngle(getPitch(), gyro[0], kalman_dt) + Pitch_offset2 + Pitch_power;
  Pitch_filter = (Pitch + Pitch_filter * (fil_N - 1)) / fil_N;
  Angle = Pitch_filter - Pitch_offset - targetBias;
}

// ============================================================
//  サーボ駆動 — n_shinichi氏のpulse_drive
// ============================================================
void pulse_drive(int16_t pL, int16_t pR) {
  pL = constrain(pL, 500, 2500);
  pR = constrain(pR, 500, 2500);
  bool doneL = false, doneR = false;
  uint32_t usec = micros();
  digitalWrite(MOTOR_PIN_L, HIGH);
  digitalWrite(MOTOR_PIN_R, HIGH);
  while (!doneL || !doneR) {
    uint32_t w = micros() - usec;
    if (w >= (uint32_t)pL) { digitalWrite(MOTOR_PIN_L, LOW); doneL = true; }
    if (w >= (uint32_t)pR) { digitalWrite(MOTOR_PIN_R, LOW); doneR = true; }
  }
}

void servo_stop() {
  digitalWrite(MOTOR_PIN_L, LOW);
  digitalWrite(MOTOR_PIN_R, LOW);
  powerL = motor_init_L;
  powerR = motor_init_R;
}

// ============================================================
//  PID制御 — n_shinichi氏と同じ構造（符号のみ変更）
// ============================================================
void PID_reset() {
  Pitch_power = Speed = I_Angle = P_Angle = D_Angle = k_speed = 0;
  wait_count = power = 0;
}

void setBalanceReference() {
  Pitch_offset = Pitch_filter;
}

void PID_ctrl() {
  Speed += kpower * power;
  P_Angle = -kp * Angle;
  I_Angle += -ki * Angle - kdst * Speed;
  D_Angle = -kd * dAngle;
  k_speed = -kspd * Speed;

  float scaledPower = P_Angle + I_Angle + D_Angle + k_speed;
  if (scaledPower > 0) {
    scaledPower *= power_pos_scale;
  } else {
    scaledPower *= power_neg_scale;
  }
  if (fabs(Angle) > min_drive_angle && fabs(scaledPower) > 1 && fabs(scaledPower) < min_drive_power) {
    scaledPower = scaledPower > 0 ? min_drive_power : -min_drive_power;
  }
  power = constrain((int16_t)scaledPower, -power_limit, power_limit);

  if (I_Angle > 300) { power = Speed = I_Angle = Pitch_power = 0; }
  if (I_Angle < -300) { power = Speed = I_Angle = Pitch_power = 0; }

  if (motor_sw == 1) {
    powerL =  power + motor_offsetL + motor_init_L;
    powerR = -power + motor_offsetR + motor_init_R;
    pulse_drive(powerL, powerR);
  } else {
    digitalWrite(MOTOR_PIN_L, LOW);
    digitalWrite(MOTOR_PIN_R, LOW);
  }
}

// ============================================================
//  シリアルコマンド
// ============================================================
void processSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd == "on") {
    PID_reset();
    setBalanceReference();
    motor_sw = 1;
    Serial.printf("Motor ON zero po=%.2f bias=%.2f\n", Pitch_offset, targetBias);
  }
  else if (cmd == "off") {
    motor_sw = 0; PID_reset(); servo_stop();
    Serial.println("Motor OFF");
  }
  else if (cmd == "zero") {
    PID_reset();
    setBalanceReference();
    Serial.printf("zero po=%.2f bias=%.2f\n", Pitch_offset, targetBias);
  }
  else if (cmd == "?") {
    Serial.printf("kp=%.2f ki=%.2f kd=%.2f kspd=%.2f kdst=%.2f kpower=%.4f po=%.2f po2=%.2f bias=%.2f plim=%d ppos=%.2f pneg=%.2f minp=%d minang=%.1f alim=%.1f\n",
      kp, ki, kd, kspd, kdst, kpower, Pitch_offset, Pitch_offset2, targetBias, power_limit, power_pos_scale, power_neg_scale, min_drive_power, min_drive_angle, angle_limit);
    Serial.printf("Angle=%.1f Pitch=%.1f PF=%.1f\n", Angle, Pitch, Pitch_filter);
    Serial.printf("acc=%.2f,%.2f,%.2f gyro=%.1f,%.1f,%.1f\n", acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2]);
  }
  else if (cmd.startsWith("kp=")) { kp = cmd.substring(3).toFloat(); Serial.printf("kp=%.2f\n", kp); }
  else if (cmd.startsWith("ki=")) { ki = cmd.substring(3).toFloat(); Serial.printf("ki=%.2f\n", ki); }
  else if (cmd.startsWith("kd=")) { kd = cmd.substring(3).toFloat(); Serial.printf("kd=%.2f\n", kd); }
  else if (cmd.startsWith("kpower=")) { kpower = cmd.substring(7).toFloat(); Serial.printf("kpower=%.4f\n", kpower); }
  else if (cmd.startsWith("kspd=")) { kspd = cmd.substring(5).toFloat(); Serial.printf("kspd=%.2f\n", kspd); }
  else if (cmd.startsWith("kdst=")) { kdst = cmd.substring(5).toFloat(); Serial.printf("kdst=%.2f\n", kdst); }
  else if (cmd.startsWith("po=")) { Pitch_offset = cmd.substring(3).toFloat(); Serial.printf("po=%.2f\n", Pitch_offset); }
  else if (cmd.startsWith("po2=")) { Pitch_offset2 = cmd.substring(4).toFloat(); Serial.printf("po2=%.2f\n", Pitch_offset2); }
  else if (cmd.startsWith("bias=")) { targetBias = cmd.substring(5).toFloat(); Serial.printf("bias=%.2f\n", targetBias); }
  else if (cmd.startsWith("plim=")) { power_limit = cmd.substring(5).toInt(); Serial.printf("plim=%d\n", power_limit); }
  else if (cmd.startsWith("ppos=")) { power_pos_scale = cmd.substring(5).toFloat(); Serial.printf("ppos=%.2f\n", power_pos_scale); }
  else if (cmd.startsWith("pneg=")) { power_neg_scale = cmd.substring(5).toFloat(); Serial.printf("pneg=%.2f\n", power_neg_scale); }
  else if (cmd.startsWith("minp=")) { min_drive_power = cmd.substring(5).toInt(); Serial.printf("minp=%d\n", min_drive_power); }
  else if (cmd.startsWith("minang=")) { min_drive_angle = cmd.substring(7).toFloat(); Serial.printf("minang=%.1f\n", min_drive_angle); }
  else if (cmd.startsWith("alim=")) { angle_limit = cmd.substring(5).toFloat(); Serial.printf("alim=%.1f\n", angle_limit); }
  else if (cmd.startsWith("oL=")) { motor_offsetL = cmd.substring(3).toInt(); Serial.printf("oL=%d\n", motor_offsetL); }
  else if (cmd.startsWith("oR=")) { motor_offsetR = cmd.substring(3).toInt(); Serial.printf("oR=%d\n", motor_offsetR); }
}

// ============================================================
//  ディスプレイ
// ============================================================
void updateDisplay() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setCursor(0, 0);
  if (motor_sw) {
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.printf("ON ");
  } else {
    StickCP2.Display.setTextColor(RED);
    StickCP2.Display.printf("OFF");
  }
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.printf(" %5.1f", Angle);

  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(0, 25);
  StickCP2.Display.printf("P=%.1f I=%.1f D=%.1f", kp, ki, kd);
  StickCP2.Display.setCursor(0, 37);
  StickCP2.Display.printf("pw=%d L=%d R=%d", power, powerL, powerR);
  StickCP2.Display.setCursor(0, 49);
  StickCP2.Display.printf("po=%.0f po2=%.1f %.1fV", Pitch_offset, Pitch_offset2, batt);
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

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(MOTOR_PIN_L, OUTPUT);
  pinMode(MOTOR_PIN_R, OUTPUT);
  M5.Imu.begin();

  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(10, 20);
  StickCP2.Display.println("Calibrating...");

  calibration();
  readGyro();
  kalman.setAngle(getPitch());
  lastMs = micros();

  servo_stop();
  ms10 = ms100 = ms1000 = millis();

  Serial.println("=== Inverted Pendulum v3 (n_shinichi aligned) ===");
  Serial.printf("kp=%.2f ki=%.2f kd=%.2f po=%.0f\n", kp, ki, kd, Pitch_offset);
  Serial.println("Commands: kp= kd= ki= po= po2= bias= zero on off ?");
}

// ============================================================
//  loop — n_shinichi氏と同じ構造
// ============================================================
void loop() {
  get_Angle();

  processSerial();

  // 10ms制御ループ（n_shinichi氏と同じ）
  if (millis() > ms10) {
    if (-angle_limit < Angle && Angle < angle_limit) {
      wait_count++;
      if (wait_count > 0) {
        PID_ctrl();
      }
    } else {
      PID_reset();
      servo_stop();
    }
    ms10 += 10;
  }

  // 100ms 表示 + ボタン + データログ
  if (millis() > ms100) {
    updateDisplay();
    if (motor_sw == 1) {
      Serial.printf("D,%.1f,%d,%d,%d\n", Angle, power, powerL, powerR);
      Serial.printf("X,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d,%d,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f\n",
        Angle, dAngle, P_Angle, I_Angle, D_Angle, k_speed, Speed,
        power, powerL, powerR, acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2]);
    }
    
    // BtnA: ON/OFF
    if (digitalRead(BTN_A) == 0) {
      motor_sw = !motor_sw;
      if (motor_sw == 1) {
        PID_reset();
        setBalanceReference();
      } else {
        PID_reset();
        servo_stop();
      }
      Serial.printf("Motor: %s\n", motor_sw ? "ON" : "OFF");
      delay(300);
    }
    // BtnB: po2 +0.5
    if (digitalRead(BTN_B) == 0) {
      Pitch_offset2 += 0.5;
      Serial.printf("po2=%.1f\n", Pitch_offset2);
      delay(300);
    }
    
    ms100 += 100;
  }

  // 1秒 バッテリー
  if (millis() > ms1000) {
    batt = M5.Power.getBatteryVoltage() / 1000.0;
    ms1000 += 1000;
  }
}
