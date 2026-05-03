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
#include <WiFi.h>
#include <WebServer.h>
#include "mbedtls/base64.h"

// ─── WiFi network table ─────────────────────────────────────────────
// Defined here so wifi_config.h can reference IpsWifiNet when the
// optional IPS_WIFI_HAS_LIST form is used.
struct IpsWifiNet { const char* ssid; const char* pass; };

#include "wifi_config.h"

// Fallback to the legacy single-SSID form if the user didn't define a
// multi-SSID list. Either form works.
#ifndef IPS_WIFI_HAS_LIST
  static const IpsWifiNet IPS_WIFI_LIST[] = { {WIFI_SSID, WIFI_PASS} };
#endif
static const size_t IPS_WIFI_LIST_LEN = sizeof(IPS_WIFI_LIST) / sizeof(IPS_WIFI_LIST[0]);

// Soft-AP fallback identifiers — used when no known SSID is in range.
#ifndef IPS_AP_SSID
  #define IPS_AP_SSID "IPS-CTRL"
#endif
#ifndef IPS_AP_PASS
  #define IPS_AP_PASS "ips12345"
#endif

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
extern String wifi_ip;
extern String wifi_ssid_in_use;
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
//  コマンド処理（Serial と WebUI で共通使用）
// ============================================================
String processCommand(String cmd) {
  cmd.trim();
  char buf[160];
  if (cmd == "on") {
    PID_reset();
    setBalanceReference();
    motor_sw = 1;
    snprintf(buf, sizeof(buf), "Motor ON zero po=%.2f bias=%.2f", Pitch_offset, targetBias);
  }
  else if (cmd == "off") {
    motor_sw = 0; PID_reset(); servo_stop();
    snprintf(buf, sizeof(buf), "Motor OFF");
  }
  else if (cmd == "zero") {
    PID_reset();
    setBalanceReference();
    snprintf(buf, sizeof(buf), "zero po=%.2f bias=%.2f", Pitch_offset, targetBias);
  }
  else if (cmd == "?") {
    snprintf(buf, sizeof(buf), "kp=%.2f ki=%.2f kd=%.2f kspd=%.2f kdst=%.2f kpower=%.4f po=%.2f po2=%.2f bias=%.2f plim=%d ppos=%.2f pneg=%.2f minp=%d minang=%.1f alim=%.1f",
      kp, ki, kd, kspd, kdst, kpower, Pitch_offset, Pitch_offset2, targetBias, power_limit, power_pos_scale, power_neg_scale, min_drive_power, min_drive_angle, angle_limit);
  }
  else if (cmd.startsWith("kp=")) { kp = cmd.substring(3).toFloat(); snprintf(buf, sizeof(buf), "kp=%.2f", kp); }
  else if (cmd.startsWith("ki=")) { ki = cmd.substring(3).toFloat(); snprintf(buf, sizeof(buf), "ki=%.2f", ki); }
  else if (cmd.startsWith("kd=")) { kd = cmd.substring(3).toFloat(); snprintf(buf, sizeof(buf), "kd=%.2f", kd); }
  else if (cmd.startsWith("kpower=")) { kpower = cmd.substring(7).toFloat(); snprintf(buf, sizeof(buf), "kpower=%.4f", kpower); }
  else if (cmd.startsWith("kspd=")) { kspd = cmd.substring(5).toFloat(); snprintf(buf, sizeof(buf), "kspd=%.2f", kspd); }
  else if (cmd.startsWith("kdst=")) { kdst = cmd.substring(5).toFloat(); snprintf(buf, sizeof(buf), "kdst=%.2f", kdst); }
  else if (cmd.startsWith("po=")) { Pitch_offset = cmd.substring(3).toFloat(); snprintf(buf, sizeof(buf), "po=%.2f", Pitch_offset); }
  else if (cmd.startsWith("po2=")) { Pitch_offset2 = cmd.substring(4).toFloat(); snprintf(buf, sizeof(buf), "po2=%.2f", Pitch_offset2); }
  else if (cmd.startsWith("bias=")) { targetBias = cmd.substring(5).toFloat(); snprintf(buf, sizeof(buf), "bias=%.2f", targetBias); }
  else if (cmd.startsWith("plim=")) { power_limit = cmd.substring(5).toInt(); snprintf(buf, sizeof(buf), "plim=%d", power_limit); }
  else if (cmd.startsWith("ppos=")) { power_pos_scale = cmd.substring(5).toFloat(); snprintf(buf, sizeof(buf), "ppos=%.2f", power_pos_scale); }
  else if (cmd.startsWith("pneg=")) { power_neg_scale = cmd.substring(5).toFloat(); snprintf(buf, sizeof(buf), "pneg=%.2f", power_neg_scale); }
  else if (cmd.startsWith("minp=")) { min_drive_power = cmd.substring(5).toInt(); snprintf(buf, sizeof(buf), "minp=%d", min_drive_power); }
  else if (cmd.startsWith("minang=")) { min_drive_angle = cmd.substring(7).toFloat(); snprintf(buf, sizeof(buf), "minang=%.1f", min_drive_angle); }
  else if (cmd.startsWith("alim=")) { angle_limit = cmd.substring(5).toFloat(); snprintf(buf, sizeof(buf), "alim=%.1f", angle_limit); }
  else if (cmd.startsWith("oL=")) { motor_offsetL = cmd.substring(3).toInt(); snprintf(buf, sizeof(buf), "oL=%d", motor_offsetL); }
  else if (cmd.startsWith("oR=")) { motor_offsetR = cmd.substring(3).toInt(); snprintf(buf, sizeof(buf), "oR=%d", motor_offsetR); }
  else { snprintf(buf, sizeof(buf), "unknown: %s", cmd.c_str()); }
  return String(buf);
}

