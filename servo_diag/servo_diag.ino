/*
 * サーボ最終診断 — GPIO動作確認 + 手動PWM生成
 * LEDC/ライブラリを一切使わず、digitalWrite で直接PWM生成
 * + GPIO出力の読み戻し検証
 */

#include <M5StickCPlus.h>

#define TEST_PIN 0

void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  Serial.begin(115200);
  delay(500);

  // 電圧診断
  float vbus = M5.Axp.GetVBusVoltage();
  float vbat = M5.Axp.GetBatVoltage();
  float vaps = M5.Axp.GetAPSVoltage();
  Serial.println("=== Power Diagnostics ===");
  Serial.printf("VBUS: %.2fV\n", vbus);
  Serial.printf("BAT:  %.2fV\n", vbat);
  Serial.printf("APS:  %.2fV\n", vaps);

  // GPIO出力テスト
  pinMode(TEST_PIN, OUTPUT);

  Serial.println("\n=== GPIO Read-back Test ===");
  digitalWrite(TEST_PIN, HIGH);
  delay(10);
  int h = digitalRead(TEST_PIN);
  Serial.printf("G%d: wrote HIGH -> read %s\n", TEST_PIN, h ? "HIGH" : "LOW");

  digitalWrite(TEST_PIN, LOW);
  delay(10);
  int l = digitalRead(TEST_PIN);
  Serial.printf("G%d: wrote LOW  -> read %s\n", TEST_PIN, l ? "HIGH" : "LOW");

  bool gpioOK = (h == 1 && l == 0);
  Serial.printf("GPIO result: %s\n", gpioOK ? "OK" : "FAIL");

  // LCD表示
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.println("Final Diag");
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(5, 30);
  M5.Lcd.printf("VBUS:%.1fV BAT:%.1fV", vbus, vbat);
  M5.Lcd.setCursor(5, 55);
  M5.Lcd.printf("GPIO G%d: %s", TEST_PIN, gpioOK ? "OK" : "FAIL");
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.setCursor(5, 85);
  M5.Lcd.println("Manual PWM on G0");
  M5.Lcd.setCursor(5, 110);
  M5.Lcd.println("Servo should spin!");

  Serial.println("\n=== Manual PWM start (G0, 1000us) ===");
}

void loop() {
  // 手動PWM: 1000us パルス (全速回転) @ 50Hz
  digitalWrite(TEST_PIN, HIGH);
  delayMicroseconds(1000);
  digitalWrite(TEST_PIN, LOW);
  delayMicroseconds(19000);
}
