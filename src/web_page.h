#ifndef WEB_PAGE_H
#define WEB_PAGE_H

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <title>STU Smart Gate</title>
  <style>
    :root {
      --bg-top: #181938;
      --bg-bot: #0d0e22;
      --purple: #7c4dff;
      --purple-border: #4d3d8c;
      --text: #ffffff;
      --muted: #6c7293;
      --green: #00e676;
      --red: #ff3d57;
      --accent: #38bdf8;
    }
    * { box-sizing: border-box; touch-action: manipulation; user-select: none; -webkit-tap-highlight-color: transparent; }
    body {
      margin: 0;
      background: linear-gradient(180deg, var(--bg-top) 0%, var(--bg-bot) 100%);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, sans-serif;
      font-weight: 300;
      display: flex; flex-direction: column; align-items: center;
      min-height: 100vh; padding: 18px 20px;
    }

    /* TOP BAR: SSID STATUS PILL & DROPDOWN */
    .top-bar {
      width: 100%; max-width: 340px; display: flex; justify-content: flex-end;
      position: relative; margin-bottom: 15px; z-index: 100;
    }
    .ssid-pill {
      background: rgba(255,255,255,0.08); backdrop-filter: blur(8px);
      border: 1px solid rgba(255,255,255,0.15);
      padding: 7px 15px; border-radius: 999px;
      display: flex; align-items: center; gap: 8px;
      font-size: 0.85rem; font-weight: 400; cursor: pointer;
    }
    .dot {
      width: 10px; height: 10px; border-radius: 50%;
      background: var(--red); box-shadow: 0 0 8px var(--red);
    }
    .dot.connected { background: var(--green); box-shadow: 0 0 8px var(--green); }

    .ssid-dropdown {
      display: none; position: absolute; top: 44px; right: 0; width: 100%;
      background: #1e2046; border: 1px solid var(--purple-border);
      border-radius: 16px; padding: 16px; box-shadow: 0 15px 35px rgba(0,0,0,0.75);
    }
    .ssid-dropdown.open { display: block; }
    .dropdown-header {
      display: flex; justify-content: space-between; align-items: center;
      margin-bottom: 10px;
    }
    .dropdown-header h4 { margin: 0; font-size: 0.9rem; color: #b39ddb; font-weight: 400; }
    .scan-btn {
      background: #2a2b54; border: 1px solid var(--purple-border);
      color: var(--accent); padding: 4px 10px; border-radius: 8px;
      font-size: 0.75rem; cursor: pointer; font-weight: 500;
    }

    .input-clean {
      width: 100%; padding: 10px 12px; margin-bottom: 10px;
      background: rgba(0,0,0,0.25); border: 1px solid var(--purple-border);
      border-radius: 8px; color: #fff; font-size: 0.9rem; outline: none;
    }
    .input-clean:focus { border-color: var(--purple); }
    select.input-clean { color: #fff; }
    select.input-clean option { background: #1e2046; color: #fff; }

    .drop-btns { display: flex; gap: 8px; margin-top: 4px; }
    .btn-small {
      flex: 1; padding: 10px; border-radius: 8px; border: none;
      font-size: 0.85rem; cursor: pointer; font-weight: 500;
    }
    .btn-save-wifi { background: var(--purple); color: #fff; }
    .btn-logout-wifi { background: rgba(255,61,87,0.2); color: var(--red); border: 1px solid var(--red); }

    /* PAGE 1: MINIMALIST KEYPAD */
    #page1 {
      width: 100%; max-width: 320px; display: flex;
      flex-direction: column; align-items: center; flex: 1;
    }
    .stu-logo {
      width: 115px; height: 115px; border-radius: 50%;
      border: 1.5px solid var(--purple-border);
      background: rgba(124, 77, 255, 0.05);
      display: flex; flex-direction: column; align-items: center; justify-content: center;
      text-align: center; margin-bottom: 22px;
    }
    .stu-logo span { font-size: 0.65rem; letter-spacing: 2px; color: var(--muted); text-transform: uppercase; }
    .stu-logo h2 { margin: 4px 0 0 0; font-size: 1rem; font-weight: 400; letter-spacing: 0.5px; padding: 0 10px; }

    .pin-display-wrap { width: 80%; text-align: center; margin-bottom: 26px; }
    .pin-readout {
      font-size: 1.7rem; letter-spacing: 4px; min-height: 36px;
      color: var(--text); font-weight: 300;
    }
    .pin-line {
      width: 100%; height: 1px; background: var(--purple-border);
      margin-top: 6px; position: relative; overflow: hidden;
    }
    .pin-timer-bar {
      height: 100%; width: 0%; background: var(--purple);
      transition: width 0.1s linear;
    }

    .keypad-grid {
      display: grid; grid-template-columns: repeat(3, 72px);
      column-gap: 24px; row-gap: 16px; justify-content: center;
    }
    .key-wrap { display: flex; flex-direction: column; align-items: center; }
    .circle-key {
      width: 72px; height: 72px; border-radius: 50%;
      background: transparent; border: 1.2px solid var(--purple-border);
      color: var(--text); font-size: 1.65rem; font-weight: 300;
      display: flex; align-items: center; justify-content: center;
      cursor: pointer; transition: all 0.12s ease; padding: 0;
    }
    .circle-key:active, .circle-key.lit {
      background: var(--purple); border-color: var(--purple);
      box-shadow: 0 0 18px rgba(124, 77, 255, 0.6);
      transform: scale(0.95);
    }
    .key-sub { font-size: 0.72rem; color: var(--muted); margin-top: 6px; }

    .footer-meta {
      margin-top: auto; padding-top: 22px; text-align: center;
      display: flex; flex-direction: column; align-items: center; gap: 12px;
    }
    .date-label { font-size: 0.85rem; color: var(--muted); letter-spacing: 0.5px; }
    .gear-btn {
      background: none; border: none; color: var(--muted);
      cursor: pointer; padding: 6px; transition: color 0.2s;
    }
    .gear-btn:active { color: var(--purple); }

    /* PAGE 2: GATE DASHBOARD */
    #page2 {
      display: none; width: 100%; max-width: 340px;
      flex-direction: column; align-items: center;
    }
    .camera-frame {
      width: 100%; height: 210px; border-radius: 18px;
      border: 1.2px solid var(--purple-border);
      background: rgba(255,255,255,0.03);
      display: flex; flex-direction: column; align-items: center; justify-content: center;
      margin-bottom: 22px; color: var(--muted);
    }
    .status-pill {
      width: 100%; border: 1.2px solid var(--purple-border);
      background: rgba(255,255,255,0.04); border-radius: 999px;
      padding: 12px 24px; display: flex; justify-content: space-around;
      margin-bottom: 20px;
    }
    .stat-group { display: flex; align-items: center; gap: 10px; font-size: 0.95rem; }
    .stat-dot { width: 16px; height: 16px; border-radius: 50%; transition: 0.25s; }
    .dot-closed { background: var(--green); box-shadow: 0 0 10px var(--green); }
    .dot-open { background: var(--red); box-shadow: 0 0 10px var(--red); }
    .dot-trig-off { background: rgba(255,255,255,0.2); }
    .dot-trig-on { background: var(--purple); box-shadow: 0 0 12px var(--purple); }

    .gate-btn-row {
      width: 100%; display: grid; grid-template-columns: 1fr 1fr;
      gap: 14px; margin-bottom: 16px;
    }
    .clean-gate-btn {
      height: 95px; border-radius: 18px; background: transparent;
      border: 1.2px solid var(--purple-border); color: var(--text);
      font-size: 1.15rem; font-weight: 400; letter-spacing: 1px;
      cursor: pointer; transition: 0.15s;
    }
    .clean-gate-btn:active {
      background: var(--purple); border-color: var(--purple);
      box-shadow: 0 0 20px rgba(124, 77, 255, 0.5);
    }
    .clean-logout-btn {
      width: 100%; height: 58px; border-radius: 18px;
      background: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.15);
      color: var(--text); font-size: 1rem; letter-spacing: 1.5px; cursor: pointer;
    }
    .clean-logout-btn:active { background: var(--red); border-color: var(--red); }

    /* ADMIN MODAL & TOAST */
    .modal {
      display: none; position: fixed; inset: 0; background: rgba(10,11,26,0.85);
      align-items: center; justify-content: center; z-index: 200; padding: 20px;
    }
    .modal-box {
      background: #181938; border: 1px solid var(--purple-border);
      padding: 22px; border-radius: 18px; width: 100%; max-width: 320px; text-align: center;
    }
    .section-lbl {
      font-size: 0.75rem; color: #b39ddb; text-align: left;
      margin: 8px 0 4px 2px; display: block;
    }
    .toast {
      position: fixed; bottom: 24px; background: #23254d; color: #fff;
      border: 1px solid var(--purple); padding: 12px 20px; border-radius: 999px;
      font-size: 0.85rem; display: none; z-index: 300; box-shadow: 0 8px 20px rgba(0,0,0,0.6);
      max-width: 90%; text-align: center;
    }
  </style>
</head>
<body>

  <!-- TOP BAR: SSID PILL & DROPDOWN -->
  <div class="top-bar">
    <div class="ssid-pill" onclick="toggleWifiMenu()">
      <div class="dot" id="wifiDot"></div>
      <span id="wifiText">Checking Wi-Fi...</span>
    </div>

    <div class="ssid-dropdown" id="wifiMenu">
      <div class="dropdown-header">
        <h4>Available 2.4GHz Networks</h4>
        <button class="scan-btn" onclick="scanNearbyWifi()">&#x21bb; Scan</button>
      </div>

      <select class="input-clean" id="scannedNetworks" onchange="selectScannedSsid()" style="display:none;">
        <option value="">-- Select detected network --</option>
      </select>

      <input type="text" class="input-clean" id="ssidInput" placeholder="Wi-Fi Name (SSID)">
      <input type="password" class="input-clean" id="passInput" placeholder="Wi-Fi Password">
      <div class="drop-btns">
        <button class="btn-small btn-save-wifi" onclick="saveWifi()">Connect</button>
        <button class="btn-small btn-logout-wifi" onclick="logoutWifi()">Log Out</button>
      </div>
    </div>
  </div>

  <!-- PAGE 1: MINIMALIST KEYPAD -->
  <div id="page1">
    <div class="stu-logo">
      <span>Smart Gate</span>
      <h2>STU SYSTEM</h2>
    </div>

    <div class="pin-display-wrap">
      <div class="pin-readout" id="pinReadout"></div>
      <div class="pin-line"><div class="pin-timer-bar" id="timerBar"></div></div>
    </div>

    <div class="keypad-grid">
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('1', this)">1</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('2', this)">2</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('3', this)">3</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('4', this)">4</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('5', this)">5</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('6', this)">6</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('7', this)">7</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('8', this)">8</button></div>
      <div class="key-wrap"><button class="circle-key" onclick="pressDigit('9', this)">9</button></div>

      <div class="key-wrap">
        <button class="circle-key" onclick="clearPin(this)">C</button>
        <span class="key-sub">Cancel</span>
      </div>
      <div class="key-wrap">
        <button class="circle-key" onclick="pressDigit('0', this)">0</button>
      </div>
      <div class="key-wrap">
        <button class="circle-key" onclick="submitPin(this)">&#8629;</button>
        <span class="key-sub">Enter</span>
      </div>
    </div>

    <div class="footer-meta">
      <div class="date-label" id="dateLabel"></div>
      <button class="gear-btn" onclick="openAdminModal()" title="Administrator Mode">
        <svg width="26" height="26" fill="none" stroke="currentColor" stroke-width="1.5" viewBox="0 0 24 24">
          <path d="M12 15a3 3 0 1 0 0-6 3 3 0 0 0 0 6Z"/>
          <path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1Z"/>
        </svg>
      </button>
    </div>
  </div>

  <!-- PAGE 2: GATE CONTROL SCREEN -->
  <div id="page2">
    <div class="camera-frame">
      <svg width="36" height="36" fill="none" stroke="currentColor" stroke-width="1.3" viewBox="0 0 24 24">
        <path d="M23 19a2 2 0 0 1-2 2H3a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h4l2-3h6l2 3h4a2 2 0 0 1 2 2z"/>
        <circle cx="12" cy="13" r="4"/>
      </svg>
      <p style="margin-top:8px; font-size:0.95rem;">Future Camera Soon</p>
    </div>

    <div class="status-pill">
      <div class="stat-group">
        <div class="stat-dot dot-closed" id="sensorDot"></div>
        <span id="sensorLabel">Closed</span>
      </div>
      <div class="stat-group">
        <div class="stat-dot dot-trig-off" id="trigDot"></div>
        <span>Trigger</span>
      </div>
    </div>

    <div class="gate-btn-row">
      <button class="clean-gate-btn" onclick="triggerGate('single')">1 GATE</button>
      <button class="clean-gate-btn" onclick="triggerGate('dual')">2 GATE</button>
    </div>

    <button class="clean-logout-btn" onclick="lockGatePage()">LOG OUT</button>
  </div>

  <!-- ADMIN MODAL -->
  <div class="modal" id="adminModal">
    <div class="modal-box">
      <h4 style="margin:0 0 14px 0; font-weight:400;">Administrator Mode</h4>
      <div id="admLoginBox">
        <input type="text" class="input-clean" id="admU" placeholder="Admin Username">
        <input type="password" class="input-clean" id="admP" placeholder="Admin Password">
        <button class="btn-small btn-save-wifi" style="width:100%" onclick="loginAdmin()">Unlock Settings</button>
      </div>
      <div id="admConfigBox" style="display:none;">
        <span class="section-lbl">Gate Keypad PIN</span>
        <input type="number" class="input-clean" id="newPinInput" placeholder="New 4-6 Digit Gate PIN">
        <span class="section-lbl">Admin Credentials (Optional)</span>
        <input type="text" class="input-clean" id="newAdmU" placeholder="New Admin Username">
        <input type="password" class="input-clean" id="newAdmP" placeholder="New Admin Password">
        <button class="btn-small btn-save-wifi" style="width:100%; margin-top:4px;" onclick="updateSettings()">Save Configuration</button>
      </div>
      <button class="btn-small" style="width:100%; margin-top:8px; background:transparent; color:var(--muted);" onclick="closeAdminModal()">Close</button>
    </div>
  </div>

  <div class="toast" id="toast"></div>

  <script>
    let wifiConnected = false;
    let pin = "";
    let activePin = "";
    let pinTimer = null;
    let barInterval = null;
    let pollTimer = null;
    const TIMEOUT = 15;

    const now = new Date();
    document.getElementById('dateLabel').innerText = now.toLocaleDateString('en-GB', {
      weekday: 'long', day: 'numeric', month: 'long', year: 'numeric'
    });

    function msg(text) {
      const t = document.getElementById('toast');
      t.innerText = text;
      t.style.display = 'block';
      setTimeout(() => t.style.display = 'none', 3500);
    }

    function flashKey(el) {
      if (!el) return;
      el.classList.add('lit');
      setTimeout(() => el.classList.remove('lit'), 180);
    }

    async function checkWifiState() {
      try {
        const res = await fetch('/wifi-status');
        const data = await res.json();
        const dot = document.getElementById('wifiDot');
        const txt = document.getElementById('wifiText');
        if (data.connected) {
          wifiConnected = true;
          dot.classList.add('connected');
          txt.innerText = data.ssid;
        } else {
          wifiConnected = false;
          dot.classList.remove('connected');
          txt.innerText = "Not Connected (Setup)";
        }
      } catch (e) {}
    }
    checkWifiState();

    function toggleWifiMenu() {
      document.getElementById('wifiMenu').classList.toggle('open');
    }

    async function scanNearbyWifi() {
      msg("Scanning 2.4GHz networks nearby...");
      try {
        const res = await fetch('/scan-wifi');
        const networks = await res.json();
        const select = document.getElementById('scannedNetworks');
        select.innerHTML = '<option value="">-- Select detected network --</option>';

        if (networks.length === 0) {
          msg("No 2.4GHz networks detected.");
          return;
        }

        networks.forEach(net => {
          if (net.ssid) {
            const opt = document.createElement('option');
            opt.value = net.ssid;
            opt.innerText = `${net.ssid} (${net.rssi} dBm)`;
            select.appendChild(opt);
          }
        });

        select.style.display = 'block';
        msg(`Found ${networks.length} networks.`);
      } catch (e) {
        msg("Scan request failed.");
      }
    }

    function selectScannedSsid() {
      const chosen = document.getElementById('scannedNetworks').value;
      if (chosen) {
        document.getElementById('ssidInput').value = chosen;
      }
    }

    async function saveWifi() {
      const s = document.getElementById('ssidInput').value.trim();
      const p = document.getElementById('passInput').value;
      if (!s) { msg("Please enter or select an SSID"); return; }

      msg("Saving credentials & rebooting gate...");

      try {
        const res = await fetch('/save-wifi', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ ssid: s, pass: p })
        });

        if (res.ok) {
          alert("SUCCESS!\nCredentials saved.\n\nThe STU-SmartGate hotspot is shutting down.\n\n1. Reconnect your phone to '" + s + "'\n2. Open http://stu-gate.local in your browser.");
        } else {
          msg("Save failed. Try again.");
        }
      } catch (e) {
        alert("Rebooting!\n\nReconnect your phone to '" + s + "' and open:\nhttp://stu-gate.local");
      }
    }

    async function logoutWifi() {
      if (!confirm("Disconnect Wi-Fi and re-broadcast 'STU-SmartGate' setup hotspot?")) return;
      try {
        await fetch('/logout-wifi', { method: 'POST' });
      } catch (e) {}
      wifiConnected = false;
      document.getElementById('wifiDot').classList.remove('connected');
      document.getElementById('wifiText').innerText = "Not Connected";
      document.getElementById('wifiMenu').classList.remove('open');
      lockGatePage();
      msg("Wi-Fi logged out! Reconnect to 'STU-SmartGate' hotspot.");
    }

    function renderPin() {
      const el = document.getElementById('pinReadout');
      if (pin.length === 0) {
        el.innerText = "";
      } else if (pin.length === 1) {
        el.innerText = pin;
      } else {
        el.innerText = "*".repeat(pin.length - 1) + pin.slice(-1);
      }
    }

    function start15sCountdown() {
      clearTimeout(pinTimer);
      clearInterval(barInterval);
      const bar = document.getElementById('timerBar');
      bar.style.width = '100%';
      let left = TIMEOUT;

      barInterval = setInterval(() => {
        left -= 0.1;
        bar.style.width = ((left / TIMEOUT) * 100) + '%';
        if (left <= 0) clearInterval(barInterval);
      }, 100);

      pinTimer = setTimeout(() => {
        clearPin();
        msg("15s timeout expired. Locked.");
      }, TIMEOUT * 1000);
    }

    function pressDigit(d, btnEl) {
      flashKey(btnEl);
      if (!wifiConnected) {
        msg("Please connect to an SSID first before entering PIN!");
        document.getElementById('wifiMenu').classList.add('open');
        return;
      }
      if (pin.length < 6) {
        pin += d;
        renderPin();
        start15sCountdown();
      }
    }

    function clearPin(btnEl) {
      if (btnEl) flashKey(btnEl);
      pin = "";
      renderPin();
      clearTimeout(pinTimer);
      clearInterval(barInterval);
      document.getElementById('timerBar').style.width = '0%';
    }

    async function submitPin(btnEl) {
      if (btnEl) flashKey(btnEl);
      if (!wifiConnected) {
        msg("Please connect to an SSID first!");
        document.getElementById('wifiMenu').classList.add('open');
        return;
      }
      if (!pin) return;

      const res = await fetch('/verify-pin', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'pin=' + encodeURIComponent(pin)
      });

      if (res.ok) {
        activePin = pin;
        clearPin();
        document.getElementById('page1').style.display = 'none';
        document.getElementById('page2').style.display = 'flex';
        pollSensor();
        pollTimer = setInterval(pollSensor, 1000);
      } else {
        msg("Wrong Passcode");
        clearPin();
      }
    }

    async function triggerGate(type) {
      const trig = document.getElementById('trigDot');
      trig.className = "stat-dot dot-trig-on";

      await fetch('/trigger', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: `gate=${type}&pin=${encodeURIComponent(activePin)}`
      });

      setTimeout(() => {
        trig.className = "stat-dot dot-trig-off";
        pollSensor();
      }, 700);
    }

    async function pollSensor() {
      try {
        const res = await fetch('/status');
        const data = await res.json();
        const dot = document.getElementById('sensorDot');
        const lbl = document.getElementById('sensorLabel');
        if (data.closed) {
          dot.className = "stat-dot dot-closed";
          lbl.innerText = "Closed";
        } else {
          dot.className = "stat-dot dot-open";
          lbl.innerText = "Open";
        }
      } catch (e) {}
    }

    function lockGatePage() {
      activePin = "";
      clearInterval(pollTimer);
      document.getElementById('page2').style.display = 'none';
      document.getElementById('page1').style.display = 'flex';
    }

    function openAdminModal() {
      if (!wifiConnected) {
        msg("Please connect to an SSID first!");
        document.getElementById('wifiMenu').classList.add('open');
        return;
      }
      document.getElementById('adminModal').style.display = 'flex';
    }
    function closeAdminModal() {
      document.getElementById('adminModal').style.display = 'none';
      document.getElementById('admLoginBox').style.display = 'block';
      document.getElementById('admConfigBox').style.display = 'none';
    }
    async function loginAdmin() {
      const u = document.getElementById('admU').value;
      const p = document.getElementById('admP').value;
      const r = await fetch('/admin-login', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: `user=${encodeURIComponent(u)}&pass=${encodeURIComponent(p)}`
      });
      if (r.ok) {
        document.getElementById('admLoginBox').style.display = 'none';
        document.getElementById('admConfigBox').style.display = 'block';
      } else {
        msg("Invalid Admin Credentials");
      }
    }
    async function updateSettings() {
      const u = document.getElementById('admU').value;
      const p = document.getElementById('admP').value;
      const np = document.getElementById('newPinInput').value.trim();
      const nu = document.getElementById('newAdmU').value.trim();
      const npass = document.getElementById('newAdmP').value.trim();

      if (np && np.length < 4) { msg("Gate PIN must be at least 4 digits"); return; }

      const r = await fetch('/set-config', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: `user=${encodeURIComponent(u)}&pass=${encodeURIComponent(p)}&newpin=${encodeURIComponent(np)}&newuser=${encodeURIComponent(nu)}&newpass=${encodeURIComponent(npass)}`
      });
      if (r.ok) {
        msg("Configuration Saved!");
        closeAdminModal();
      } else {
        msg("Failed to save settings");
      }
    }
  </script>
</body>
</html>
)rawliteral";

#endif