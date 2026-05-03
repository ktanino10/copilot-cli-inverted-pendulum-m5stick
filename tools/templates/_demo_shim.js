// ═══════════════════════════════════════════════════════════════════
// IPS demo / LIVE / close-button shim — shared between index.html and
// dash.html.  Loaded as <script src="/_demo_shim.js"></script>.
//
// Three IIFEs that all early-return harmlessly when their flag is off:
//   1. liveGate()      — hidden 3-tap unlock + LIVE-mode fetch rewrite
//   2. demoShim()      — when window.IPS_DEMO is true, intercept /api/*
//                        with an in-browser simulated device + bundled
//                        session JSON.  Disabled in LIVE mode.
//   3. closeButton()   — small "✕ CLOSE" link bottom-right that returns
//                        the visitor to the GitHub repo. Only shown in
//                        DEMO or LIVE (never on the dev/Flask server).
//
// Build-time placeholders set in the parent HTML BEFORE this script
// loads:
//   window.IPS_DEMO              — true when served from Pages demo
//   window.IPS_LIVE_HASH         — sha-256(LIVE_PASSWORD), hex
//   window.IPS_LIVE_DEFAULT_URL  — pre-filled backend URL (may be "")
//   window.IPS_REPO_URL          — repo URL for ✕ CLOSE target
// ═══════════════════════════════════════════════════════════════════

(function liveGate() {
  const LS_URL = 'ips_live_url';
  const LS_OK  = 'ips_live_ok';
  const hashTarget = (window.IPS_LIVE_HASH || '').toLowerCase();
  const liveUrl = () => localStorage.getItem(LS_URL) || '';
  const isLive  = () => !!liveUrl() && localStorage.getItem(LS_OK) === '1';
  window.__ipsLiveUrl = liveUrl;
  window.__ipsIsLive  = isLive;

  async function sha256Hex(s) {
    const enc = new TextEncoder().encode(s);
    const buf = await crypto.subtle.digest('SHA-256', enc);
    return Array.from(new Uint8Array(buf))
      .map(b => b.toString(16).padStart(2, '0')).join('');
  }

  async function unlockFlow() {
    if (!hashTarget) {
      alert('LIVE モードはこのビルドでは無効です。\n(tools/local_config.py の LIVE_PASSWORD を設定して再ビルドしてください)');
      return;
    }
    const pw = prompt('🔒 LIVE-mode password:');
    if (pw == null || pw === '') return;
    const h = await sha256Hex(pw);
    if (h !== hashTarget) { alert('❌ Wrong password.'); return; }
    const def = localStorage.getItem(LS_URL) || window.IPS_LIVE_DEFAULT_URL || 'http://192.168.0.42:5000';
    const url = prompt(
      'Backend URL (your home or phone-tether server.py):\n' +
      '  http://<your-pc-ip>:5000  /  http://pendulum.local:5000',
      def
    );
    if (!url) return;
    const clean = url.replace(/\/+$/, '');
    try {
      const r = await fetch(clean + '/api/s', { mode: 'cors' });
      if (!r.ok) throw new Error('HTTP ' + r.status);
    } catch (e) {
      if (!confirm('⚠ Connection test failed: ' + e.message + '\nSave anyway?')) return;
    }
    localStorage.setItem(LS_URL, clean);
    localStorage.setItem(LS_OK,  '1');
    alert('✅ LIVE mode unlocked. Reloading…');
    location.reload();
  }
  function exitLive() {
    if (!confirm('Exit LIVE mode and return to demo?')) return;
    localStorage.removeItem(LS_OK);
    location.reload();
  }
  window.__ipsExitLive   = exitLive;
  window.__ipsUnlockLive = unlockFlow;

  document.addEventListener('DOMContentLoaded', () => {
    let taps = [];
    function corner(e) {
      // Ignore taps that originated on the ✕ CLOSE button (it lives in the
      // same bottom-right corner — its onclick also stopPropagation()s,
      // but this is belt-and-braces in case event ordering differs).
      const t = e.target;
      if (t && t.closest && t.closest('#ipsCloseBtn')) return false;
      const x = e.clientX ?? (e.touches && e.touches[0] && e.touches[0].clientX);
      const y = e.clientY ?? (e.touches && e.touches[0] && e.touches[0].clientY);
      if (x == null || y == null) return false;
      return x > innerWidth - 60 && y > innerHeight - 60;
    }
    function tap(e) {
      if (!corner(e)) return;
      const now = Date.now();
      taps = taps.filter(t => now - t < 1200);
      taps.push(now);
      if (taps.length >= 3) { taps = []; unlockFlow(); }
    }
    document.addEventListener('click', tap);
    document.addEventListener('touchend', tap);

    if (isLive()) {
      const b = document.createElement('div');
      b.innerHTML = '🔴 LIVE <span style="opacity:.55;font-size:9px;margin-left:6px;cursor:pointer" id="ipsExitLive">[exit]</span>';
      Object.assign(b.style, {
        position: 'fixed', top: '8px', right: '8px', zIndex: 9999,
        background: 'rgba(255,80,80,.18)', border: '1px solid #ff5050',
        color: '#ff7878', fontFamily: 'Orbitron,sans-serif', fontSize: '11px',
        letterSpacing: '.25em', padding: '4px 12px',
        clipPath: 'polygon(8px 0,100% 0,calc(100% - 8px) 100%,0 100%)',
        textShadow: '0 0 6px #ff5050',
      });
      document.body.appendChild(b);
      document.getElementById('ipsExitLive').onclick = exitLive;
    }
  });
})();

