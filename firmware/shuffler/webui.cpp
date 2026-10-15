// webui.cpp — see webui.h
#include "webui.h"
#if DEALER
#include <WiFi.h>
#include <WebServer.h>
#include "dealer.h"
#include "hardware.h"

// shuffler.ino
bool app_busy();
const char* app_state_name();
const char* app_message();
void app_request_stop();

static WebServer server(80);
static bool s_rf = false;
static WebRequest s_pending = { WC_NONE, 0, 0 };
bool web_rf_on() { return s_rf; }
bool web_take(WebRequest* r) { if (s_pending.cmd == WC_NONE) return false; *r = s_pending; s_pending.cmd = WC_NONE; return true; }

static const char PAGE[] PROGMEM = R"HTML(<!doctype html><html lang=en><head><meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1"><title>Shuffler</title>
<style>
:root{--bg:#f6f7f9;--fg:#15181d;--mut:#5b6472;--card:#fff;--line:#d9dde3;--acc:#1f6feb;--warn:#b42318}
@media(prefers-color-scheme:dark){:root{--bg:#111418;--fg:#e8eaed;--mut:#9aa3ae;--card:#1a1f25;--line:#2c333b;--acc:#4c8dff;--warn:#ff6b5e}}
*{box-sizing:border-box}body{margin:0;font:15px/1.4 system-ui,sans-serif;background:var(--bg);color:var(--fg)}
main{max-width:560px;margin:auto;padding:12px 16px 40px}h1{font-size:20px;margin:6px 0 2px}
h2{font-size:13px;text-transform:uppercase;letter-spacing:.06em;color:var(--mut);margin:20px 0 6px}
.card{background:var(--card);border:1px solid var(--line);border-radius:12px;padding:12px;margin:8px 0}
#st{font-size:14px;color:var(--mut)}#msg{margin-top:4px;min-height:1.4em}
button{font:inherit;border:1px solid var(--line);background:var(--card);color:var(--fg);border-radius:9px;padding:9px 12px;margin:3px 2px}
button:disabled{opacity:.45}button.p{background:var(--acc);border-color:var(--acc);color:#fff;font-weight:600;padding:13px;width:100%}
button.s{background:var(--warn);border-color:var(--warn);color:#fff}.row{display:flex;gap:6px;align-items:center;flex-wrap:wrap}
.g{display:grid;grid-template-columns:1fr 1fr;gap:8px}label{color:var(--mut);font-size:13px}
input,select{font:inherit;background:var(--bg);color:var(--fg);border:1px solid var(--line);border-radius:8px;padding:6px}
select{width:100%}input[type=number]{width:70px}input[type=range]{width:100%}
table{width:100%;border-collapse:collapse}td{padding:6px 2px;border-top:1px solid var(--line)}.sm button{padding:5px 8px;margin:1px}
</style></head><body><main>
<h1>Shuffler</h1><div id=st>connecting…</div><div id=msg></div>
<div class=card><button class=p onclick="cmd('deal')">Shuffle &amp; deal</button>
<div class=row><button id=bs onclick="cmd('street')">Next street</button><button onclick="cmd('finish')">Finish hand</button>
<button onclick="cmd('shuffle')">Shuffle only</button><button class=s onclick="cmd('stop')">Stop</button></div></div>
<h2>Game</h2><div class=card><div class=g>
<label>Seats round the table<select id=n></select></label>
<label>Cards each<select id=cards><option>1<option>2<option>3<option>4<option>5<option>6<option>7</select></label>
<label>After the hole cards<select id=board><option value=1>Keep the deck: deal each street on request<option value=0>Give the dealer the rest of the deck</select></label>
<label>Dealer button<select id=btn></select></label></div>
<div class=row style="margin-top:8px"><label><input type=checkbox id=auto> Move the button after each hand</label></div></div>
<h2>Seats</h2><div class=card><table id=seats></table>
<div class=row style="margin-top:8px"><label>Spread seats from <input type=number id=sa value=50> to <input type=number id=sb value=310> °</label><button onclick="spread()">Spread</button></div></div>
<h2>Turntable</h2><div class=card><div id=tt></div>
<div class="row sm"><button onclick="cmd('jog',-15)">−15°</button><button onclick="cmd('jog',-5)">−5°</button><button onclick="cmd('jog',-1)">−1°</button>
<button onclick="cmd('jog',1)">+1°</button><button onclick="cmd('jog',5)">+5°</button><button onclick="cmd('jog',15)">+15°</button></div>
<div class=row><button onclick="cmd('home')">Home</button><button onclick="cmd('aimdealer')">Aim at dealer</button><button onclick="cmd('setdealer')">Dealer is here</button></div></div>
<h2>Wi-Fi</h2><div class=card><div class=row><label>New password <input id=pw minlength=8 maxlength=23></label><button onclick="pw()">Save</button></div>
<label>Used after the next power-up.</label></div>
<button class=p onclick="save()">Save settings</button>
</main><script>
let S=null,dirty=false;const $=i=>document.getElementById(i);
const q=o=>Object.entries(o).map(([k,v])=>k+'='+encodeURIComponent(v)).join('&');
async function post(u,o){try{const r=await fetch(u,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:q(o)});show(await r.json(),true)}catch(e){$('msg').textContent='not connected'}}
function cmd(c,v,s){post('/api/cmd',{c:c,v:v??0,s:s??0})}
function form(){const c=S.cfg;
$('n').innerHTML=[...Array(9)].map((_,i)=>`<option ${i+2==c.n?'selected':''}>${i+2}</option>`).join('');
$('btn').innerHTML=[...Array(c.n)].map((_,i)=>`<option value=${i} ${i==c.btn?'selected':''}>seat ${i+1}</option>`).join('');
$('cards').value=c.cards;$('board').value=c.board;$('auto').checked=!!c.auto;
let h='<tr><td><label>Player</label></td><td><label>Angle</label></td><td><label>Throw</label></td><td></td></tr>';
for(let k=0;k<c.n;k++)h+=`<tr><td><label><input type=checkbox class=on data-k=${k} ${c.on>>k&1?'checked':''}> ${k+1}</label></td>
<td><input type=number step=0.5 class=deg data-k=${k} value=${c.deg[k]}></td><td><input type=range min=60 max=255 class=pwm data-k=${k} value=${c.pwm[k]}></td>
<td class=sm><button onclick="cmd('aim',0,${k})">Aim</button><button onclick="cmd('setseat',0,${k})">Here</button><button onclick="cmd('test',0,${k})">Test</button></td></tr>`;
$('seats').innerHTML=h;document.querySelectorAll('#seats input,#cards,#board,#btn,#auto').forEach(e=>e.oninput=()=>dirty=true)}
function show(j,force){if(!j)return;S=j;
$('st').textContent=`${j.busy?'busy':j.state} · ${j.stage} · ${j.left} cards in the wheel · battery ${j.bat.toFixed(2)} V`;
$('msg').textContent=j.msg||'';$('msg').style.color=j.state=='error'?'var(--warn)':'';
$('tt').textContent=j.homed?`Pointing at ${j.tt.toFixed(1)}° · dealer at ${j.cfg.dealer}°`:'Turntable not homed yet';
const nx={'hole cards dealt':'flop','flop out':'turn','turn out':'river','river out':'rest of the deck'}[j.stage];
$('bs').textContent=nx?'Deal the '+nx:'Next street';$('bs').disabled=!nx;if(force||!dirty)form()}
function save(){let on=0,o={n:$('n').value,cards:$('cards').value,board:$('board').value,btn:$('btn').value,auto:$('auto').checked?1:0};
document.querySelectorAll('.on').forEach(e=>{if(e.checked)on|=1<<e.dataset.k});o.on=on;
document.querySelectorAll('.deg').forEach(e=>o['d'+e.dataset.k]=e.value);document.querySelectorAll('.pwm').forEach(e=>o['p'+e.dataset.k]=e.value);
dirty=false;post('/api/cfg',o)}
$('n').onchange=()=>{dirty=false;post('/api/cfg',{n:$('n').value})};
function spread(){dirty=false;post('/api/cfg',{sa:$('sa').value,sb:$('sb').value})}
function pw(){const p=$('pw').value;if(p.length<8||p.length>23){alert('8 to 23 characters');return}post('/api/cfg',{pass:p})}
async function tick(){try{const r=await fetch('/api/state');show(await r.json())}catch(e){$('st').textContent='not connected'}}
tick();setInterval(tick,1000);
</script></body></html>)HTML";

static void clean(char* s) { for (; *s; s++) if (*s == '"' || *s == '\\' || (unsigned char)*s < 32) *s = '\''; }
static void send_state() {
    static char b[1400]; char msg[128];
    strncpy(msg, app_message(), sizeof msg - 1); msg[sizeof msg - 1] = 0; clean(msg);
    int n = snprintf(b, sizeof b,
        "{\"state\":\"%s\",\"busy\":%s,\"stage\":\"%s\",\"left\":%d,\"bat\":%.2f,\"tt\":%.1f,\"homed\":%s,\"msg\":\"%s\","
        "\"cfg\":{\"n\":%u,\"on\":%u,\"cards\":%u,\"board\":%u,\"btn\":%u,\"auto\":%u,\"dealer\":%.1f,\"deg\":[",
        app_state_name(), app_busy() ? "true" : "false", hand_stage_name(hand_stage()), hand_cards_left(), battery_volts(),
        tt_angle(), tt_homed() ? "true" : "false", msg, dcfg.n_seats, dcfg.seat_on, dcfg.cards_each, dcfg.board_mode,
        dcfg.button, dcfg.auto_button, dcfg.dealer_deg);
    for (int k = 0; k < MAX_SEATS; k++) n += snprintf(b + n, sizeof b - n, "%s%.1f", k ? "," : "", dcfg.seat_deg[k]);
    n += snprintf(b + n, sizeof b - n, "],\"pwm\":[");
    for (int k = 0; k < MAX_SEATS; k++) n += snprintf(b + n, sizeof b - n, "%s%u", k ? "," : "", dcfg.seat_pwm[k]);
    snprintf(b + n, sizeof b - n, "]}}");
    server.send(200, "application/json", String(b));
}
static float argf(const char* k, float dflt) { return server.hasArg(k) ? server.arg(k).toFloat() : dflt; }
static int   argi(const char* k, int dflt)   { return server.hasArg(k) ? server.arg(k).toInt() : dflt; }

static void h_root() { server.send_P(200, "text/html", PAGE); }
static void h_state() { send_state(); }
static void h_cfg() {
    if (app_busy()) { send_state(); return; }                   // never change seats under a deal in progress
    DealConfig& c = dcfg;
    c.n_seats     = (uint8_t)constrain(argi("n", c.n_seats), 2, MAX_SEATS);
    c.cards_each  = (uint8_t)constrain(argi("cards", c.cards_each), 1, 7);
    c.board_mode  = argi("board", c.board_mode) ? 1 : 0;
    c.auto_button = argi("auto", c.auto_button) ? 1 : 0;
    c.button      = (uint8_t)constrain(argi("btn", c.button), 0, c.n_seats - 1);
    c.seat_on     = (uint16_t)(argi("on", c.seat_on) & ((1 << c.n_seats) - 1));
    char k[4];
    for (int i = 0; i < MAX_SEATS; i++) {
        snprintf(k, sizeof k, "d%d", i); c.seat_deg[i] = argf(k, c.seat_deg[i]);
        snprintf(k, sizeof k, "p%d", i); c.seat_pwm[i] = (uint8_t)constrain(argi(k, c.seat_pwm[i]), 0, 255);
    }
    if (server.hasArg("sa") && server.hasArg("sb")) deal_spread_seats(c, argf("sa", 50), argf("sb", 310));
    if (server.hasArg("pass")) { String p = server.arg("pass");
        if (p.length() >= 8 && p.length() < sizeof c.ap_pass) { strncpy(c.ap_pass, p.c_str(), sizeof c.ap_pass - 1); c.ap_pass[sizeof c.ap_pass - 1] = 0; } }
    deal_save(c);
    send_state();
}
static void h_cmd() {
    String c = server.arg("c");
    if (c == "stop") { app_request_stop(); deal_request_abort(); send_state(); return; }
    if (app_busy() || s_pending.cmd != WC_NONE) { send_state(); return; }   // the page shows "busy"
    WebRequest r = { WC_NONE, constrain(argi("s", 0), 0, MAX_SEATS - 1), argf("v", 0) };
    if      (c == "deal")      r.cmd = WC_DEAL;
    else if (c == "shuffle")   r.cmd = WC_SHUFFLE;
    else if (c == "street")    r.cmd = WC_STREET;
    else if (c == "finish")    r.cmd = WC_FINISH;
    else if (c == "unload")    r.cmd = WC_UNLOAD;
    else if (c == "home")      r.cmd = WC_TT_HOME;
    else if (c == "jog")       r.cmd = WC_JOG;
    else if (c == "aim")       r.cmd = WC_AIM_SEAT;
    else if (c == "aimdealer") r.cmd = WC_AIM_DEALER;
    else if (c == "setseat")   r.cmd = WC_SET_SEAT;
    else if (c == "setdealer") r.cmd = WC_SET_DEALER;
    else if (c == "test")      r.cmd = WC_TEST;
    s_pending = r;
    send_state();
}

void web_init() {
#if WIFI_ENABLE
    char ssid[24]; uint64_t mac = ESP.getEfuseMac();
    snprintf(ssid, sizeof ssid, "Shuffler-%04X", (unsigned)((mac >> 32) & 0xFFFF));
    WiFi.mode(WIFI_AP);
    s_rf = WiFi.softAP(ssid, strlen(dcfg.ap_pass) >= 8 ? dcfg.ap_pass : WIFI_AP_PASS_DEFAULT);
    server.on("/", HTTP_GET, h_root);
    server.on("/api/state", HTTP_GET, h_state);
    server.on("/api/cfg", HTTP_POST, h_cfg);
    server.on("/api/cmd", HTTP_POST, h_cmd);
    server.onNotFound(h_root);
    server.begin();
    Serial.printf("# Wi-Fi %s: join %s, open http://%s\n", s_rf ? "up" : "FAILED", ssid, WiFi.softAPIP().toString().c_str());
#endif
}
void web_poll() {
#if WIFI_ENABLE
    server.handleClient();
#endif
}
#endif
