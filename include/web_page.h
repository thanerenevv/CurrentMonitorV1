#pragma once

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Power Monitor</title>
<style>
  @import url('https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Barlow:wght@300;600;800&display=swap');

  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

  :root {
    --bg:       #0a0c0f;
    --surface:  #111418;
    --border:   #1e2530;
    --accent:   #00e5ff;
    --accent2:  #ff6d00;
    --danger:   #ff1744;
    --ok:       #00e676;
    --text:     #e8eaf0;
    --muted:    #4a5568;
    --mono:     'Share Tech Mono', monospace;
    --sans:     'Barlow', sans-serif;
  }

  html, body {
    height: 100%;
    background: var(--bg);
    color: var(--text);
    font-family: var(--sans);
    font-weight: 300;
    letter-spacing: 0.01em;
  }

  body {
    display: flex;
    flex-direction: column;
    min-height: 100vh;
  }

  header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 20px 32px;
    border-bottom: 1px solid var(--border);
    background: var(--surface);
  }

  .logo {
    font-family: var(--mono);
    font-size: 0.75rem;
    color: var(--accent);
    letter-spacing: 0.2em;
    text-transform: uppercase;
  }

  .status-pill {
    display: flex;
    align-items: center;
    gap: 8px;
    font-family: var(--mono);
    font-size: 0.7rem;
    color: var(--muted);
    letter-spacing: 0.12em;
  }

  .dot {
    width: 7px;
    height: 7px;
    border-radius: 50%;
    background: var(--danger);
    transition: background 0.4s;
  }
  .dot.live { background: var(--ok); animation: pulse 2s infinite; }

  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.35; }
  }

  main {
    flex: 1;
    padding: 32px;
    display: grid;
    gap: 20px;
    grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
    align-content: start;
    max-width: 1280px;
    margin: 0 auto;
    width: 100%;
  }

  .card {
    background: var(--surface);
    border: 1px solid var(--border);
    border-radius: 4px;
    padding: 28px 32px;
    display: flex;
    flex-direction: column;
    gap: 8px;
    position: relative;
    overflow: hidden;
    transition: border-color 0.3s;
  }

  .card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0;
    height: 2px;
    background: var(--accent);
    opacity: 0.6;
  }

  .card.warn::before  { background: var(--accent2); }
  .card.alert::before { background: var(--danger); }

  .card-label {
    font-family: var(--mono);
    font-size: 0.65rem;
    letter-spacing: 0.22em;
    color: var(--muted);
    text-transform: uppercase;
  }

  .card-value {
    font-family: var(--mono);
    font-size: 3.2rem;
    font-weight: 400;
    line-height: 1;
    color: var(--text);
    transition: color 0.3s;
  }

  .card-value span {
    font-size: 1.1rem;
    color: var(--muted);
    margin-left: 4px;
    vertical-align: baseline;
  }

  .card-sub {
    font-family: var(--mono);
    font-size: 0.72rem;
    color: var(--muted);
    margin-top: 4px;
  }

  .card-sub b {
    color: var(--accent);
    font-weight: 400;
  }

  .wide {
    grid-column: 1 / -1;
  }

  .chart-wrap {
    width: 100%;
    height: 160px;
    position: relative;
    margin-top: 12px;
  }

  canvas#chart {
    width: 100% !important;
    height: 100% !important;
  }

  .gauge-row {
    display: flex;
    gap: 16px;
    align-items: center;
    margin-top: 8px;
  }

  .bar-outer {
    flex: 1;
    height: 6px;
    background: var(--border);
    border-radius: 3px;
    overflow: hidden;
  }

  .bar-inner {
    height: 100%;
    background: var(--accent);
    border-radius: 3px;
    transition: width 0.6s cubic-bezier(.25,.8,.25,1), background 0.4s;
    width: 0%;
  }

  .bar-pct {
    font-family: var(--mono);
    font-size: 0.7rem;
    color: var(--muted);
    min-width: 36px;
    text-align: right;
  }

  footer {
    padding: 16px 32px;
    border-top: 1px solid var(--border);
    font-family: var(--mono);
    font-size: 0.63rem;
    color: var(--muted);
    letter-spacing: 0.1em;
    display: flex;
    justify-content: space-between;
  }

  @media (max-width: 600px) {
    main { padding: 16px; gap: 14px; }
    .card { padding: 20px; }
    .card-value { font-size: 2.4rem; }
    header { padding: 14px 16px; }
    footer { flex-direction: column; gap: 4px; }
  }