(function demoShim() {
  const liveActive = window.__ipsIsLive && window.__ipsIsLive();
  const liveBase = liveActive ? window.__ipsLiveUrl() : '';
  const _origFetch = window.fetch.bind(window);

  if (liveActive) {
    window.fetch = (url, opts) => {
      if (typeof url === 'string' && url.startsWith('/api/')) {
        return _origFetch(liveBase + url, Object.assign({ mode: 'cors' }, opts || {}));
      }
      return _origFetch(url, opts);
    };
    return;
  }

  if (!window.IPS_DEMO) return;
  const dev = {
    angle: 6.0, dangle: 0, power: 0, p_term: 0, d_term: 0,
    on: false, batt: 4.05, speed: 0,
    kp: 50, kd: 8, ki: 3.2, kspd: 0.08, ppos: 1.0, pneg: 1.2,
    bias: 0, alim: 45, plim: 350, kpower: 0.0001, minp: 0, po: 0,
    _t0: performance.now(),
  };
  setInterval(() => {
    const t = (performance.now() - dev._t0) / 1000;
    if (dev.on) {
      dev.angle  = Math.sin(t * 1.6) * 1.4 + (Math.random() - .5) * 0.5 + dev.bias * 0.4;
      dev.dangle = Math.cos(t * 1.6) * 2.2 + (Math.random() - .5) * 0.6;
    } else {
      dev.angle  = Math.sin(t * 0.6) * 6 + (Math.random() - .5) * 0.4;
      dev.dangle = Math.cos(t * 0.6) * 4 + (Math.random() - .5) * 0.4;
    }
    dev.p_term = dev.kp * dev.angle;
    dev.d_term = dev.kd * dev.dangle;
    dev.power  = dev.on ? Math.max(-dev.plim, Math.min(dev.plim, Math.round(dev.p_term + dev.d_term))) : 0;
    dev.batt   = Math.max(3.65, dev.batt - 0.0000015);
  }, 50);

  const json = (obj, status = 200) =>
    new Response(JSON.stringify(obj), { status, headers: { 'Content-Type': 'application/json' } });

  // Resolve a "data/sessions/<id>.json" path that works both at root
  // (index.html) and one level deeper if we ever end up there.
  function dataPath(p) {
    return (location.pathname.endsWith('/dash.html') ? './' : '') + p;
  }

  window.fetch = async (url, opts) => {
    if (typeof url !== 'string') return _origFetch(url, opts);

    // ⚠ Order matters: /api/sessions and /api/sessions/<id> MUST be matched
    // BEFORE the /api/s status endpoint, otherwise startsWith('/api/s')
    // would swallow them and return the device state instead of session data.
    if (url === '/api/sessions' || url.startsWith('/api/sessions?')) {
      if (opts && opts.method && opts.method.toUpperCase() === 'POST') {
        return json({ id: 'demo_' + Date.now(), path: '(demo, not saved)' });
      }
      try { return await _origFetch(dataPath('data/sessions/index.json')); }
      catch (e) { return json([]); }
    }
    const sm = url.match(/^\/api\/sessions\/(.+?)\/?$/);
    if (sm) {
      if (opts && opts.method && opts.method.toUpperCase() === 'DELETE') return json({ ok: true });
      try { return await _origFetch(dataPath(`data/sessions/${sm[1]}.json`)); }
      catch (e) { return json({ error: 'not found' }, 404); }
    }
    if (url === '/api/s' || url.startsWith('/api/s?')) {
      await new Promise(r => setTimeout(r, 30));
      return json(dev);
    }
    if (url === '/api/c' || url.startsWith('/api/c?')) {
      const m = url.match(/[?&]q=([^&]+)/);
      if (m) {
        const q = decodeURIComponent(m[1]);
        if (q === 'on') dev.on = true;
        else if (q === 'off') dev.on = false;
        else if (q === 'zero') { /* no-op */ }
        else {
          const kv = q.split('=');
          if (kv.length === 2 && kv[0] in dev) {
            const v = parseFloat(kv[1]);
            if (!Number.isNaN(v)) dev[kv[0]] = v;
          }
        }
      }
      return new Response('OK', { status: 200 });
    }
    if (url === '/api/m5_target') {
      // Demo mode has no real backend — return a stub so the widget
      // doesn't error out. (The widget itself is hidden in demo mode,
      // but other code paths might still call this endpoint.)
      return json({ url: '(demo mode — no backend)' });
    }
    return _origFetch(url, opts);
  };

  document.addEventListener('DOMContentLoaded', () => {
    const b = document.createElement('div');
    b.textContent = 'DEMO MODE · simulated device · no hardware required';
    Object.assign(b.style, {
      position: 'fixed', bottom: '8px', left: '8px', zIndex: 9998,
      background: 'rgba(184,102,255,.15)', border: '1px solid #b866ff',
      color: '#b866ff', fontFamily: 'Orbitron,sans-serif', fontSize: '10px',
      letterSpacing: '.2em', padding: '4px 10px', pointerEvents: 'none',
      clipPath: 'polygon(6px 0,100% 0,calc(100% - 6px) 100%,0 100%)',
      textShadow: '0 0 6px #b866ff',
    });
    document.body.appendChild(b);
  });
})();

