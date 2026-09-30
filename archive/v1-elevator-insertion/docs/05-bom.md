# 5. Bill of materials

Machine-readable copy: `electronics/bom.csv`. Prices are USD estimates for
single quantities from hobby suppliers (Amazon/eBay/AliExpress/DigiKey);
"verified" means a price in that range was seen in a supplier listing during
this design pass (September 2026), "unverified" means a typical price from
experience. Shipping and 3D printing are excluded. Nearly every item has a
cheaper multi-pack option.

## 5.1 Electronics

| # | Item | Qty | Specification | Alternatives | Est. | Status |
|---|---|---|---|---|---|---|
| E1 | ESP32 DevKitC V4 | 1 | WROOM-32, 30-pin, USB serial | ESP32-S3 DevKitC | $6 | verified $5–10 |
| E2 | TMC2208 stepstick | 1 | standalone, with heatsink | A4988, DRV8825 | $5 | verified $4–20 |
| E3 | NEMA 17 stepper | 1 | 1.0–1.7 A, 4-wire (17HS4401 / 17HS4023) | integrated lead-screw motor (LDO, $33) | $10 | typical |
| E4 | T8×8 lead screw 100 mm + brass nut | 1 | Tr8×8, flanged nut | T8×2 | $6 | verified $4–7 |
| E5 | 5→8 mm flexible coupler | 1 | 19 × 25 mm | (in some E4 kits) | $2.50 | typical |
| E6 | 8 mm × 100 mm rod | 2 | hardened steel | stainless | $5 | typical |
| E7 | LM8UU bearing | 2 | 8×15×24 | printed bushing | $2 | typical |
| E8 | MG90S servo | 2 | metal gear | SG90 | $6 | verified $2.6–3.5 ea |
| E9 | N20 gearmotor 6 V 300 rpm | 2 | 3 mm D-shaft | 200–500 rpm | $8 | partly verified |
| E10 | DRV8833 module | 1 | dual H-bridge | TB6612FNG | $3 | verified $2–5 |
| E11 | 3S Li-ion pack with BMS | 1 | 11.1 V, 2200–2600 mAh, ≥ 5 A BMS | any protected 3S pack ≤ 75×60×22 mm | $18 | typical |
| E12 | 12.6 V 1 A charger, 5.5×2.1 plug | 1 | CC/CV adapter | 12.6 V 2 A | $6 | verified $4–19 |
| E13 | Panel DC jack 5.5×2.1 | 1 | | | $1 | typical |
| E14 | Buck module 5 V 3 A | 1 | MP1584/LM2596 | fixed 5 V | $2 | typical |
| E15 | Toggle switch SPST | 1 | ≥ 3 A | rocker | $1.50 | typical |
| E16 | Push button 12 mm | 1 | momentary | arcade | $2 | typical |
| E17 | WS2812B pixel | 1 | | 3 LEDs | $1 | typical |
| E18 | Passive piezo | 1 | | active buzzer | $1 | typical |
| E19 | IR LED 3 mm 940 nm | 3 | TSAL4400 / IR204 | 5 mm | $1.05 | verified |
| E20 | IR phototransistor 3 mm | 3 | TEFT4300 / PT204 | 5 mm | $1.20 | verified |
| E21 | Lever microswitch | 1 | KW11/KW12 | optical endstop | $0.50 | typical |
| E22 | NPN 2N2222 | 1 | | N-MOSFET | $0.20 | typical |
| E23 | Resistors | set | 150 Ω×3, 1 k, 10 k×4, 100 k, 33 k, 330, 100 | | $1 | typical |
| E24 | Capacitors | set | 100 µF/25 V ×2, 470 µF, 100 nF ×3 | | $1 | typical |
| E25 | Perfboard, headers, JST-XH kit, wire | 1 | 70×50 mm | Dupont | $8 | typical |
| E26 | Polyfuse 3 A | 1 | | blade fuse | $1 | typical |
| | **Electronics subtotal** | | | | **≈ $100** | |

## 5.2 Mechanical

| # | Item | Qty | Specification | Alternatives | Est. | Status |
|---|---|---|---|---|---|---|
| M1 | O-ring ID 20 × CS 3.5 nitrile | 6 | rollers (2 spare) | silicone | $2.40 | typical (assortment ≈ $8 covers M1+M2) |
| M2 | O-ring 9 × 2 | 2 | idler wheels | none | $0.40 | typical |
| M3 | Brass sheet 0.5 mm | 1 | 100×100 mm, cut 2 blades 40×30 | feeler-gauge set ($7, verified) | $6 | partly verified |
| M4 | Self-adhesive felt 1.5 mm | 1 | back pad, lid pad | EVA foam | $3 | typical |
| M5 | M3 screw assortment | 1 | M3×6…20, ×50, ×60, nuts, 4 grub | heat-set inserts optional | $7 | typical |
| M6 | M2×8 screws | 4 | servos, endstop | | $0.40 | typical |
| M7 | Pen springs | 2 | idler pressure | rubber band | $0 | scrap |
| M8 | Dead AA cells | 2 | lid ballast | coins | $0 | scrap |
| M9 | Velcro strap + foam pad | 1 | battery retention | zip ties | $2 | typical |
| | **Mechanical subtotal** | | | | **≈ $21** | |

## 5.3 Totals

| | |
|---|---|
| Parts total, buying everything new in single quantities | **≈ $121** |
| If you already have screws, wire, perfboard, resistors, capacitors (E23–E25, M5) | ≈ $104 |
| Cheapest configuration (A4988, SG90 ×2, feeler-gauge blades, printed bushings) | ≈ $95 |
| Optional TCRT5000 deck sensor | +$1 |
| Optional heat-set inserts (20) | +$5 |

Printing: about 650 g of filament (PETG or PLA) for all parts; not costed.

## 5.4 Sourcing notes

* Lead screw kits sold for Ender-3 Z axes ("T8 8 mm lead 100/150 mm with
  nut, coupler, bearing") contain E4 + E5.
* Buy the O-rings as a metric assortment kit; sizes are not critical
  (ID 19–21 mm, cross-section 3.0–3.5 mm all fit the groove).
* Choose N20 motors with the 9 mm D-shaft; some listings have a 10 mm
  round shaft (then use the roller's set screw only).
* The 3S pack must physically fit 75 × 60 × 22 mm; verify before buying.
* Supplier links checked during design: see the reference list in
  `docs/10-deliverables-and-gaps.md`.
