# 📡 通信技術ガイド — WiFiで倒立振子を動かすまで

ブラウザに表示されているUIは、どうやって**M5StickC Plus2の中の制御ループ**まで届いているのか？  この資料は、その「通信」部分だけを切り出して説明する。

> 想定読者を分けて、**前半は初心者向け**（電子工作はやるけど通信は触ったことがない人）、**後半は技術者向け**（ネットワーク・組込みの素養がある人）に書く。同じ話を2回、解像度を変えて読めるようにしてある。

- 関連: [README §🌍 持ち運び・遠隔運用](../README.md#-持ち運び遠隔運用--どこででもすぐ起動する方法) / [pid_theory.md](pid_theory.md) / [glossary.md](glossary.md)

---

## Part 1 — 初心者向け：そもそも何が起きている？

### 1-1. 登場人物は3つだけ

| 登場人物 | 役割 | このプロジェクトでの正体 |
|---|---|---|
| **ブラウザ** | あなたが見ているチューニング画面 | Chrome / Safari / iPhoneのブラウザ |
| **PC上のサーバ** | ブラウザと振子の間の「通訳・案内人」 | `tools/server.py`（Pythonの `Flask`） |
| **振子（M5StickC）** | 実際にモーターを回している本体 | M5StickC Plus2（ESP32マイコン） |

矢印で書くと、こうなる。

```
[ブラウザ] ──①──▶ [PCのFlaskサーバ] ──②──▶ [M5StickC] ──モーター──▶ 倒立振子
   │                  │                    │
   │◀────③────────────│◀────④──────────────│
```

①〜④はいずれも **WiFiの上のHTTP通信**。WiFiという「電波の郵便制度」の上を、HTTPという「手紙の書式」で行き来している。

### 1-2. WiFiって何？

WiFi は **「家の中だけで使える、電波の郵便配達ネットワーク」** だと思えばいい。

- **ルータ**（家のWiFi機器）が郵便局の役目
- 機器それぞれに **「IPアドレス」** という住所が配られる（例: `192.168.10.32`）
- ブラウザもM5StickCも、ルータに「私はここです」と登録すると、**お互いに手紙（パケット）を出せるようになる**

> 📦 要するに：**WiFiにつながった瞬間、その機器は家の中の住所を一つもらう**。住所さえあれば、お互いが話しかけられる。

### 1-3. M5StickCがWiFiに繋がる流れ（家にいるとき）

1. 電源ON
2. ファームウェアが「家のSSID（WiFi名）と合言葉（パスワード）」をルータに送る
3. ルータが「OK、君の住所は `192.168.10.32` ね」と教えてくれる
4. M5の画面下に **その住所が表示される** ← これが目印

```
┌─────────────────────────┐
│ ON   2.3°               │
│ P=50 I=3 D=8            │
│ pw=42 L=1500 R=1500     │
│ home_2g 192.168.10.32   │ ◀ SSID と IP
└─────────────────────────┘
```

### 1-4. ブラウザからM5までの「往復」

- ブラウザは `http://192.168.10.32/s` という**手紙**を出す  
  （「いまの角度とモーター値を教えて」というお願い）
- M5は `{"angle": 2.3, "power": 42, ...}` という**返事**を返す
- これを **50ミリ秒に1回**繰り返している（人間の目では「リアルタイム」に見える速さ）

> 🐢 → 🐇 ：50ms = 0.05秒。1秒間に20回ペースで会話している。

### 1-5. 場所を変えたとき何が起きる？

ここがこのプロジェクトの肝。

#### (a) オフィスに持って行ったら、家のWiFiは見えなくなる
→ M5は「家のSSIDが見つからない」と気づく。

#### (b) 解決策その1 — **複数のWiFiを覚えさせておく**
家のWiFi、スマホのテザリング、オフィスのWiFi…全部 `wifi_config.h` に書いておく。  
M5は起動時に**周りに見えるWiFiをスキャン**して、知ってるやつに自動接続する。

#### (c) 解決策その2 — **M5自身がWiFi親機になる（APモード）**
誰の知ってるWiFiもなければ、M5が **「IPS-CTRL」というWiFiを自分で立ち上げる**。  
ノートPCで「IPS-CTRL」に繋いで `http://192.168.4.1/` を開けば、それで動く。  
**ルータも家のWiFiも一切要らない**。電車の中でも動く。

> 💡 たとえ：普段は「家の郵便制度」を使ってるけど、郵便制度がない場所に行ったら、**M5が自分で近所限定の郵便ポストになる**イメージ。

### 1-6. 遠くから操作したいとき（家のM5を会社から動かす）

- 家のPC側で `tools/server.py` を起動しっぱなしにする
- 家のPCと会社のPCを **Tailscale** という「秘密のトンネル」で繋ぐ（無料アプリ）
- 会社のブラウザから `http://<家のPCのトンネル住所>:5000/` を開く
- すると会社→家のPC→M5、と中継されてモーターが動く

> 🚇 たとえ：物理的に離れていても、**インターネットの中に2つのPC専用の地下トンネル**を掘ってあげる感じ。

#### Tailscale の使い方（超ざっくり3ステップ）

1. **家のPC** に Tailscale をインストール → ログイン → 自動で `100.x.y.z` の専用住所がもらえる（例: `100.96.12.34`）
2. **持ち歩く端末**（スマホ・ノートPC）にも同じアカウントで Tailscale を入れる
3. 出先で `tools/server.py` を家のPCで動かしたまま、ブラウザで `http://100.96.12.34:5000/` にアクセスする → 家にいるかのように M5 を操作できる

> 💡 重要：**M5StickC本体には Tailscale を入れません**（ESP32 はリソース不足）。必ず「家のPC＋Tailscale」を中継地点として挟みます。

### 1-7. GitHub Pagesデモは何が違う？

`https://<owner>.github.io/inverted-pendulum-m5stick/` は **ハードがなくても触れる**シミュレーション版。

- 本物のM5には繋がらない
- ブラウザ内の JavaScript が「振子のフリ」をしている
- パラメータを動かすと「シミュレートされた角度」が反応する
- 過去のテストデータを5本同梱しているので、解析画面も見れる

> 🎮 ：取扱説明書の代わりに「動くショールーム」を置いた感じ。

### 1-8. LIVE モード（自分だけの「裏口」）

実は GitHub Pages のデモには**隠しスイッチ**が仕込んであります。

- ページ右下の隅っこを **3回連続でタップ**（または click）
- パスワードを聞かれる → 設定者本人だけが知っている合言葉を入力
- 通れば「Backend URL」を聞かれる → 自分の家のPC（Tailscale 住所）を入れる
- これで **同じデモ画面のままで本物の M5 が動く**ようになる

しくみ的なポイント：

- パスワードそのものはコードに入っていません。**SHA-256 ハッシュ**だけ埋め込まれています（暗号学的「指紋」のような片道変換）
- 万一ハッシュが漏れても、Backend URL（Tailscale 住所）も知らないと M5 に届きません → 二重の鍵
- 解除ボタンでデモに戻せます

> 🔐 例えるなら：**ショールームの隅にある「関係者専用」ドア**。鍵を持ってる人にしか開かないし、開いた先がどこに繋がってるかも本人しか知らない。


---

## Part 2 — 技術者向け：プロトコルと実装

### 2-1. レイヤ構成

このプロジェクトで実際に動いている通信スタック：

```
 アプリ層        : HTTP/1.1（テキスト）+ JSON
 セッション層    : なし（ステートレス）
 トランスポート層: TCP（ESP32 WebServer は TCPベース）
 ネットワーク層  : IPv4（IPv6は未使用）
 リンク層        : IEEE 802.11 b/g/n（2.4GHz） — ESP32は2.4GHzのみ対応、5GHz不可
 物理層          : 2.4GHz ISM帯
```

ポーリング周期は 50 ms（20 Hz）。WebSocketも検討候補だが、ペイロードが小さく往復1ms以下のLAN環境ではHTTPで十分。サーバ実装も `WebServer.send()` 一行で済むため複雑度が低い。

### 2-2. ESP32側のサーバ実装（`inverted_pendulum.ino`）

```cpp
WebServer server(80);

server.on("/s", handleStatus);   // GET → JSON status
server.on("/c", handleCmd);      // GET ?q=k=v → mutate state
server.on("/", handleRoot);
server.begin();

// メインループで
server.handleClient();           // ノンブロッキングで受け付け
```

- `WebServer` は同期APIだが、`handleClient()` 1回あたり1リクエストしか処理しない設計のため**制御ループのジッタを抑えやすい**
- 本プロジェクトの制御ループは100Hz（10ms）。`handleClient()` は1ループ1回呼ぶことで、HTTPによる占有を最大1リクエスト分に抑制
- ペイロードは `snprintf` で組み立てた最小JSON（~300 bytes）。`ArduinoJson` 等のライブラリは使わない（ヒープフラグメント回避）

### 2-3. WiFiモード（STA / AP / STA+AP）

| モード | API | 役割 | 制約 |
|---|---|---|---|
| STA | `WiFi.mode(WIFI_STA); WiFi.begin(ssid, pass);` | 既存APに参加。LAN内の他機器へアクセス可、インターネットも可 | APの電波・認証を満たす必要あり |
| AP | `WiFi.mode(WIFI_AP); WiFi.softAP(ssid, pass);` | 自身がAPになる。デフォルトIP `192.168.4.1` | パスワードは8文字以上必須（WPA2-PSK制約）。インターネット非接続 |
| STA+AP | `WiFi.mode(WIFI_AP_STA);` | 両方同時。リレー的な使い方 | 帯域共有、消費電力増 |

このプロジェクトは「STA優先 → 全SSID失敗時にAPへフォールバック」。STA+APは未使用だが、将来的に「家ではSTAで運用しつつ、APで近接デバッグもしたい」場合の選択肢。

### 2-4. マルチSSID + スキャンの根拠

`WiFi.begin()` は接続失敗時に内部リトライを行うため、デフォルトで **5〜8秒ブロック**する。N個のSSIDを順試行すると最悪 N×8s 待たされる。

`WiFi.scanNetworks()`（passive scan, ~200ms）で**現に見えているSSIDだけを試す**ことで、最良ケース（家にいる）で1試行 ≤ 1秒、最悪ケース（全滅）でもAPフォールバックまで2秒程度に圧縮できる。

```cpp
int n = WiFi.scanNetworks();                       // 同期スキャン
for (auto& net : IPS_WIFI_LIST) {
  bool inRange = false;
  for (int j = 0; j < n; ++j)
    if (WiFi.SSID(j) == net.ssid) { inRange = true; break; }
  if (!inRange) continue;                          // 圏外スキップ
  if (tryConnect(net.ssid, net.pass, 8000)) {      // 8秒タイムアウト
    /* connected */ break;
  }
}
```

### 2-5. ソフトAPフォールバックの実装ポイント

- `WiFi.softAP(ssid, pass)` の戻り値が `bool`。**戻り値を必ずチェック**（パスワード長不足など黙って失敗する）
- DHCPサーバはESP32が内蔵で起動。`192.168.4.2`〜にクライアントを払い出す
- 同時接続クライアント数は ESP32 の場合 **最大4** がデフォルト。`WiFi.softAPConfig()` でIP帯変更可
- AP起動後も `server.begin()` を呼べばそのまま `192.168.4.1:80` でWebサーバが動く（実装上STAと同等）

### 2-6. Flaskプロキシを挟む理由

直接ブラウザ → M5でも動かせるが、以下の理由でプロキシを挟む：

| 理由 | 詳細 |
|---|---|
| **CORS回避** | 静的ファイルとAPIが**同一オリジン**になり、CORSプリフライト不要 |
| **IP抽象化** | フロントエンドは常に `/api/s` を見れば良く、M5のIP変動は `M5_URL` 環境変数だけで吸収 |
| **セッション保存** | 解析用データの永続化（CSV/JSON）はM5のフラッシュではなくホストPC側で実施 |
| **将来の拡張点** | 認証・キャッシュ・複数デバイス集約・Tailscale越え等、M5側を変えずに足せる |
| **mDNS非依存** | `<host>.local` 解決の不安定なクライアントを避け、IP直指定に統一 |

実装は薄い：

```python
@app.route("/api/s")
def api_status():
    r = requests.get(f"{M5_BASE}/s", timeout=0.4)
    return (r.text, r.status_code, {"Content-Type": "application/json"})
```

### 2-7. ポーリング vs WebSocket vs SSE

| 方式 | レイテンシ | 実装複雑度 | このプロジェクトでの選定 |
|---|---|---|---|
| **HTTP polling 50ms** | 〜50ms | 最低 | ✅ 採用 |
| Server-Sent Events | 〜10ms | 中 | M5側のサーバ実装が太る |
| WebSocket | <10ms | 高 | ESP32 RAMコストが大きい、双方向は不要 |

ペイロードが300バイト程度・人間が見るUIである・LAN内RTT < 5ms、という条件下ではHTTPポーリングで体感差ゼロ。デバッグも `curl` 一発でできる利点が大きい。

### 2-8. Tailscale による遠隔アクセス

- WireGuard ベースのメッシュVPN
- **NAT越え不要**（DERPリレー経由でも常時繋がる）
- 各デバイスに `100.x.y.z` の固定IPが配布される
- 自宅PCで `python3 tools/server.py` 常駐 → オフィスPCから `http://100.x.y.z:5000/` で透過アクセス
- 経路：`Office Browser → Tailnet → Home PC → home WiFi → M5`
- 暗号化されるのは Office ↔ Home PC 間だけ。**家のLAN内（Home PC ↔ M5）は素のHTTP**なので、家のWiFi自体のセキュリティ（WPA2/3）に依存

#### セットアップ手順

| # | 端末 | 操作 |
|---|---|---|
| 1 | 家のPC（M5と同じLAN） | `brew install --cask tailscale` → ログイン → `tailscale ip -4` で 100.x.y.z を控える |
| 2 | 家のPC | `cd inverted-pendulum-m5stick && python3 tools/server.py` を常駐（`tmux` や `nohup` 推奨） |
| 3 | 家のPC | `tools/local_config.py` の `CORS_ALLOW_ORIGINS = ["https://<owner>.github.io"]` を設定 |
| 4 | 持ち出し端末 | Tailscale をインストール → 同じアカウントでログイン |
| 5 | 持ち出し端末 | ブラウザで `https://<owner>.github.io/inverted-pendulum-m5stick/` → 右下を3回タップ → パスワード → Backend URL に `http://100.x.y.z:5000` |

#### トラブルシュート

| 症状 | 原因 | 対策 |
|---|---|---|
| Backend URL の疎通テストが NG | 家のPCで server.py が落ちてる / Tailscale 切断 | `tailscale status` で双方 online 確認、server.py 再起動 |
| LIVE 切替後 CORS エラー | `CORS_ALLOW_ORIGINS` 未設定 | `tools/local_config.py` に Pages の origin を追加して再起動 |
| 家のPCはOK、M5に届かない | 家のWiFi DHCP で M5 のIPが変わった | `tools/local_config.py` の `M5_URL` を再設定 or mDNS 化 |
| Tailscale が遅い | DERPリレー経由になっている | 双方 IPv6 / UPnP を許可、もしくは諦めて使う（実用上問題なし） |

### 2-9. LIVE モードのセキュリティ設計

GitHub Pages 上で本番機を操作させるための設計：

```
Pages (静的)
   │
   ├─ window.IPS_LIVE_HASH = "<sha256>" ← build_demo.py が埋め込み
   │  window.IPS_LIVE_DEFAULT_URL = "..."
   │
   ├─ 右下60×60px 3連タップ (1.2s 内)
   │  → prompt(password)
   │  → SubtleCrypto.digest('SHA-256', utf8(pw))
   │  → hex 比較
   │
   ├─ OK なら Backend URL を localStorage に保存 + IPS_LIVE_OK=1
   │
   └─ 以後 fetch('/api/*') → fetch(<backend>/api/*, {mode:'cors'})
```

セキュリティ層別の評価:

| 層 | 想定脅威 | 対策 |
|---|---|---|
| Pages のソース閲覧 | パスワード奪取 | ❌ 平文なし／✅ SHA-256 のみ。辞書攻撃には弱い → 強固なパスフレーズ必須 |
| ハッシュ突破 | LIVE 解除 | ✅ 二重鍵：Backend URL（Tailscale IP）を知らないと到達不可 |
| Backend URL 漏れ | 第三者の操作 | ✅ Tailscale で公開IP外。タネがバレてもアクセス権がない |
| CORS | 任意サイトからの fetch | ✅ `Access-Control-Allow-Origin` を Pages の origin に限定 |
| LAN内の盗聴 | パケット解析 | ⚠ 家のWiFi WPA2/3 に依存。HTTPSではない |
| DoS | モーター連続起動 | ⚠ 認可機構なし。LAN/Tailnet 内信頼前提 |

#### パスワードを変えるとき

```bash
# tools/local_config.py を編集して LIVE_PASSWORD を新しい値に
python3 tools/build_demo.py
git add docs/demo/index.html
git commit -m "rotate LIVE password"
git push
```
ハッシュは `docs/demo/index.html` の上部に埋まる。`git diff` でハッシュだけ変わる（パスワード本体は出ない）。

### 2-10. GitHub Pages デモの fetch シム

静的ホスト上ではFlaskが居ないため、`window.fetch` を起動時に差し替える：

```js
const _origFetch = window.fetch.bind(window);
window.fetch = async (url, opts) => {
  if (url.startsWith('/api/s'))   return json(simulatedDeviceState);
  if (url.startsWith('/api/c'))   { mutate(simulatedDeviceState, parseQ(url)); return ok(); }
  if (url === '/api/sessions')    return _origFetch('data/sessions/index.json');
  const m = url.match(/^\/api\/sessions\/(.+)/);
  if (m)                          return _origFetch(`data/sessions/${m[1]}.json`);
  return _origFetch(url, opts);
};
```

- Service Workerを使わず**1ファイルで完結**（GitHub Pages の単純デプロイに収まる）
- `Response` オブジェクトを手で組み立てて返す（`new Response(JSON.stringify(obj), {status:200, headers:...})`）
- 50ms周期の `setInterval` でシミュレートされたデバイス状態を更新（実機の制御則を簡略化したもの）
- 利点：ビルドが一段。欠点：オフラインキャッシュは不可。

### 2-11. セキュリティに関する注意

このプロジェクトは **教育・自宅ホビー前提**で書かれており、以下の脆弱性を含む。インターネット直公開は推奨しない：

| 項目 | 現状 | 緩和策（必要なら） |
|---|---|---|
| HTTP（平文） | M5↔ブラウザは平文 | 自宅LANに留め、Tailscale等のVPNで遠隔越し |
| 認証なし | `/c?q=on` を投げれば誰でもモーター起動可 | リバースプロキシで Basic 認証を被せる |
| CSRF対策なし | GETでステートを変える設計（軽量化のため） | 同一オリジンに留める運用で実害最小 |
| WiFiパスワード平文埋め込み | `wifi_config.h` 内 | リポジトリは `.gitignore` 済み。共有時は配慮 |

---

## 付録 — このプロジェクトの実装位置

| 機能 | ファイル | 行 |
|---|---|---|
| マルチSSID + APフォールバック | `inverted_pendulum/inverted_pendulum.ino` | `setupWiFi()` |
| WiFi設定（個人用、git無視） | `inverted_pendulum/wifi_config.h` | — |
| WiFi設定テンプレ + 説明 | `inverted_pendulum/wifi_config.h.example` | 全部 |
| Flaskプロキシ | `tools/server.py` | `/api/s`, `/api/c` |
| 起動時診断（実プローブ） | `tools/templates/index.html` | `bootDiag()` |
| GitHub Pages 用 fetch シム | `tools/templates/index.html` | `demoShim()` |
| デモビルドスクリプト | `tools/build_demo.py` | 全部 |

---

## 用語ミニ辞典

| 用語 | 意味 |
|---|---|
| **SSID** | WiFiネットワーク名（家のWiFiに付けてある名前） |
| **STA / Station** | 既存APに繋ぐ「子機」モード |
| **AP / Access Point** | 自分が「親機」になり子機を受け入れるモード |
| **DHCP** | 接続してきた子機にIPアドレスを自動配布する仕組み |
| **mDNS** | `pendulum.local` のような **`.local`名前解決**。LAN内専用 |
| **CORS** | 別オリジン（別ドメイン/別ポート）からのfetchを制御するブラウザの仕組み |
| **NAT** | 家のルータが「ローカルIP ↔ グローバルIP」を翻訳する仕組み |
| **WPA2-PSK** | WiFiパスワード認証方式。8文字以上必須 |
| **VPN** | 物理的に別の場所のPC同士を、暗号トンネルで「同じLAN扱い」にする技術 |
| **Tailscale** | WireGuardベースの簡単VPNサービス（無料枠あり） |
