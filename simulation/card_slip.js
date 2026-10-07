#!/usr/bin/env node
// card_slip.js — radial slip of every card in the wheel through whole shuffles, and what reaches the entry nip.
//
// Each seated card can slide along its slot: gravity, the centrifugal and Euler terms of the wheel's real motion
// (the firmware's stepper ramp, from the core in wheel_sim.html) and Coulomb friction on the fin it leans on.
// Stops: the hub (seated), the shroud (r 88.5) over 172°..368°, the closed shutter blade (r 87.0), and the
// optional re-seating ramp on shroud_B (cad/params.scad: ramp_*).  A card that rode the shroud in the lower half
// keeps its 3 mm of play until gravity pulls it back; the entry nip roller reaches in to r 88.3 at 42.2°.
//
//   node simulation/card_slip.js [shuffles per case, default 12]
//
// Prints, for each friction coefficient, how often a counter-clockwise move carries a card into the roller,
// without and with the ramp.  Geometry from cad/params.scad; card speeds and timing as in wheel_sim.html.
"use strict";
const fs = require("fs"), path = require("path"), crypto = require("crypto");

// ---- load the simulation core (RNG, firmware sequence, stepper profile) from the page
const html = fs.readFileSync(path.join(__dirname, "wheel_sim.html"), "utf8");
const start = html.indexOf("const SIM = (() => {"), end = html.indexOf("\n})();", start) + 6;   // the core's own closing line
const SIM = new Function(html.slice(start, end) + "\nreturn SIM;")();

const D2R = Math.PI / 180, g = 9810, P = 360 / 54;
const R_SEAT = 85.5, R_CM0 = 22 + 63.5 / 2;                                  // seated card edge; card centre of mass
const ROLLER_ANG = 46.667 - Math.atan2(7.5, 96) / D2R;                       // 42.2°: entry nip roller, nearest point
const ROLLER_R = Math.hypot(96, 7.5) - 8;                                    // r 88.3
const RAMP = { a0: 8, a1: 26, a2: 29, a3: 32, rmin: 86.0, rend: 87.5 };     // cad/params.scad ramp_* (angles above +X)

function rampR(phi) {
  if (phi < RAMP.a0 || phi > RAMP.a3) return null;
  if (phi <= RAMP.a1) return 88.5 - (88.5 - RAMP.rmin) * (phi - RAMP.a0) / (RAMP.a1 - RAMP.a0);
  if (phi <= RAMP.a2) return RAMP.rmin;
  return RAMP.rmin + (RAMP.rend - RAMP.rmin) * (phi - RAMP.a2) / (RAMP.a3 - RAMP.a2);
}

function slip(plan, mu, ramp, dt = 2.5e-4) {
  const segs = plan.wheel, T = plan.T;
  const life = plan.cards.map(c => {
    const s = c.segs.find(x => x.type === "slot"), e = c.segs.find(x => x.type === "exit");
    return s ? { slot: s.slot, t0: s.t0, t1: e ? e.t0 : T, u: 0, v: 0, prev: null } : null;
  }).filter(Boolean);
  let si = 0;
  const angAt = t => {
    while (si + 1 < segs.length && segs[si + 1].t0 <= t) si++;
    const s = segs[si]; if (!s || t < s.t0) return plan.opt.theta0;
    return s.a0 + s.dir * SIM.stepsAtTime(s.cum, s.k, Math.min(t, s.t1) - s.t0) / SIM.SPD;
  };
  const shutOpen = t => { let f = 0; for (const s of plan.shutter) if (s.t0 <= t) f = s.to; return f; };
  const out = { hits: 0, cards: new Set(), worst: 0, cwCatch: 0 };
  let a0 = angAt(0), a1 = angAt(dt);
  for (let t = dt; t < T - dt; t += dt) {
    const a2 = angAt(t + dt);
    const w = (a2 - a0) / (2 * dt) * D2R, al = (a2 - 2 * a1 + a0) / (dt * dt) * D2R, open = shutOpen(t);
    for (const c of life) {
      if (t < c.t0 || t >= c.t1) continue;
      const phi = ((a1 + (c.slot + 0.5) * P) % 360 + 360) % 360, r = R_CM0 + c.u;
      const A = w * w * r - g * Math.sin(phi * D2R);                         // outward: centrifugal + gravity
      const N = Math.abs(-g * Math.cos(phi * D2R) - al * r - 2 * w * c.v);    // load on the fin (gravity, Euler, Coriolis)
      let acc;
      if (c.v === 0) acc = Math.abs(A) > mu * N ? A - Math.sign(A) * mu * N : 0;
      else { acc = A - Math.sign(c.v) * mu * N; if (Math.sign(c.v + acc * dt) !== Math.sign(c.v)) { c.v = 0; acc = 0; } }
      c.v += acc * dt; c.u += c.v * dt;
      const inShroud = phi >= 172 || phi <= 8, onBlade = open < 0.5 && phi >= 199.4 && phi <= 216.6, rr = ramp ? rampR(phi) : null;
      if (rr != null && c.prev != null && c.prev > RAMP.a3 && phi <= RAMP.a3 && R_SEAT + c.u > RAMP.rend) out.cwCatch++;   // would meet the ramp's end
      const umax = onBlade ? 87.0 - R_SEAT : inShroud ? 88.5 - R_SEAT : rr != null ? rr - R_SEAT : 63.5;
      if (c.u <= 0) { c.u = 0; if (c.v < 0) c.v = 0; }
      if (c.u >= umax) { c.u = umax; if (c.v > 0) c.v = 0; }
      if (c.prev != null && c.prev < ROLLER_ANG && phi >= ROLLER_ANG && phi - c.prev < 30) {   // passes the roller, counter-clockwise
        out.worst = Math.max(out.worst, c.u);
        if (R_SEAT + c.u > ROLLER_R) { out.hits++; out.cards.add(c.slot); }
      }
      c.prev = phi;
    }
    a0 = a1; a1 = a2;
  }
  return out;
}

const N = +process.argv[2] || 12;
console.log(`${N} shuffles per case, 52 cards, shortest-path moves as in the firmware. Roller limit: ${(ROLLER_R - R_SEAT).toFixed(2)} mm of play.`);
for (const ramp of [false, true]) {
  console.log(ramp ? "\nWith the re-seating ramp on shroud_B:" : "\nWithout the ramp:");
  for (const mu of [0, 0.2, 0.3, 0.36, 0.4, 0.5, 0.6]) {
    let hit = 0, cards = 0, worst = 0, cw = 0;
    for (let n = 0; n < N; n++) {
      const plan = SIM.buildPlan({ key: crypto.createHash("sha256").update("card-slip-" + n).digest(), counter: 1, nCards: 52, theta0: (n * 61) % 360, unloadFix: true });
      const r = slip(plan, mu, ramp); hit += r.hits > 0; cards += r.cards.size; worst = Math.max(worst, r.worst); cw += r.cwCatch;
    }
    console.log(`  friction ${mu.toFixed(2)} (a card slides on PETG above ${(Math.atan(mu) / D2R).toFixed(0).padStart(2)}°): ` +
      `${String(hit).padStart(2)}/${N} shuffles hit the roller, ${(cards / N).toFixed(1).padStart(4)} cards per shuffle, worst play there ${worst.toFixed(2)} mm` +
      (ramp ? `, catches on the ramp end ${cw}` : ""));
  }
}
