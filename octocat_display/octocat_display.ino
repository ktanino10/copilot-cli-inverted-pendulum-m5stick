/*
 * GitHub Sticker Animated Display for M5StickC Plus2
 * 全キャラ90°反時計回り回転（倒立振子搭載時に正面を向く）
 *
 * Aボタン: 静止画切替 (Octocat/Copilot/Duck/Mascot)
 * Bボタン: 現在のキャラのGIFアニメ再生/停止
 *         (Octocatは静止画のみ)
 */

#include <M5StickCPlus2.h>

// 静止画 (110x110, rotated 90° CCW)
#include "octocat_img.h"
#include "copilot_img.h"
#include "duck_img.h"
#include "mascot_img.h"

// アニメーション (60x60, rotated 90° CCW)
#include "copilot_anim.h"
#include "duck_anim.h"
#include "mascot_anim.h"

#define IMG_W 110
#define IMG_H 110
#define BG    0x0861

// 静止画
const uint16_t* images[] = {octocat_img, copilot_img, duck_img, mascot_img};
const char* names[] = {"Octocat", "Copilot", "Duck", "Mascot"};
const int imgCount = 4;

// アニメーション情報
struct AnimInfo {
  const uint16_t* const* frames;
  int frameCount;
  int w, h;
};

const AnimInfo anims[] = {
  {NULL, 0, 0, 0},                                           // Octocat: no anim
  {copilot_anim, COPILOT_ANIM_FRAMES, COPILOT_ANIM_W, COPILOT_ANIM_H},
  {duck_anim, DUCK_ANIM_FRAMES, DUCK_ANIM_W, DUCK_ANIM_H},
  {mascot_anim, MASCOT_ANIM_FRAMES, MASCOT_ANIM_W, MASCOT_ANIM_H},
};

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
  StickCP2.Display.printf("[A]Next ");
  if (anims[idx].frames != NULL) {
    StickCP2.Display.printf("[B]Anim ");
  }
  StickCP2.Display.setTextColor(0xA11F);
  StickCP2.Display.printf("%s %d/%d", names[idx], idx+1, imgCount);
}

void showAnimFrame() {
  const AnimInfo& a = anims[current];
  int ox = (240 - a.w) / 2;
  int oy = (110 - a.h) / 2;
  const uint16_t* frame = (const uint16_t*)pgm_read_ptr(&a.frames[animFrame]);
  StickCP2.Display.pushImage(ox, oy, a.w, a.h, frame);
}

void startAnim() {
  if (anims[current].frames == NULL) return;
  animMode = true;
  animFrame = 0;
  lastFrameMs = millis();
  StickCP2.Display.fillScreen(BG);
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setTextColor(WHITE);
  StickCP2.Display.setCursor(5, 118);
  StickCP2.Display.printf("[B]Stop          ");
  StickCP2.Display.setTextColor(0xA11F);
  StickCP2.Display.printf("%s GIF", names[current]);
  
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
    if (millis() - lastFrameMs >= 60) {
      animFrame = (animFrame + 1) % anims[current].frameCount;
      showAnimFrame();
      lastFrameMs = millis();
    }
    if (digitalRead(39) == 0) {
      animMode = false;
      showImage(current);
      delay(300);
    }
  } else {
    if (digitalRead(37) == 0) {
      current = (current + 1) % imgCount;
      showImage(current);
      delay(300);
    }
    if (digitalRead(39) == 0) {
      startAnim();
      delay(300);
    }
  }
  delay(10);
}