// ============================================================
//  シリアル入力ラッパー
// ============================================================
void processSerial() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  String resp = processCommand(cmd);
  Serial.println(resp);
}

// ============================================================
//  ディスプレイ
// ============================================================
// ─── Face-link state (M5 LCD ⟵ PC streamed Mona via /face POST) ───
// PC pushes ~5 FPS JPEG frames. While LINK is on, the LCD becomes a
// full-screen Mona display (no text). Disable LINK or stop the
// streamer (FACE_TIMEOUT_MS of silence) → LCD reverts to the normal
// status text view automatically. Fail-safe: never get stuck on a
// stale face if the network dies.
static const int   LCD_W            = 240;
static const int   LCD_H            = 135;
static const int   FACE_W           = 128;                            // full big Mona (LCD_H limit)
static const int   FACE_H           = 128;
static const int   FACE_X           = (LCD_W - FACE_W) / 2;           // horizontally centered
static const int   FACE_Y           = (LCD_H - FACE_H) / 2;           // vertically centered (≈3px top/bottom)
static const uint32_t FACE_TIMEOUT_MS = 5000;
static const size_t   FACE_BUF_MAX    = 16384;                        // 128² JPEG q70 ≈ 4-7KB; double for safety

static volatile bool g_face_active   = false;
static volatile bool g_face_changed  = false;
static uint32_t      g_face_last_rx_ms = 0;
static uint8_t       g_face_buf[FACE_BUF_MAX];
static size_t        g_face_len     = 0;
static volatile uint16_t g_face_bg565 = 0;   // RGB565 bg color (0 = BLACK; updated per /face?bg=)