(function closeButton() {
  // Visible "✕ CLOSE" only when this is the Pages-served demo OR a
  // user is in LIVE mode.  On a plain dev/Flask server we don't add
  // the button — there's nowhere meaningful to "close" to.
  //
  // Placed bottom-right (above the DEMO badge column on the left).
  // The 3-tap LIVE unlock hot-spot also sits in the bottom-right
  // corner, so we (a) stopPropagation on the button so its clicks
  // don't count as taps, and (b) liveGate's corner() ignores events
  // whose target is inside this element.
  document.addEventListener('DOMContentLoaded', () => {
    const inDemo = !!window.IPS_DEMO;
    const inLive = window.__ipsIsLive && window.__ipsIsLive();
    if (!inDemo && !inLive) return;
    const repo = window.IPS_REPO_URL ||
      'https://github.com/ktanino10/copilot-cli-inverted-pendulum-m5stick';
    const a = document.createElement('a');
    a.id = 'ipsCloseBtn';
    a.href = repo;
    a.title = 'Close demo and return to the GitHub repository';
    a.innerHTML = '✕ CLOSE';
    Object.assign(a.style, {
      position: 'fixed', bottom: '8px', right: '8px', zIndex: 9999,
      background: 'rgba(0,0,0,.55)', border: '1px solid #888',
      color: '#bbb', fontFamily: 'Orbitron,sans-serif', fontSize: '10px',
      letterSpacing: '.25em', padding: '5px 12px',
      textDecoration: 'none', cursor: 'pointer',
      clipPath: 'polygon(8px 0,100% 0,calc(100% - 8px) 100%,0 100%)',
    });
    a.onmouseenter = () => { a.style.color = '#fff'; a.style.borderColor = '#fff'; };
    a.onmouseleave = () => { a.style.color = '#bbb'; a.style.borderColor = '#888'; };
    // Don't let the button's click bubble to the document-level
    // 3-tap detector in liveGate().
    const swallow = (ev) => ev.stopPropagation();
    a.addEventListener('click', swallow);
    a.addEventListener('touchend', swallow);
    document.body.appendChild(a);
  });
})();