</style>
</head>
<body>

<header>
  <div class="logo">&#9632; POWER MONITOR // ESP32-C3</div>
  <div class="status-pill">
    <div class="dot" id="dot"></div>
    <span id="conn-label">CONNECTING</span>
  </div>
</header>

<main>
  <div class="card" id="card-current">
    <div class="card-label">Current</div>
    <div class="card-value" id="val-current">--<span>A</span></div>
    <div class="card-sub">RMS &mdash; Peak: <b id="val-peak">--</b> A</div>
    <div class="gauge-row">
      <div class="bar-outer"><div class="bar-inner" id="bar-current"></div></div>
      <div class="bar-pct" id="pct-current">--%</div>
    </div>
  </div>

  <div class="card" id="card-power">
    <div class="card-label">Active Power</div>
    <div class="card-value" id="val-power">--<span>W</span></div>
    <div class="card-sub">Apparent: <b id="val-apparent">--</b> VA &nbsp;&bull;&nbsp; PF: <b id="val-pf">--</b></div>
  </div>

  <div class="card" id="card-energy">
    <div class="card-label">Energy</div>
    <div class="card-value" id="val-energy">--<span>kWh</span></div>
    <div class="card-sub">Session uptime: <b id="val-uptime">--</b></div>
  </div>

  <div class="card wide">
    <div class="card-label">Current history &mdash; last 60 samples</div>
    <div class="chart-wrap">
      <canvas id="chart"></canvas>
    </div>
  </div>
</main>

<footer>
  <span>ACS712-30A &bull; 230V AC &bull; Max 30A</span>
  <span id="ts">--</span>
</footer>

