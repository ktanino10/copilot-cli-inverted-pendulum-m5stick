# 倒立振子 (Inverted Pendulum) — M5StickC Plus

## きっかけ

書店で [Interface誌 2025年9月号](https://interface.cqpub.co.jp/202509st/) を見かけて、倒立振子の特集記事が目に留まりました。PID制御でロボットが自力でバランスを取るという仕組みが面白そうで、実際に手を動かしてやってみたくなり、キットと部品を揃えて挑戦することにしました。

Arduino や電子工作は多少経験がありますが、制御工学は初めてです。倒立振子を通じて PID 制御やセンサフュージョンを学びながら、将来的には 3 軸姿勢制御やリアルタイム可視化にも発展させていきたいと考えています。

## ハードウェア

- **マイコン**: M5StickC Plus (ESP32, IMU: MPU6886)
- **サーボモータ**: FS90R (連続回転サーボ) × 2
- **タイヤ**: FS90R対応 × 2
- **ボディ**: Interface誌専用キット

## 配線

| RCサーボ1 | M5StickC Plus |
|-----------|---------------|
| PWM (橙)  | G0            |
| VCC (赤)  | 5V            |
| GND (茶)  | GND           |

| RCサーボ2 | M5StickC Plus |
|-----------|---------------|
| PWM (橙)  | G26           |
| VCC (赤)  | 5V            |
| GND (茶)  | GND           |

## 必要なライブラリ (Arduino IDE)

- [KalmanFilter](https://github.com/TKJElectronics/KalmanFilter)
- [RemoteXY](https://remotexy.com/en/help/)
- [M5StickCPlus](https://github.com/m5stack/M5StickC-Plus)
- ボードマネージャ: M5Stack

## 使い方

1. Arduino IDEでファームウェアを書き込む
2. M5StickCを水平に持ち、電源を入れ直す（キャリブレーション）
3. 「M5」ボタンを長押し → 倒立制御スタート

## PIDデフォルト値

```cpp
float kpower = 0.001;
float kp = 21.0;
float ki = 7.0;
float kd = 1.6;
float kdst = 0.07;
float kspd = 2.5;
```

## 進捗ログ

### 2026-04-13: 半田付け完了・サーボ動作確認

- キット基板へのサーボ半田付けを完了
- サーボ動作テスト (`servo_test.ino`) およびピン自動スキャン診断を実施
- 配線修正後、サーボの動作を確認 ✅
- 再半田付け中に M5StickC Plus が過熱・故障 → 基板ショートによる損傷の可能性大
- USB 接続するだけで発熱し画面表示なし、復旧不可と判断 → **M5StickC Plus 交換待ち**
- **次のステップ**: 新しい M5StickC Plus 入手後、半田付け前にジャンパーワイヤーでサーボ動作確認してからメインファームウェア (`inverted_pendulum.ino`) の書き込み＆倒立テスト

#### 購入が必要な部品

- M5StickC Plus（新品交換）
- デジタルテスター（通電前のショート確認用）

## 参考

- [Interface誌 記事ページ](https://interface.cqpub.co.jp/202509st/)
