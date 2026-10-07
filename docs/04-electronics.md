# 4. Electronics and battery power

Wiring diagram: `electronics/wiring-diagram.svg`. BOM: `05-bom.md` /
`electronics/bom.csv`. Firmware: `firmware/shuffler/`. All currents and
energies are **estimates [EST]**.

## 4.1 Architecture

```
 3S Li-ion pack (BMS) ─ fuse 3 A ─ switch ─┬─ VBAT (9.0–12.6 V) ─► TMC2208 ─► NEMA 17 (wheel)
 charge jack ── pack terminals             │                    └► 100k/33k divider ─► GPIO39
                                           └─ buck 5.0 V / 3 A ─┬─► ESP32 DevKitC (VIN)
                                                                ├─► DRV8833 #1 ─► N20 feed, N20 entry nip
                                                                ├─► DRV8833 #2 ─► N20 exit nip
                                                                ├─► MG90S shutter servo
                                                                ├─► 4 × IR LED (NPN switched) + index LED (always on)
                                                                └─► WS2812B
 Sensors (3V3): 4 phototransistors ─► GPIO34/35/32/36 (B, E, S, X); index phototransistor ─► GPIO19; button ─► GPIO21
```

## 4.2 Components

| Function | Part | Notes |
|---|---|---|
| Controller | ESP32 DevKitC V4 (WROOM-32, 30 pins) | hardware RNG, LEDC PWM, ADC1, USB serial |
| Wheel driver | TMC2208 stepstick, standalone, 1/16 | Vref ≈ 1.4 V ≈ 1.0 A RMS on the common 0.11 Ω-sense modules (I_RMS ≈ 0.71 × Vref; check your module); heatsink fitted. A4988 works from a 3S pack |
| Wheel motor | NEMA 17, 1.5–1.7 A class, 40 mm | direct drive through a 5→8 mm coupler onto the M8 rod |
| DC drivers | 2 × DRV8833 modules | 3 channels used |
| Feed roller, entry nip, exit nip | 3 × N20 6 V 300 rpm | feed at ~60 % PWM, nips at ~85–90 % |
| Shutter | MG90S | ±50° swing through an M2 pin |
| Sensors | 5 × 3 mm 940 nm IR LED + phototransistor: beams B (gate), E (card in slot), S (card seated), X (exit), and the index beam | B, E, S, X are read LED-on minus LED-off. E and S run 116 mm tower to tower, so their LEDs get 68 Ω. The index LED is always on |
| Battery | 3S Li-ion 11.1 V 2200–2600 mAh with BMS | 12.6 V 1 A barrel charger |
| 5 V rail | MP1584/LM2596 buck, ≥ 3 A | |
| Indicators | WS2812B pixel, passive piezo | |

## 4.3 ESP32 pin assignment

| GPIO | Name | Dir | Connected to |
|---|---|---|---|
| 25 / 26 / 27 | STEP / DIR / EN | out | TMC2208 (EN active low) |
| 16 / 17 | FEED_IN1 (PWM) / IN2 | out | DRV8833 #1 AIN1/AIN2 → feed roller |
| 4 / 5 | NIPE_IN1 (PWM) / IN2 | out | DRV8833 #1 BIN1/BIN2 → entry nip |
| 15 / 2 | NIPX_IN1 (PWM) / IN2 | out | DRV8833 #2 AIN1/AIN2 → exit nip (strapping pins: the driver inputs do not pull them; keep the DRV asleep at boot) |
| 23 | DRV_SLEEP | out | both DRV8833 nSLEEP |
| 18 | SERVO | out (50 Hz) | shutter MG90S |
| 13 | IR_LED_EN | out | NPN base via 1 kΩ (three emitters) |
| 14 | PIXEL | out | WS2812B DIN via 330 Ω |
| 33 | BUZZER | out (tone) | piezo via 100 Ω |
| 34 / 35 / 32 / 36 | BEAM_B / BEAM_E / BEAM_S / BEAM_X | ADC1 | phototransistor emitters, 10 kΩ pull-downs |
| 39 | VBAT | ADC1 | 100 kΩ / 33 kΩ divider, 100 nF |
| 19 | INDEX | in | index phototransistor emitter, 10 kΩ pull-down (LOW when the tab blocks the beam) |
| 21 | BUTTON | in (pull-up) | start button to GND |
| 22 | DECK_SENSE | in (opt.) | TCRT5000 DO |
| 12, 0 | avoid | | GPIO12 must not be pulled high at boot |

## 4.4 Power budget [EST]