<script>
(function () {
  const VOLTAGE = 230;
  const MAX_CURRENT = 30;
  const HISTORY = 60;

  const $ = id => document.getElementById(id);
  const dot = $('dot');
  const connLabel = $('conn-label');

  const vals = {
    current:  $('val-current'),
    peak:     $('val-peak'),
    power:    $('val-power'),
    apparent: $('val-apparent'),
    pf:       $('val-pf'),
    energy:   $('val-energy'),
    uptime:   $('val-uptime'),
  };
  const barCurrent = $('bar-current');
  const pctCurrent = $('pct-current');
  const ts = $('ts');

  const canvas = $('chart');
  const ctx = canvas.getContext('2d');
  const history = new Array(HISTORY).fill(null);

  function resizeCanvas() {
    const wrap = canvas.parentElement;
    canvas.width = wrap.clientWidth * window.devicePixelRatio;
    canvas.height = wrap.clientHeight * window.devicePixelRatio;
    ctx.scale(window.devicePixelRatio, window.devicePixelRatio);
  }
  resizeCanvas();
  window.addEventListener('resize', () => { resizeCanvas(); drawChart(); });

  function drawChart() {
    const W = canvas.width / window.devicePixelRatio;
    const H = canvas.height / window.devicePixelRatio;
    ctx.clearRect(0, 0, W, H);

    const valid = history.filter(v => v !== null);
    if (valid.length < 2) return;

    const max = Math.max(...valid, 1);
    const step = W / (HISTORY - 1);

    ctx.strokeStyle = 'rgba(0,229,255,0.08)';
    ctx.lineWidth = 1;
    const gridLines = 4;
    for (let i = 0; i <= gridLines; i++) {
      const y = H - (i / gridLines) * H;
      ctx.beginPath();
      ctx.moveTo(0, y);
      ctx.lineTo(W, y);
      ctx.stroke();
    }

    const grad = ctx.createLinearGradient(0, 0, 0, H);
    grad.addColorStop(0, 'rgba(0,229,255,0.18)');
    grad.addColorStop(1, 'rgba(0,229,255,0)');

    ctx.beginPath();
    let started = false;
    for (let i = 0; i < HISTORY; i++) {
      const v = history[i];
      if (v === null) continue;
      const x = i * step;
      const y = H - (v / max) * (H - 8) - 4;
      if (!started) { ctx.moveTo(x, y); started = true; }
      else ctx.lineTo(x, y);
    }
    const lastIdx = history.reduceRight((acc, v, i) => acc === -1 && v !== null ? i : acc, -1);
    if (lastIdx >= 0) {
      const lx = lastIdx * step;
      ctx.lineTo(lx, H);
      ctx.lineTo(0, H);
      ctx.closePath();
      ctx.fillStyle = grad;
      ctx.fill();
    }

    ctx.beginPath();
    started = false;
    for (let i = 0; i < HISTORY; i++) {
      const v = history[i];
      if (v === null) continue;
      const x = i * step;
      const y = H - (v / max) * (H - 8) - 4;
      if (!started) { ctx.moveTo(x, y); started = true; }
      else ctx.lineTo(x, y);
    }
    ctx.strokeStyle = '#00e5ff';
    ctx.lineWidth = 1.5;
    ctx.lineJoin = 'round';
    ctx.stroke();
  }

  function setCardState(card, value, max, warnPct, alertPct) {
    const pct = value / max;
    card.classList.remove('warn', 'alert');
    if (pct >= alertPct) card.classList.add('alert');
    else if (pct >= warnPct) card.classList.add('warn');
  }

  function fmt(n, dec) { return isNaN(n) ? '--' : n.toFixed(dec); }

  function formatUptime(sec) {
    const h = Math.floor(sec / 3600);
    const m = Math.floor((sec % 3600) / 60);
    const s = sec % 60;
    return (h > 0 ? h + 'h ' : '') + (m > 0 || h > 0 ? m + 'm ' : '') + s + 's';
  }

  function applyData(d) {
    const current = parseFloat(d.current_rms);
    const peak    = parseFloat(d.current_peak);
    const power   = current * VOLTAGE;
    const apparent = peak * VOLTAGE * 0.707;
    const pf      = apparent > 0 ? Math.min(power / apparent, 1) : 0;
    const energy  = parseFloat(d.energy_kwh);
    const uptime  = parseInt(d.uptime_sec);

    vals.current.innerHTML  = fmt(current, 2) + '<span>A</span>';
    vals.peak.textContent   = fmt(peak, 2);
    vals.power.innerHTML    = fmt(power, 1) + '<span>W</span>';
    vals.apparent.textContent = fmt(apparent, 1);
    vals.pf.textContent     = fmt(pf, 3);
    vals.energy.innerHTML   = fmt(energy, 4) + '<span>kWh</span>';
    vals.uptime.textContent = formatUptime(uptime);

    const pct = Math.min((current / MAX_CURRENT) * 100, 100);
    barCurrent.style.width = pct + '%';
    pctCurrent.textContent = fmt(pct, 0) + '%';

    if (pct >= 90) barCurrent.style.background = 'var(--danger)';
    else if (pct >= 70) barCurrent.style.background = 'var(--accent2)';
    else barCurrent.style.background = 'var(--accent)';

    setCardState($('card-current'), current, MAX_CURRENT, 0.7, 0.9);
    setCardState($('card-power'), power, MAX_CURRENT * VOLTAGE, 0.7, 0.9);

    history.push(current);
    if (history.length > HISTORY) history.shift();
    drawChart();

    ts.textContent = new Date().toLocaleTimeString('en-US', { hour12: false });
  }

  function setOnline(online) {
    dot.classList.toggle('live', online);
    connLabel.textContent = online ? 'LIVE' : 'RECONNECTING';
  }

  let retryDelay = 1000;

  function poll() {
    fetch('/data')
      .then(r => { if (!r.ok) throw new Error(); return r.json(); })
      .then(d => {
        setOnline(true);
        applyData(d);
        retryDelay = 1000;
        setTimeout(poll, 1000);
      })
      .catch(() => {
        setOnline(false);
        retryDelay = Math.min(retryDelay * 1.5, 10000);
        setTimeout(poll, retryDelay);
      });
  }

  poll();
})();
</script>
</body>
</html>
)=====";