/*
 * 方向テスト — PIDなし、固定出力で前進/後退
 * Aボタン: 4パターン切替
 * パターン1: L=1600 R=1600 (+100/+100)
 * パターン2: L=1400 R=1400 (-100/-100) 
 * パターン3: L=1600 R=1400 (+100/-100)
 * パターン4: L=1400 R=1600 (-100/+100)
 * Bボタン: 停止
 */
#include <M5StickCPlus2.h>

#define MOTOR_PIN_L  0
#define MOTOR_PIN_R  26
#define BTN_A        37
#define BTN_B        39

int mode = 0; // 0=stop
const char* modeNames[] = {"STOP", "+100/+100", "-100/-100", "+100/-100", "-100/+100"};
int16_t pLvals[] = {1500, 1600, 1400, 1600, 1400};
int16_t pRvals[] = {1500, 1600, 1400, 1400, 1600};

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

void showMode() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(10, 5);
  StickCP2.Display.println("Dir Test");
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.printf("Mode %d: %s", mode, modeNames[mode]);
  StickCP2.Display.setCursor(10, 55);
  StickCP2.Display.printf("L=%d R=%d", pLvals[mode], pRvals[mode]);
  StickCP2.Display.setTextColor(CYAN);
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(10, 85);
  StickCP2.Display.println("[A] Next pattern");
  StickCP2.Display.setCursor(10, 100);
  StickCP2.Display.println("[B] Stop");
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.setTextColor(GREEN);
  StickCP2.Display.println("Which one goes FORWARD?");
}

void setup() {
  auto cfg = M5.config();
  cfg.output_power = true;
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(MOTOR_PIN_L, OUTPUT);
  pinMode(MOTOR_PIN_R, OUTPUT);
  Serial.begin(115200);
  showMode();
}

void loop() {
  if (mode > 0) {
    pulse_drive(pLvals[mode], pRvals[mode]);
  }
  
  if (digitalRead(BTN_A) == 0) {
    mode = (mode + 1) % 5;
    showMode();
    Serial.printf("Mode %d: %s L=%d R=%d\n", mode, modeNames[mode], pLvals[mode], pRvals[mode]);
    delay(500);
  }
  if (digitalRead(BTN_B) == 0) {
    mode = 0;
    digitalWrite(MOTOR_PIN_L, LOW);
    digitalWrite(MOTOR_PIN_R, LOW);
    showMode();
    delay(300);
  }
  delay(10);
}