// ─── Display task — runs on Core 0 to keep the 10ms control loop ────
// (which lives in Arduino's loop() on Core 1) free of LCD-rendering jitter.
// drawJpg() takes ~30-50ms on M5StickC Plus2's SPI bus; fillScreen takes
// ~13ms. Together they can starve the 10ms PID tick of 4-5 cycles, which
// shows up as control degradation (the user reported this after enabling
// LINK MONA). By moving all LCD work to a separate FreeRTOS task pinned
// to Core 0, the control loop on Core 1 is no longer touched by display
// rendering — only the cheap (<1ms) cmd-parse / WiFi-handle work remains
// in loop().
//
// Synchronization: g_face_buf may be written by handleFace() on Core 1
// while the display task tries to drawJpg() it on Core 0. We use a mutex:
//   - handleFace: try-take with 0 timeout. If display is mid-render, the
//     incoming HTTP frame is dropped (next will arrive in 100ms). This
//     keeps Core 1 / loop() / control completely non-blocking.
//   - displayTask: portMAX_DELAY take. Blocks on Core 0 only — safe.
static SemaphoreHandle_t g_face_buf_mutex = nullptr;
static TaskHandle_t      g_display_task   = nullptr;

// Convert "RRGGBB" hex to 16-bit 5-6-5. Returns 0 on parse error.
static uint16_t hexToRgb565(const String& hex) {
  if (hex.length() != 6) return 0;
  long v = strtol(hex.c_str(), nullptr, 16);
  uint8_t r = (v >> 16) & 0xFF;
  uint8_t g = (v >>  8) & 0xFF;
  uint8_t b =  v        & 0xFF;
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void updateDisplay() {
  // Auto-revert face mode if PC streamer stopped pushing frames.
  static bool prev_face_active = false;
  if (g_face_active && (millis() - g_face_last_rx_ms) > FACE_TIMEOUT_MS) {
    g_face_active = false;
  }
  bool mode_changed = (prev_face_active != g_face_active);
  prev_face_active  = g_face_active;

  // ─── Face-only mode: full-screen state-color takeover ───────────
  // The PC streamer fills the JPEG canvas with the state color (green/
  // amber/red/black) and tells us which color to paint the surrounding
  // LCD edges via /face?bg=RRGGBB so the entire 240×135 device looks
  // like one uniform colored panel with Mona + label centered on top.
  if (g_face_active) {
    if (g_face_changed || mode_changed) {
      // Hold the buffer mutex around the SPI render to keep handleFace()
      // (Core 1) from overwriting g_face_buf mid-drawJpg (Core 0). This
      // is a Core 0-side wait, so it never blocks the control loop.
      if (g_face_buf_mutex) xSemaphoreTake(g_face_buf_mutex, portMAX_DELAY);
      StickCP2.Display.fillScreen(g_face_bg565);
      StickCP2.Display.drawJpg(g_face_buf, g_face_len, FACE_X, FACE_Y, FACE_W, FACE_H);
      g_face_changed = false;
      if (g_face_buf_mutex) xSemaphoreGive(g_face_buf_mutex);
    }
    return;
  }

  // ─── Text-only mode: original status display ────────────────────
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
  if (wifi_ip.length() > 0) {
    StickCP2.Display.setCursor(0, 61);
    StickCP2.Display.setTextColor(CYAN);
    if (wifi_ssid_in_use.length() > 0) {
      // Show SSID (first 12 chars) + IP — vital when moving between locations.
      StickCP2.Display.printf("%s %s",
        wifi_ssid_in_use.substring(0, 12).c_str(), wifi_ip.c_str());
    } else {
      StickCP2.Display.printf("%s", wifi_ip.c_str());
    }
  }
}

// ─── displayTaskFn — Core 0 render loop ─────────────────────────────
// Wakes every 100ms, calls updateDisplay() (which does the heavy SPI
// rendering). Pinned to Core 0 so it cannot stall the 10ms PID control
// running on Core 1.
//
// Why this matters: before this task existed, updateDisplay was called
// from loop() on Core 1, which meant every 100ms the control thread was
// blocked for 30-50ms by drawJpg / fillScreen. That dropped 4-5 PID
// ticks every second when LINK MONA was on, visibly degrading balance.
static void displayTaskFn(void* /*arg*/) {
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    updateDisplay();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(100));
  }
}

