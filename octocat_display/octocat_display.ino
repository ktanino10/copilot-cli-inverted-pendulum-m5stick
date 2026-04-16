/*
 * GitHub Sticker Animated Display for M5StickC Plus2
 * Aボタン: 静止画スライドショー (Octocat/Copilot/Duck/Mascot)
 * Bボタン: Copilot アニメーション再生/停止
 */

#include <M5StickCPlus2.h>
#include "octocat_img.h"
#include "copilot_img.h"
#include "duck_img.h"
#include "mascot_img.h"
#include "copilot_anim.h"

#define IMG_W 110
#define IMG_H 110
#define BG    0x0861

const uint16_t* images[] = {octocat_img, copilot_img, duck_img, mascot_img};
const char* names[] = {"Octocat", "Copilot", "Duck", "Mascot"};
const int imgCount = 4;
int current = 0;
bool animMode = false;
int animFrame = 0;
unsigned long lastFrameMs = 0;

void showImage(int idx) {
  StickCP2.Display.fillScreen(BG);
  int ox = (240 - IMG_W) / 2;
  StickCP2.Display.pushImage(ox, 0, IMG_W, IMG_H, images[idx]);
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(5, 118);
  StickCP2.Display.printf("[A]Next [B]Anim ");
  StickCP2.Display.setTextColor(0xA11F);
  StickCP2.Display.printf("%s %d/%d", names[idx], idx+1, imgCount);
}

void showAnimFrame() {
  int ox = (240 - ANIM_W) / 2;
  int oy = (110 - ANIM_H) / 2;
  
  const uint16_t* frame = (const uint16_t*)pgm_read_ptr(&anim_frames[animFrame]);
  StickCP2.Display.pushImage(ox, oy, ANIM_W, ANIM_H, frame);
}

void startAnim() {
  animMode = true;
  animFrame = 0;
  lastFrameMs = millis();
  StickCP2.Display.fillScreen(BG);
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(5, 118);
  StickCP2.Display.printf("[B] Stop         ");
  StickCP2.Display.setTextColor(0xA11F);
  StickCP2.Display.printf("Copilot GIF");
  
  showAnimFrame();
}

void setup() {
  auto cfg = M5.config();
  StickCP2.begin(cfg);
  StickCP2.Display.setRotation(3);
  pinMode(37, INPUT_PULLUP);
  pinMode(39, INPUT_PULLUP);
  showImage(0);
}

void loop() {
  StickCP2.update();
  
  if (animMode) {
    // アニメーション再生
    if (millis() - lastFrameMs >= 60) {  // ~16fps (2フレーム間引き×30ms)
      animFrame = (animFrame + 1) % ANIM_FRAMES;
      showAnimFrame();
      lastFrameMs = millis();
    }
    
    // Bボタン: アニメ停止
    if (digitalRead(39) == 0) {
      animMode = false;
      showImage(current);
      delay(300);
    }
  } else {
    // 静止画モード
    // Aボタン: 次の画像
    if (digitalRead(37) == 0) {
      current = (current + 1) % imgCount;
      showImage(current);
      delay(300);
    }
    
    // Bボタン: アニメーション開始
    if (digitalRead(39) == 0) {
      startAnim();
      delay(300);
    }
  }
  
  delay(10);
}