(function m5TargetWidget() {
  // ───────────────────────────────────────────────────────────────────
  // Floating "M5 target" widget. Lets the user point server.py at a
  // different M5 IP from the browser (no file edit / no restart).
  //
  // Useful when the M5's IP changes between locations:
  //   home WiFi  → 192.168.10.32
  //   Pixel hot. → 10.39.x.x (DHCP, varies)
  //   AP mode    → 192.168.4.1
  //
  // Skipped in DEMO mode (no real backend to talk to).
  // ───────────────────────────────────────────────────────────────────
  if (window.IPS_DEMO) return;

  const LS = 'ips_m5_target';

  function normalize(raw) {
    let s = (raw || '').trim().replace(/\/+$/, '');
    if (!s) return '';
    if (!/^https?:\/\//i.test(s)) s = 'http://' + s;
    return s;
  }

  async function pushToServer(url) {
    try {
      const r = await fetch('/api/m5_target', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ url }),
      });
      if (!r.ok) return null;
      const j = await r.json();
      return j.url || null;
    } catch (e) { return null; }
  }

  async function fetchCurrent() {
    try {
      const r = await fetch('/api/m5_target');
      if (!r.ok) return null;
      const j = await r.json();
      return j.url || null;
    } catch (e) { return null; }
  }

  document.addEventListener('DOMContentLoaded', async () => {
    const wrap = document.createElement('div');
    Object.assign(wrap.style, {
      display: 'flex', alignItems: 'center', gap: '6px',
      color: '#9adcff', fontFamily: 'Orbitron,sans-serif', fontSize: '11px',
      letterSpacing: '.15em', padding: '2px 10px',
      background: 'rgba(0,0,0,.35)', border: '1px solid #355',
      clipPath: 'polygon(6px 0,calc(100% - 6px) 0,100% 100%,0 100%)',
    });

    const label = document.createElement('span');
    label.textContent = 'M5:';
    label.style.color = '#7fbfff';
    wrap.appendChild(label);

    const view = document.createElement('span');
    view.style.color = '#fff';
    view.style.cursor = 'pointer';
    view.title = 'click to change';
    wrap.appendChild(view);

    const input = document.createElement('input');
    input.type = 'text';
    input.placeholder = '10.39.199.241';
    Object.assign(input.style, {
      display: 'none',
      background: '#000', border: '1px solid #7fbfff', color: '#fff',
      fontFamily: 'monospace', fontSize: '11px',
      padding: '2px 6px', width: '170px', outline: 'none',
    });
    wrap.appendChild(input);

    const btn = document.createElement('button');
    btn.textContent = 'APPLY';
    Object.assign(btn.style, {
      display: 'none',
      background: '#7fbfff', color: '#000', border: 'none',
      fontFamily: 'Orbitron,sans-serif', fontSize: '10px',
      letterSpacing: '.15em', padding: '3px 8px',
      cursor: 'pointer',
    });
    wrap.appendChild(btn);

    const cancel = document.createElement('button');
    cancel.textContent = '✕';
    Object.assign(cancel.style, {
      display: 'none',
      background: 'transparent', color: '#888', border: '1px solid #555',
      fontSize: '10px', padding: '2px 6px', cursor: 'pointer',
    });
    wrap.appendChild(cancel);

    function setView(url) {
      view.textContent = url || '(unset)';
    }

    function showInput(prefill) {
      view.style.display = 'none';
      label.style.display = 'none';
      input.style.display = '';
      btn.style.display = '';
      cancel.style.display = '';
      input.value = prefill || '';
      input.focus();
      input.select();
    }
    function showView() {
      view.style.display = '';
      label.style.display = '';
      input.style.display = 'none';
      btn.style.display = 'none';
      cancel.style.display = 'none';
    }

    async function apply() {
      const url = normalize(input.value);
      if (!url) { input.focus(); return; }
      const got = await pushToServer(url);
      if (got) {
        localStorage.setItem(LS, got);
        setView(got);
        view.style.color = '#9fff7f';
        setTimeout(() => { view.style.color = '#fff'; }, 1200);
      } else {
        view.style.color = '#ff7f7f';
        setTimeout(() => { view.style.color = '#fff'; }, 1500);
      }
      showView();
    }

    view.addEventListener('click', () => showInput(view.textContent));
    btn.addEventListener('click', apply);
    cancel.addEventListener('click', showView);
    input.addEventListener('keydown', (e) => {
      if (e.key === 'Enter') apply();
      else if (e.key === 'Escape') showView();
    });

    // Don't let widget clicks bubble to liveGate's 3-tap unlock corner.
    const swallow = (ev) => ev.stopPropagation();
    wrap.addEventListener('click', swallow);
    wrap.addEventListener('touchend', swallow);

    // ─── 🔗 LINK MONA toggle (mirror dashboard mascot to M5 LCD) ───
    // Server pushes ~5 FPS JPEG frames of the dashboard mascot to the
    // M5's /face endpoint. Off by default so the IP/SSID stays visible
    // when the user is still configuring the device.
    const sep = document.createElement('span');
    sep.textContent = '·';
    sep.style.color = '#555';
    sep.style.padding = '0 4px';
    wrap.appendChild(sep);

    const linkBtn = document.createElement('button');
    linkBtn.type = 'button';
    Object.assign(linkBtn.style, {
      background: 'transparent', color: '#9adcff',
      border: '1px solid #555', cursor: 'pointer',
      fontFamily: 'Orbitron,sans-serif', fontSize: '10px',
      letterSpacing: '.18em', padding: '3px 8px',
      transition: 'background .15s, color .15s, border-color .15s',
    });
    wrap.appendChild(linkBtn);

    let linked = false;
    function paintLinkBtn() {
      if (linked) {
        linkBtn.textContent = '🔗 LINKED';
        linkBtn.style.color = '#000';
        linkBtn.style.background = '#9fff7f';
        linkBtn.style.borderColor = '#9fff7f';
      } else {
        linkBtn.textContent = '🔓 LINK MONA';
        linkBtn.style.color = '#9adcff';
        linkBtn.style.background = 'transparent';
        linkBtn.style.borderColor = '#555';
      }
    }
    paintLinkBtn();

    async function toggleLink() {
      const want = !linked;
      linkBtn.disabled = true;
      try {
        const r = await fetch('/api/lcd_link', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ on: want }),
        });
        const j = await r.json().catch(() => ({}));
        if (r.ok) {
          linked = !!j.on;
        } else {
          linkBtn.style.color = '#ff7f7f';
          setTimeout(() => paintLinkBtn(), 1200);
          if (j && j.error) console.warn('lcd_link:', j.error);
        }
      } catch (e) {
        console.warn('lcd_link toggle failed:', e);
      } finally {
        linkBtn.disabled = false;
        paintLinkBtn();
      }
    }
    linkBtn.addEventListener('click', toggleLink);

    // Hang a thin sub-bar directly below the main header, right-aligned so
    // the widget sits visually under the LINK / MOTOR / PWR pips. This
    // keeps it well clear of the link-lost Ducky banner that drops in from
    // the very top of the screen.
    const subbar = document.createElement('div');
    Object.assign(subbar.style, {
      display: 'flex', justifyContent: 'flex-end',
      padding: '4px 18px 6px',
      background: 'linear-gradient(90deg,#000a14,#001828,#000a14)',
      borderBottom: '1px solid #1a3a4a',
    });
    subbar.appendChild(wrap);
    const header = document.querySelector('header');
    if (header && header.parentElement) {
      header.parentElement.insertBefore(subbar, header.nextSibling);
    } else {
      document.body.appendChild(wrap);
    }

    // Boot: prefer the localStorage value (push it to the server so a
    // fresh server.py picks up where we left off). Otherwise fetch
    // whatever the server already has.
    const saved = localStorage.getItem(LS);
    if (saved) {
      const got = await pushToServer(saved);
      setView(got || saved);
    } else {
      const cur = await fetchCurrent();
      setView(cur || '');
    }

    // Sync the LINK button with whatever the server thinks the state is
    // (in case server.py was restarted while the page stayed open).
    try {
      const r = await fetch('/api/lcd_link');
      if (r.ok) {
        const j = await r.json();
        linked = !!j.on;
        paintLinkBtn();
      }
    } catch (e) { /* server not up yet — leave default */ }
  });
})();
