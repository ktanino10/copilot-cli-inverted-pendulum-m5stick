/*
 * FS90R ピン探索テスト for M5StickC Plus2
 *
 * 全ピン(G0, G26, G25, G36)を順番に試して
 * どのピンにサーボが繋がっているか確認するスケッチ
 *
 * Aボタン: 次のピンに切り替え
 * Bボタン: 現在のピンでサーボ正転/停止トグル
 */

#include <M5StickCPlus2.h>
#include <ESP32Servo.h>

const int pins[] = {0, 26, 25, 36, 32, 33};
const char* pinNames[] = {"G0", "G26", "G25", "G36", "G32", "G33"};
const int pinCount = 6;

int currentPin = 0;
bool running = false;
Servo servo;

void updateDisplay() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);

  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("Pin Finder");

  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 40);
  StickCP2.Display.printf("Pin: %s", pinNames[currentPin]);

  StickCP2.Display.setCursor(10, 65);
  if (running) {
    StickCP2.Display.setTextColor(GREEN);
    StickCP2.Display.println("RUNNING");
  } else {
    StickCP2.Display.setTextColor(RED);
    StickCP2.Display.println("STOPPED");
  }

  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 100);
  StickCP2.Display.println("[A] Next Pin");
  StickCP2.Display.setCursor(10, 125);
  StickCP2.Display.println("[B] Run/Stop");
}

void setup() {
  auto cfg = M5.config();
  cfg.output_power = true;  // 5V外部出力を有効化
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);
  Serial.begin(115200);
  Serial.println("=== Servo Pin Finder (Plus2) ===");

  servo.attach(pins[currentPin]);
  servo.write(90);
  updateDisplay();
  Serial.printf("Testing pin: %s\n", pinNames[currentPin]);
}

void loop() {
  StickCP2.update();

  // Aボタン: 次のピンへ
  if (StickCP2.BtnA.wasPressed()) {
    servo.write(90);
    servo.detach();
    running = false;

    currentPin = (currentPin + 1) % pinCount;
    servo.attach(pins[currentPin]);
    servo.write(90);

    updateDisplay();
    Serial.printf("Switched to pin: %s\n", pinNames[currentPin]);
  }

  // Bボタン: 回転/停止トグル
  if (StickCP2.BtnB.wasPressed()) {
    running = !running;
    if (running) {
      servo.write(130);
      Serial.printf("Pin %s: RUNNING\n", pinNames[currentPin]);
    } else {
      servo.write(90);
      Serial.printf("Pin %s: STOPPED\n", pinNames[currentPin]);
    }
    updateDisplay();
  }

  delay(20);
}
