String getHTML() {
  int waterPercent = getWaterPercent();   // Using same helper function

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<meta http-equiv="refresh" content="5">
<title>FloraGuard • Plant Monitor</title>
<style>
:root{--p:#166534;--a:#4ade80;--t:#1a2e1f;--m:#64705f;--g:rgba(255,255,255,0.88);--r:24px;--d:#e11d48;}
*{margin:0;padding:0;box-sizing:border-box;}
body{font-family:system-ui,sans-serif;background:linear-gradient(145deg,#f0f7f0,#e8f5eb);color:var(--t);padding:20px;}
.container{max-width:1100px;margin:auto;}
.header{text-align:center;margin-bottom:28px;}
.header h1{font-size:2.6rem;font-weight:700;background:linear-gradient(90deg,var(--p),#22c55e);-webkit-background-clip:text;-webkit-text-fill-color:transparent;}
.dashboard{display:grid;grid-template-columns:2fr 1fr;gap:20px;}
.main-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:16px;}
.glass{background:var(--g);border-radius:var(--r);padding:20px;box-shadow:0 8px 25px rgba(0,0,0,0.09);border:1px solid rgba(255,255,255,0.7);}
.metric{display:flex;align-items:center;gap:12px;}
.icon{font-size:2.3rem;}
.val{font-size:2.35rem;font-weight:700;}
.label{font-size:0.93rem;color:var(--m);}
.water-tank .tank{width:128px;height:205px;margin:18px auto;background:#f0f9ff;border:8px solid #0ea5e9;border-radius:12px;position:relative;overflow:hidden;}
.water-level{position:absolute;bottom:0;width:100%;background:linear-gradient(#67e8f9,#22d3ee);transition:height .9s;}
.percent{font-size:2.9rem;font-weight:800;color:#0ea5e9;}
.alerts .item{display:flex;justify-content:space-between;padding:13px 0;border-bottom:1px solid #eee;}
.alerts .item:last-child{border:none;}
.badge{padding:6px 15px;border-radius:30px;font-size:0.84rem;font-weight:600;}
.good{background:#dcfce7;color:#166534;}
.bad{background:#fee2e2;color:var(--d);}
.light h3{color:var(--p);}
#bright{font-size:2.05rem;font-weight:700;color:var(--p);margin:8px 0;}
input[type=range]{width:100%;height:13px;border-radius:999px;background:linear-gradient(to right,#14532d,#4ade80);outline:none;}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:32px;height:32px;background:#fff;border:4px solid var(--p);border-radius:50%;box-shadow:0 0 0 7px rgba(74,222,128,0.35);}
.toggle{padding:13px 32px;margin:15px 0;font-weight:600;background:linear-gradient(90deg,var(--p),#22c55e);color:#fff;border:none;border-radius:50px;cursor:pointer;}
.footer{text-align:center;margin-top:35px;color:var(--m);font-size:0.92rem;}
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <h1>FloraGuard</h1>
    <p>Intelligent Indoor Plant Monitoring</p>
  </div>

  <div class="dashboard">
    <div class="main-grid">
      <div class="glass metric"><div class="icon">🌱</div><div><div class="label">Soil Moisture 1</div><div class="val">)rawliteral";
  html += String(soil1);
  html += R"rawliteral(</div></div></div>
      <div class="glass metric"><div class="icon">🌿</div><div><div class="label">Soil Moisture 2</div><div class="val">)rawliteral";
  html += String(soil2);
  html += R"rawliteral(</div></div></div>
      <div class="glass metric"><div class="icon">🌡️</div><div><div class="label">Temperature</div><div class="val">)rawliteral";
  html += String(tempValue, 1);
  html += R"rawliteral(°C</div></div></div>
      <div class="glass metric"><div class="icon">💧</div><div><div class="label">Humidity</div><div class="val">)rawliteral";
  html += String(humValue, 1);
  html += R"rawliteral(%</div></div></div>
      <div class="glass metric"><div class="icon">🌬️</div><div><div class="label">CO₂ Level</div><div class="val" style="color:#7c3aed;">)rawliteral";
  html += String(co2Value);
  html += R"rawliteral( ppm</div></div></div>
    </div>

    <div class="water-tank glass">
      <div class="label">Water Reservoir</div>
      <div class="tank"><div class="water-level" style="height:)rawliteral";
  html += String(waterPercent);
  html += R"rawliteral(%"></div></div>
      <div class="percent">)rawliteral";
  html += String(waterPercent);
  html += R"rawliteral(%</div>
      <div style="color:var(--m);font-size:0.9rem;">Raw: )rawliteral";
  html += String(waterValue);
  html += R"rawliteral(</div>
    </div>
  </div>

  <div style="display:grid;grid-template-columns:1fr 1fr;gap:20px;margin-top:22px;">
    <div class="alerts glass">
      <h3 style="margin-bottom:14px;color:var(--p);">System Alerts</h3>
      <div class="item"><span>Soil Sensor 1</span>)rawliteral";
  html += badge(soilAlert1, "Normal", "Low Moisture");
  html += R"rawliteral(</div>
      <div class="item"><span>Soil Sensor 2</span>)rawliteral";
  html += badge(soilAlert2, "Normal", "Low Moisture");
  html += R"rawliteral(</div>
      <div class="item"><span>Water Level</span>)rawliteral";
  html += badge(waterAlert, "Normal", "Low");
  html += R"rawliteral(</div>
      <div class="item"><span>Temperature</span>)rawliteral";
  html += badge(tempAlert, "Normal", "High");
  html += R"rawliteral(</div>
      <div class="item"><span>Humidity</span>)rawliteral";
  html += badge(humAlert, "Normal", "High");
  html += R"rawliteral(</div>
      <div class="item" style="font-weight:600;"><span>Overall Health</span>)rawliteral";
  html += badge(anyAlert, "Healthy", "Critical");
  html += R"rawliteral(</div>
    </div>

    <div class="light glass">
      <h3>Grow Light</h3>
      <div id="bright">)rawliteral";
  html += String(ledBrightness);
  html += R"rawliteral(</div>
      <div style="color:var(--m);margin-bottom:10px;">0 – 255</div>
      <button onclick="tog()" class="toggle">Toggle ON / OFF</button>
      <input type="range" min="0" max="255" value=")rawliteral";
  html += String(ledBrightness);
  html += R"rawliteral(" oninput="upd(this.value)">
    </div>
  </div>

  <div class="footer">FloraGuard • Auto refresh every 5s • plant.local</div>
</div>

<script>
function upd(v){document.getElementById('bright').textContent=v;var x=new XMLHttpRequest();x.open("GET","/setLED?value="+v,true);x.send();}
function tog(){var c=parseInt(document.getElementById('bright').textContent);var n=(c>30)?0:255;document.getElementById('bright').textContent=n;var x=new XMLHttpRequest();x.open("GET","/setLED?value="+n,true);x.send();}
</script>
</body>
</html>
)rawliteral";

  return html;
}