Assumptions: 3S at 11.1 V, buck 85 %, wheel driver disabled between moves
(the shuffle is 40 s: 26 s loading with the wheel moving ~50 % of the time,
12 s unloading with the wheel moving ~40 %).

| Load | Condition | Power | Duty | Average |
|---|---|---|---|---|
| ESP32 | always | 0.40 W | 100 % | 0.40 W |
| IR LEDs (4 pulsed, ~160 mA while on) + index LED | polling | 0.45 W | 100 % | 0.45 W |
| Pixel | | 0.10 W | 100 % | 0.10 W |
| NEMA 17 at 1.0 A RMS | moving | 5.5 W | 45 % | 2.5 W |
| N20 feed + entry nip | 0.5 s per card during loading | 1.5 W | 45 % (of the load phase ≈ 30 % of total) | 0.45 W |
| N20 exit nip | 0.15 s per card during unloading | 0.75 W | 20 % of total | 0.15 W |
| Servo | holding open during unload | 0.5 W | 30 % | 0.15 W |
| **Total from the pack while shuffling** | | | | **≈ 4.7 W** |
| Idle | | | | ≈ 0.8 W |

* Energy per 40 s shuffle ≈ 190 J ≈ **0.05 Wh**.
* Usable pack energy ≈ 19.5 Wh → **≈ 390 shuffles** back-to-back (upper bound).
* A 4-hour evening with 40 shuffles ≈ 2 + 3.2 = 5.2 Wh → about 3–4 evenings per charge.

## 4.5 Peak and stall currents [EST]

| Item | Peak / stall |
|---|---|
| NEMA 17 (driver-limited) | 1.0 A RMS, 1.4 A peak per phase; at a 12 V bus ≈ 0.6 A from the pack during acceleration. Needed: ≈ 13 N·cm at 5000°/s²; a 17HS4401 gives ≈ 25–30 N·cm at 1 A, so the margin is about 2× |
| MG90S | ≈ 0.7 A at 5 V stall (shutter travel is unobstructed) |
| N20 ×3 | ≈ 0.8–1.0 A each at 5 V stall; at most two run at once |
| Worst case simultaneous | ≈ 5.5 W + 5 V × 2.7 A / 0.85 ≈ 21 W ≈ **2 A from the pack** |

3 A fuse, 3 A buck, pack BMS ≥ 5 A.

## 4.6 Battery, charging, low-battery behaviour

Unchanged from v1 (`archive/v1-elevator-insertion/docs/04-electronics.md`
§4.6):
* **Pack:** 3S1P 18650 pack with integrated protection, Velcro-strapped on
  a foam pad in the front base box.
* **Charging:** 5.5 × 2.1 mm charge jack wired to the pack terminals,
  upstream of the switch.
* **Thresholds:** warning below 9.9 V, refuse to start below 9.6 V, abort
  below 9.3 V. When the firmware aborts mid-load, it finishes the current
  card and unloads the cards already in the wheel into the chute. The
  remaining cards stay in the hopper. Charge, put all cards back in the
  hopper and shuffle again.
* **Options:** the USB-C PD power-bank option and the 2S option are as in
  v1.

## 4.7 Safety and shutdown

* The wheel cannot drop anything on power loss: cards sit in slots, the
  shroud holds the lower half, and the shutter's servo is unpowered when
  idle with the shutter closed by its own friction (add the return spring
  post if the shutter drifts).
* Any jam stops all motors within one sensor poll and waits for the
  button. Holding the button for 3 s runs "unload all": close the
  shutter, home, scan, bring the first occupied slot to the exit, open
  the shutter, then empty every occupied slot into the chute.
* **Interlock:** the wheel refuses to move while beam S (a card bridging
  the feeder and a slot) or beam X (a card in the exit nip) is blocked. A
  half-inserted card can therefore never be dragged into the frame.
* The wheel is enclosed by the towers, shroud, feeder and chute; the open
  areas are the hopper (under the lid) and the chute. Fingers cannot reach
  the wheel rim except through the chute end when the shutter is open; the
  wheel moves at up to 360°/s but has little inertia and the stepper stalls
  at about 20 N·cm.

## 4.8 Perfboard

70 × 50 mm perfboard: ESP32 on female headers, TMC2208 and two DRV8833 on
female headers, buck module, NPN with a 470 Ω base resistor, LED resistors
(150 Ω for B and X, 68 Ω for E and S, 330 Ω for the always-on index LED),
five 10 kΩ pull-downs, divider, capacitors (100 µF on TMC VM, 100 µF on
each DRV VM, 470 µF on 5 V), JST-XH headers J1–J17 (see the wiring
diagram). Mount in the front base box with
the ESP32's USB reachable through the side cut-out.
