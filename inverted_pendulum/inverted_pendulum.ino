/*
 * 倒立振子 — シリアルチューニング対応版
 * for M5StickC Plus2 + FS90R サーボ × 2
 *
 * PCからシリアルコマンドでPIDパラメータをリアルタイム変更可能。
 * USBケーブルを繋いだまま倒立テスト。
 *
 * シリアルコマンド（115200bps）:
 *   kp=30.0    — kp変更
 *   kd=2.0     — kd変更
 *   ki=3.0     — ki変更
 *   po=1.5     — Pitch_offset2変更
 *   on         — モーターON
 *   off        — モーターOFF
 *   ?          — 現在のパラメータ表示
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
//  PIDパラメータ
// ============================================================
int motor_offsetL = 0, motor_offsetR = 0;
int16_t motor_init_L = 1500, motor_init_R = 1500;
float kpower = 0.003;
float kp = 50.0;
float ki = 2.0;
float kd = 10.0;
float kspd = 5.0;
float kdst = 0.14;
float Pitch_offset2 = 0.0;
float Pitch_power = 0.0;
int fil_N = 5;

// ============================================================
//  制御変数
// ============================================================
Kalman kalman;
long lastMs = 0;
float acc[3], accOffset[3];
float gyro[3], gyroOffset[3];
float Pitch, Pitch_filter, Angle;
float Pitch_offset = 0;  // ONにした瞬間の直立角度
float dAngle;
int wait_count;
unsigned char motor_sw = 0;
int16_t power, powerL, powerR;
unsigned long ms10, ms100, ms1000;
float Speed, P_Angle, I_Angle, D_Angle, k_speed;
float batt;

// ============================================================
//  IMU
// ============================================================
void readGyro() {
  float gx, gy, gz, ax, ay, az;
  M5.Imu.getGyro(&gx, &gy, &gz);
  M5.Imu.getAccel(&ax, &ay, &az);
  gyro[0] = gx; gyro[1] = gy; gyro[2] = gz;
  acc[0] = ax;  acc[1] = ay;  acc[2] = az;
  dAngle = (gyro[2] - gyroOffset[2]);
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

float getPitch() {
  float val = constrain(acc[2], -1.0, 1.0);
  return asin(val) * RAD_TO_DEG;
}

void get_Angle() {
  readGyro();
  applyCalibration();
  float dt = (micros() - lastMs) / 1000000.0;
  lastMs = micros();
  Pitch = kalman.getAngle(getPitch(), gyro[2], dt) + Pitch_offset2 + Pitch_power;
  Pitch_filter = (Pitch + Pitch_filter * (fil_N - 1)) / fil_N;
  Angle = Pitch_filter - Pitch_offset;
}

// ============================================================
//  サーボ駆動
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
}

// ============================================================
//  PID制御
// ============================================================
void PID_reset() {
  Pitch_power = wait_count = power = Speed = I_Angle = 0;
}

void PID_ctrl() {
  Speed += kpower * power;
  P_Angle = -kp * Angle;
  I_Angle += -ki * Angle - kdst * Speed;
  I_Angle = constrain(I_Angle, -200, 200);  // I項を厳しく制限
  D_Angle = -kd * dAngle;
  k_speed = -kspd * Speed;

  power = P_Angle + I_Angle + D_Angle + k_speed;
  power = constrain(power, -400, 400);  // 出力を±400μsに制限

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
//  シリアルコマンド処理
// ============================================================
void processSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd == "on") {
    motor_sw = 1; 
    Pitch_offset = Pitch_filter;  // 今の角度を直立基準に
    PID_reset();
    Serial.printf("Motor ON (offset=%.1f)\n", Pitch_offset);
  }
  else if (cmd == "off") {
    motor_sw = 0; PID_reset(); servo_stop();
    Serial.println("Motor OFF");
  }
  else if (cmd == "?") {
    Serial.printf("kp=%.2f ki=%.2f kd=%.2f kspd=%.2f kdst=%.2f po=%.2f\n",
      kp, ki, kd, kspd, kdst, Pitch_offset2);
  }
  else if (cmd.startsWith("kp=")) { kp = cmd.substring(3).toFloat(); Serial.printf("kp=%.2f\n", kp); }
  else if (cmd.startsWith("ki=")) { ki = cmd.substring(3).toFloat(); Serial.printf("ki=%.2f\n", ki); }
  else if (cmd.startsWith("kd=")) { kd = cmd.substring(3).toFloat(); Serial.printf("kd=%.2f\n", kd); }
  else if (cmd.startsWith("kspd=")) { kspd = cmd.substring(5).toFloat(); Serial.printf("kspd=%.2f\n", kspd); }
  else if (cmd.startsWith("kdst=")) { kdst = cmd.substring(5).toFloat(); Serial.printf("kdst=%.2f\n", kdst); }
  else if (cmd.startsWith("po=")) { Pitch_offset2 = cmd.substring(3).toFloat(); Serial.printf("po=%.2f\n", Pitch_offset2); }
  else if (cmd.startsWith("oL=")) { motor_offsetL = cmd.substring(3).toInt(); Serial.printf("oL=%d\n", motor_offsetL); }
  else if (cmd.startsWith("oR=")) { motor_offsetR = cmd.substring(3).toInt(); Serial.printf("oR=%d\n", motor_offsetR); }
  else { Serial.println("Commands: kp= ki= kd= kspd= kdst= po= oL= oR= on off ?"); }
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
  StickCP2.Display.printf(" A:%5.1f", Angle);

  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(0, 25);
  StickCP2.Display.printf("kp=%.1f ki=%.1f kd=%.1f", kp, ki, kd);
  StickCP2.Display.setCursor(0, 37);
  StickCP2.Display.printf("spd=%.1f dst=%.2f po=%.1f", kspd, kdst, Pitch_offset2);
  StickCP2.Display.setCursor(0, 49);
  StickCP2.Display.printf("L=%d R=%d pw=%d", powerL, powerR, power);

  StickCP2.Display.setCursor(0, 65);
  StickCP2.Display.setTextColor(CYAN);
  StickCP2.Display.printf("[A]ON/OFF [B]po+0.5");
  StickCP2.Display.setCursor(0, 77);
  StickCP2.Display.printf("Serial: kp=30 kd=2 on off ?");
  
  StickCP2.Display.setCursor(0, 95);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.printf("%.1fV", batt);
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
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.println("Keep still!");

  calibration();

  readGyro();
  kalman.setAngle(getPitch());
  lastMs = micros();

  // 初期角度収束
  for (int i = 0; i < 100; i++) { get_Angle(); delay(5); }

  servo_stop();
  ms10 = ms100 = ms1000 = millis();

  Serial.println("=== Inverted Pendulum — Serial Tuning ===");
  Serial.println("Commands: kp=30 ki=3 kd=2 kspd=5 kdst=0.14 po=0 on off ?");
  Serial.printf("Current: kp=%.2f ki=%.2f kd=%.2f kspd=%.2f kdst=%.2f\n", kp, ki, kd, kspd, kdst);
}

// ============================================================
//  loop
// ============================================================
void loop() {
  get_Angle();

  // シリアルコマンド受信
  processSerial();

  // 10ms制御ループ
  if (millis() > ms10) {
    if (motor_sw == 1) {
      if (-45 < Angle && Angle < 45) {
        wait_count++;
        if (wait_count > 50) {
          PID_ctrl();
        }
      } else {
        PID_reset();
        servo_stop();
      }
    }
    ms10 += 10;
  }

  // 100ms 表示 + ボタン + データログ
  if (millis() > ms100) {
    updateDisplay();

    // データログ
    if (motor_sw == 1) {
      Serial.printf("D,%.1f,%d,%d,%d\n", Angle, power, powerL, powerR);
    }

    // BtnA: ON/OFF
    if (digitalRead(BTN_A) == 0) {
      motor_sw = !motor_sw;
      if (motor_sw == 0) { PID_reset(); servo_stop(); }
      else { 
        Pitch_offset = Pitch_filter;  // 今の角度を直立基準にする
        PID_reset(); 
      }
      Serial.printf("Motor: %s (offset=%.1f)\n", motor_sw ? "ON" : "OFF", Pitch_offset);
      delay(300);
    }

    // BtnB: Pitch_offset2 +0.5
    if (digitalRead(BTN_B) == 0) {
      Pitch_offset2 += 0.5;
      Serial.printf("po=%.1f\n", Pitch_offset2);
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
