# 5. Bill of materials

Machine-readable copy: `electronics/bom.csv`. USD estimates for single
quantities from hobby suppliers; "verified" = a price in that range was
seen in a supplier listing in September 2026, "typical" = from experience.
Shipping and printing excluded.

## 5.1 Electronics

| # | Item | Qty | Spec | Alternatives | Est. | Status |
|---|---|---|---|---|---|---|
| E1 | ESP32 DevKitC V4 | 1 | WROOM-32, 30-pin | ESP32-S3 | $6 | verified $5–10 |
| E2 | TMC2208 stepstick | 1 | standalone | A4988 (3S only) | $5 | verified $4–20 |
| E3 | NEMA 17 stepper | 1 | 1.5–1.7 A | 17HS4023 | $10 | typical |
| E4 | 5→8 mm flexible coupler | 1 | 19 × 25 | rigid | $2.50 | typical |
| E5 | M8 threaded rod 200 mm, 4 nyloc nuts, 6 washers | 1 | zinc | 8 mm rod + flat | $2 | typical |
| E6 | 608ZZ bearing | 2 | 8×22×7 | 608-2RS | $1.50 | typical |
| E7 | MG90S servo | 1 | metal gear | SG90 | $3 | verified $2.6–3.5 |
| E8 | N20 gearmotor 6 V 300 rpm | 3 | 3 mm D-shaft | 200–500 rpm | $12 | partly verified |
| E9 | DRV8833 module | 2 | dual H-bridge | TB6612FNG | $6 | verified $2–5 ea |
| E10 | 3S Li-ion pack with BMS | 1 | 2200–2600 mAh, ≥ 5 A | any protected 3S ≤ 75×60×22 | $18 | typical |
| E11 | 12.6 V 1 A charger | 1 | 5.5×2.1 | 2 A | $6 | verified $4–19 |
| E12 | Panel DC jack | 1 | 5.5×2.1 | | $1 | typical |
| E13 | Buck 5 V 3 A | 1 | MP1584/LM2596 | fixed 5 V | $2 | typical |
| E14 | Toggle switch | 1 | ≥ 3 A | rocker | $1.50 | typical |
| E15 | Push button 12 mm | 1 | | arcade | $2 | typical |
| E16 | WS2812B pixel | 1 | | 3 LEDs | $1 | typical |
| E17 | Passive piezo | 1 | | active | $1 | typical |
| E18 | IR LED 3 mm 940 nm | 5 | TSAL4400 / IR204 (beams B, E, S, X, index) | 5 mm | $1.75 | verified |
| E19 | IR phototransistor 3 mm | 5 | TEFT4300 / PT204 | 5 mm | $2.00 | verified |
| E21 | NPN 2N2222 | 1 | | N-MOSFET | $0.20 | typical |
| E22 | Resistors | set | 150×2, 68×2, 330×2, 470, 10 k×5, 100 k, 33 k, 100 | | $1 | typical |
| E23 | Capacitors | set | 100 µF×3, 470 µF, 100 nF×3 | | $1 | typical |
| E24 | Perfboard, headers, JST-XH kit, wire | 1 | | Dupont | $8 | typical |
| E25 | Polyfuse 3 A | 1 | | blade | $1 | typical |
| | **Electronics subtotal** | | | | **≈ $95.50** | |

## 5.2 Mechanical

| # | Item | Qty | Spec | Alternatives | Est. | Status |
|---|---|---|---|---|---|---|
| M1 | O-ring ID 20 × 3.5 | 5 | feed roller | silicone | $2 | typical (assortment ≈ $8 covers M1–M3) |
| M2 | O-ring ID 10 × 3.0 | 9 | nip rollers | | $2.25 | typical |
| M3 | O-ring 9 × 2 | 4 | idlers | none | $0.80 | typical |
| M4 | Self-adhesive felt 1.5 mm | 1 | chute wall, lid pad | EVA | $3 | typical |
| M5 | M3 screw assortment | 1 | ~80 pcs (6–20 mm) + M3 × 40 ×4, × 50 ×2, × 60 ×1, nuts, 3 grubs | inserts | $8 | typical |
| M6 | M2 × 8 / M2 × 12 | 4 + 1 | servo flange, horn pin (12 mm) | | $0.60 | typical |
| M7 | Pen springs | 4 | idler pressure | rubber bands | $0 | scrap |
| M8 | Dead AA cells | 2 | lid ballast | coins | $0 | scrap |
| M9 | Velcro strap + foam | 1 | battery | zip ties | $2 | typical |
| | **Mechanical subtotal** | | | | **≈ $19** | |

## 5.3 Totals

| | |
|---|---|
| Everything new, single quantities | **≈ $114** |
| With a normal parts drawer (E22–E24, M5) | ≈ $96 |
| Cheapest configuration (A4988, SG90, 17HS4023, skate bearings, printed idlers) | ≈ $88 |
| Optional TCRT5000 | +$1 |

Compared with v1: no brass blades, no lead screw, rods or LM8UU, one servo
instead of two; one more N20, one more DRV8833, two more IR pairs, an M8
rod and two bearings. Net ≈ −$6.

## 5.4 Sourcing notes

* "Ender-3 Z-axis kit" listings contain the 5→8 mm coupler.
* Skateboard bearings are 608 (any grade works).
* N20 motors: choose the 9 mm D-shaft version; 3 identical motors keep the
  BOM simple, and the feed roller runs at lower PWM.
* The index sensor is a bare LED and phototransistor pair in the printed
  index bracket. A slotted module cannot straddle the tab without its inner
  arm hitting the disc.