// ============================================================
//  Web UI (WiFi + HTTP server)
// ============================================================
WebServer server(80);
String wifi_ip = "";
String wifi_ssid_in_use = "";

const char INDEX_HTML[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="ja"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Inverted Pendulum Tuning</title>
<style>
body{font-family:-apple-system,sans-serif;margin:0;padding:8px;background:#111;color:#eee;font-size:14px}
h2{margin:6px 0;font-size:16px;color:#9cf}
.row{display:flex;align-items:center;margin:4px 0;gap:6px}
.row label{flex:0 0 60px;font-weight:bold}
.row input[type=range]{flex:1;min-width:80px}
.row input[type=number]{width:70px;background:#222;color:#fff;border:1px solid #444;padding:3px;font-size:13px}
.row .step{flex:0 0 50px}
button{padding:8px 14px;margin:3px;border:0;border-radius:4px;font-size:14px;font-weight:bold;cursor:pointer}
.on{background:#0a0;color:#fff}.off{background:#a00;color:#fff}.zero{background:#08c;color:#fff}
#status{background:#000;border:1px solid #333;padding:6px;font-family:monospace;font-size:12px;white-space:pre;height:90px;overflow:auto}
.ang{font-size:36px;font-weight:bold;text-align:center;padding:4px;background:#000;border-radius:4px}
.ang.warn{color:#f80}.ang.err{color:#f00}.ang.ok{color:#0f0}
</style></head><body>
<h2>🤖 Inverted Pendulum</h2>
<div class="ang" id="ang">--</div>
<div style="text-align:center">
<button class="on" onclick="cmd('on')">ON</button>
<button class="off" onclick="cmd('off')">OFF</button>
<button class="zero" onclick="cmd('zero')">ZERO</button>
</div>
<h2>Gains</h2>
<div id="ctrls"></div>
<h2>Log</h2>
<div id="status"></div>
<script>
const PARAMS=[
 ['kp',  0, 30,  0.5],
 ['kd',  0,  3,  0.05],
 ['ki',  0,  5,  0.1],
 ['kspd',0, 1.0, 0.02],
 ['ppos',0.5,2.0,0.05],
 ['pneg',0.5,2.0,0.05],
 ['bias',-10,10, 0.5],
 ['alim',10,60, 1],
 ['plim',100,500,10],
 ['kpower',0,0.005,0.0001],
 ['minp', 0, 200, 5],
 ['po',  -30, 30, 0.5],
];
const ct=document.getElementById('ctrls');
const refs={};
PARAMS.forEach(([k,mn,mx,st])=>{
  const d=document.createElement('div'); d.className='row';
  d.innerHTML=`<label>${k}</label><input type="range" min="${mn}" max="${mx}" step="${st}" id="r_${k}"><input type="number" min="${mn}" max="${mx}" step="${st}" id="n_${k}">`;
  ct.appendChild(d);
  const r=d.querySelector('#r_'+k), n=d.querySelector('#n_'+k);
  refs[k]={r,n};
  const send=v=>{n.value=v;r.value=v;cmd(`${k}=${v}`)};
  r.oninput=e=>send(parseFloat(e.target.value));
  n.onchange=e=>send(parseFloat(e.target.value));
});
const log=document.getElementById('status');
function append(s){log.textContent=(s+"\n"+log.textContent).slice(0,2000)}
async function cmd(c){
  try{
    const r=await fetch('/c?q='+encodeURIComponent(c));
    const t=await r.text();
    append('> '+c+' → '+t);
    if(c==='?'||c==='on'||c==='off'||c==='zero')await refresh();
  }catch(e){append('ERR '+e)}
}
async function refresh(){
  try{
    const r=await fetch('/s'); const j=await r.json();
    const a=document.getElementById('ang');
    a.textContent=j.angle.toFixed(1)+'°';
    a.className='ang '+(Math.abs(j.angle)<5?'ok':Math.abs(j.angle)<15?'warn':'err');
    PARAMS.forEach(([k])=>{
      if(k in j){refs[k].r.value=j[k];refs[k].n.value=j[k];}
    });
  }catch(e){}
}
setInterval(refresh,250); refresh();
</script></body></html>
)HTML";

void handleRoot()    { server.send_P(200, "text/html", INDEX_HTML); }

void handleCmd() {
  String q = server.arg("q");
  String resp = processCommand(q);
  server.send(200, "text/plain", resp);
}

void handleStatus() {
  char j[600];
  snprintf(j, sizeof(j),
    "{\"angle\":%.2f,\"dangle\":%.2f,\"power\":%d,\"on\":%d,"
    "\"p_term\":%.2f,\"i_term\":%.2f,\"d_term\":%.2f,\"speed\":%.2f,"
    "\"kp\":%.2f,\"kd\":%.2f,\"ki\":%.2f,\"kspd\":%.3f,"
    "\"ppos\":%.2f,\"pneg\":%.2f,\"bias\":%.2f,\"alim\":%.1f,"
    "\"plim\":%d,\"kpower\":%.4f,\"minp\":%d,\"po\":%.2f,\"batt\":%.2f,"
    "\"face_link\":%d,\"ms\":%lu}",
    Angle, dAngle, power, motor_sw,
    P_Angle, I_Angle, D_Angle, Speed,
    kp, kd, ki, kspd,
    power_pos_scale, power_neg_scale, targetBias, angle_limit,
    power_limit, kpower, min_drive_power, Pitch_offset, batt,
    g_face_active ? 1 : 0, millis());
  server.send(200, "application/json", j);
}

// ─── /face: receive a JPEG frame (binary POST body) and stash it ───
// PC server.py streams ~5 FPS of the dashboard mascot here. We do NOT
// decode here (drawJpg in updateDisplay() does that on the next 100ms
// tick) — keeps this handler short so the control loop stays smooth.
// ─── /face: receive a base64-encoded JPEG (text body) and stash it ──
// PC server.py base64-encodes each JPEG so the body is 100% ASCII and
// survives ESP32 WebServer's String-based body parser (which truncates
// raw binary at the first NUL byte). We decode here and store the raw
// JPEG bytes for drawJpg() in updateDisplay() to render on the next tick.
void handleFace() {
  if (server.hasArg("bg")) {
    g_face_bg565 = hexToRgb565(server.arg("bg"));
  }
  if (server.hasArg("plain")) {
    const String& body = server.arg("plain");
    size_t enc_len = body.length();
    if (enc_len > 0) {
      // Try-take with 0 timeout: if displayTask (Core 0) is mid-render
      // (drawJpg ~30-50ms), drop this incoming frame and reply OK so the
      // sender keeps streaming. The next frame arrives in ~100ms, by
      // which time the render is finished. This keeps Core 1 / loop() /
      // PID control completely non-blocking — the whole point of having
      // moved display to Core 0 in the first place.
      if (g_face_buf_mutex && xSemaphoreTake(g_face_buf_mutex, 0) == pdTRUE) {
        size_t out_len = 0;
        int rc = mbedtls_base64_decode(g_face_buf, FACE_BUF_MAX, &out_len,
                                       (const unsigned char*)body.c_str(), enc_len);
        if (rc == 0 && out_len > 0) {
          g_face_len        = out_len;
          g_face_active     = true;
          g_face_last_rx_ms = millis();
          g_face_changed    = true;
        }
        xSemaphoreGive(g_face_buf_mutex);
        if (rc == 0 && out_len > 0) {
          server.send(200, "text/plain", "ok");
          return;
        }
      } else {
        // Display is rendering — frame intentionally dropped.
        server.send(200, "text/plain", "busy");
        return;
      }
    }
  }
  server.send(400, "text/plain", "bad");
}

// ─── WiFi: try every configured network, then fall back to soft-AP ──
// This makes the device portable: bring it to the office, the firmware
// will automatically try your phone hotspot SSID; if nothing is in
// range, it starts its own AP at http://192.168.4.1/ so you can still
// reach the UI from a laptop with zero infrastructure.
static bool tryConnect(const char* ssid, const char* pass, uint32_t timeout_ms) {
  Serial.printf("WiFi → trying \"%s\" ", ssid);
  WiFi.disconnect(true, true);
  delay(50);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeout_ms) {
    delay(200);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf(" OK (%lu ms)\n", millis() - t0);
    return true;
  }
  Serial.println(" timed out");
  return false;
}

static void startAccessPoint() {
  WiFi.disconnect(true, true);
  delay(50);
  WiFi.mode(WIFI_AP);
  bool ok = WiFi.softAP(IPS_AP_SSID, IPS_AP_PASS);
  if (ok) {
    wifi_ssid_in_use = String(IPS_AP_SSID) + " (AP)";
    wifi_ip = WiFi.softAPIP().toString();
    Serial.printf("WiFi: started soft-AP \"%s\" pass=\"%s\" → http://%s/\n",
                  IPS_AP_SSID, IPS_AP_PASS, wifi_ip.c_str());
  } else {
    wifi_ssid_in_use = "";
    wifi_ip = "AP FAIL";
    Serial.println("WiFi: soft-AP start FAILED");
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  // Scan first so we only attempt SSIDs that are actually in range —
  // avoids wasting the full timeout on networks that aren't there.
  Serial.println("WiFi: scanning...");
  int n = WiFi.scanNetworks(false, false, false, 200);
  Serial.printf("WiFi: %d networks visible\n", n);

  bool connected = false;
  for (size_t i = 0; i < IPS_WIFI_LIST_LEN && !connected; ++i) {
    const char* ssid = IPS_WIFI_LIST[i].ssid;
    const char* pass = IPS_WIFI_LIST[i].pass;
    bool inRange = (n <= 0);  // if scan failed/empty, just try blindly
    for (int j = 0; j < n; ++j) {
      if (WiFi.SSID(j) == ssid) { inRange = true; break; }
    }
    if (!inRange) {
      Serial.printf("WiFi → \"%s\" not visible, skipping\n", ssid);
      continue;
    }
    if (tryConnect(ssid, pass, 8000)) {
      wifi_ssid_in_use = ssid;
      wifi_ip = WiFi.localIP().toString();
      Serial.printf("WiFi OK: SSID=\"%s\"  http://%s/\n",
                    wifi_ssid_in_use.c_str(), wifi_ip.c_str());
      connected = true;
    }
  }

  if (!connected) {
    Serial.println("WiFi: no known network reachable — falling back to AP mode");
    startAccessPoint();
  }

  if (wifi_ip.length() > 0 && wifi_ip != "AP FAIL") {
    server.on("/", handleRoot);
    server.on("/c", handleCmd);
    server.on("/s", handleStatus);
    server.on("/face", HTTP_POST, handleFace);
    server.begin();
  }
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

  setupWiFi();

  // Create the LCD render task pinned to Core 0. This decouples display
  // (drawJpg / fillScreen, ~30-50ms each) from the 10ms PID control loop
  // running in loop() on Core 1. Without this, LINK MONA visibly degrades
  // balance because every 100ms drawJpg blocks ~5 PID ticks.
  g_face_buf_mutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
      displayTaskFn, "display", 6144, nullptr,
      1 /* low priority — yields to WiFi/system tasks */,
      &g_display_task,
      0 /* Core 0 (Arduino loop runs on Core 1) */);

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
  server.handleClient();

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
    // NOTE: updateDisplay() is no longer called here — it runs in
    // displayTaskFn on Core 0 (see setup()). Calling it here would
    // re-introduce the 30-50ms drawJpg blocking that degrades PID.
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
