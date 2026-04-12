/*
 * FS90R サーボモータ動作テスト for M5StickC Plus
 *
 * 配線:
 *   サーボ1: PWM(橙)→G0,  VCC(赤)→5V, GND(茶)→GND
 *   サーボ2: PWM(橙)→G26, VCC(赤)→5V, GND(茶)→GND
 *
 * 動作:
 *   起動時: 両サーボ停止（ニュートラル）
 *   Aボタン（M5ボタン）: 正転 → 逆転 → 停止 を順番に切り替え
 *   Bボタン（側面）:     両サーボ緊急停止
 */

#include <M5StickCPlus.h>
#include <ESP32Servo.h>

#define SERVO1_PIN 0
#define SERVO2_PIN 26

// FS90R: 90=停止, <90=逆転, >90=正転
#define NEUTRAL 90
#define FORWARD 120
#define REVERSE 60

Servo servo1;
Servo servo2;

int state = 0; // 0:停止, 1:正転, 2:逆転

void setServos(int val) {
  servo1.write(val);
  servo2.write(val);
}

void updateDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Servo Test");
  M5.Lcd.setCursor(10, 40);

  switch (state) {
    case 0:
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.println("STOP");
      break;
    case 1:
      M5.Lcd.setTextColor(GREEN);
      M5.Lcd.println("FORWARD");
      break;
    case 2:
      M5.Lcd.setTextColor(RED);
      M5.Lcd.println("REVERSE");
      break;
  }

  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 80);
  M5.Lcd.println("[A] Switch");
  M5.Lcd.setCursor(10, 110);
  M5.Lcd.println("[B] Stop");
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);

  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  setServos(NEUTRAL);

  updateDisplay();
  Serial.begin(115200);
  Serial.println("Servo test ready");
}

void loop() {
  M5.update();

  // Aボタン: 停止→正転→逆転→停止...
  if (M5.BtnA.wasPressed()) {
    state = (state + 1) % 3;
    switch (state) {
      case 0: setServos(NEUTRAL); break;
      case 1: setServos(FORWARD); break;
      case 2: setServos(REVERSE); break;
    }
    updateDisplay();
    Serial.printf("State: %d\n", state);
  }

  // Bボタン: 緊急停止
  if (M5.BtnB.wasPressed()) {
    state = 0;
    setServos(NEUTRAL);
    updateDisplay();
    Serial.println("Emergency stop");
  }

  delay(20);
}
