/*
 * IMU ジャイロ軸確認ツール
 * 加速度とジャイロを全軸リアルタイム表示
 * 直立→前後に傾けて、どのジャイロ軸が変化するか確認
 */
#include <M5StickCPlus2.h>

void setup() {
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);
  M5.Imu.begin();
  Serial.begin(115200);
}

void loop() {
  float gx, gy, gz, ax, ay, az;
  M5.Imu.getGyro(&gx, &gy, &gz);
  M5.Imu.getAccel(&ax, &ay, &az);
  
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.setTextColor(YELLOW);
  StickCP2.Display.setCursor(5, 0);
  StickCP2.Display.println("Gyro Axis Check");
  
  // 加速度
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(5, 22);
  StickCP2.Display.printf("Acc  X:%+5.2f Y:%+5.2f Z:%+5.2f", ax, ay, az);
  
  // ジャイロ（大きく表示）
  StickCP2.Display.setTextSize(2);
  
  StickCP2.Display.setTextColor(abs(gx) > 10 ? GREEN : WHITE);
  StickCP2.Display.setCursor(5, 38);
  StickCP2.Display.printf("gX:%+6.0f", gx);
  
  StickCP2.Display.setTextColor(abs(gy) > 10 ? GREEN : WHITE);
  StickCP2.Display.setCursor(5, 58);
  StickCP2.Display.printf("gY:%+6.0f", gy);
  
  StickCP2.Display.setTextColor(abs(gz) > 10 ? GREEN : WHITE);
  StickCP2.Display.setCursor(5, 78);
  StickCP2.Display.printf("gZ:%+6.0f", gz);
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setTextColor(CYAN);
  StickCP2.Display.setCursor(5, 105);
  StickCP2.Display.print("Tilt FWD/BWD - which gyro?");
  StickCP2.Display.setCursor(5, 118);
  StickCP2.Display.print("Rotate L/R - which gyro?");
  
  delay(50);
}
