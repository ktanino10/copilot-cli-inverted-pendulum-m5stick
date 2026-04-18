/*
 * pulse_drive サーボテスト for M5StickC Plus2
 * Aボタン: G26サーボ ON/OFF
 * Bボタン: G0サーボ ON/OFF
 * pulse_drive方式（digitalWriteパルス）でサーボを駆動
 */
#include <M5StickCPlus2.h>

#define MOTOR_PIN_L  0
#define MOTOR_PIN_R  26
#define BTN_A        37
#define BTN_B        39

bool motorR_on = false;
bool motorL_on = false;

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

void updateDisplay() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(10, 5);
  StickCP2.Display.println("Pulse Test");
  
  StickCP2.Display.setCursor(10, 35);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.printf("G26: ");
  StickCP2.Display.setTextColor(motorR_on ? GREEN : RED);
  StickCP2.Display.println(motorR_on ? "ON (1800us)" : "OFF (1500us)");
  
  StickCP2.Display.setCursor(10, 60);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.printf("G0:  ");
  StickCP2.Display.setTextColor(motorL_on ? GREEN : RED);
  StickCP2.Display.println(motorL_on ? "ON (1800us)" : "OFF (1500us)");
  
  StickCP2.Display.setTextColor(CYAN);
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(10, 95);
  StickCP2.Display.println("[A] G26 toggle");
  StickCP2.Display.setCursor(10, 108);
  StickCP2.Display.println("[B] G0 toggle");
}

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
  
  updateDisplay();
  Serial.println("=== Pulse Drive Test ===");
}

void loop() {
  // パルス出力（ONのサーボだけ）
  if (motorL_on || motorR_on) {
    int16_t pL = motorL_on ? 1800 : 0;
    int16_t pR = motorR_on ? 1800 : 0;
    
    uint32_t usec = micros();
    if (motorL_on) digitalWrite(MOTOR_PIN_L, HIGH);
    if (motorR_on) digitalWrite(MOTOR_PIN_R, HIGH);
    
    bool doneL = !motorL_on, doneR = !motorR_on;
    while (!doneL || !doneR) {
      uint32_t w = micros() - usec;
      if (!doneL && w >= (uint32_t)pL) { digitalWrite(MOTOR_PIN_L, LOW); doneL = true; }
      if (!doneR && w >= (uint32_t)pR) { digitalWrite(MOTOR_PIN_R, LOW); doneR = true; }
    }
  }
  
  // ボタン
  if (digitalRead(BTN_A) == 0) {
    motorR_on = !motorR_on;
    updateDisplay();
    Serial.printf("G26: %s\n", motorR_on ? "ON" : "OFF");
    delay(300);
  }
  if (digitalRead(BTN_B) == 0) {
    motorL_on = !motorL_on;
    updateDisplay();
    Serial.printf("G0: %s\n", motorL_on ? "ON" : "OFF");
    delay(300);
  }
  
  delay(10);
}
