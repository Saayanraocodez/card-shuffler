# 4. Electronics and battery power

Wiring diagram: `electronics/wiring-diagram.svg`. Bill of materials:
`05-bom.md` / `electronics/bom.csv`. Firmware: `firmware/shuffler/`.

All current and energy figures below are **estimates [EST]** from datasheet
values; none has been measured on a built unit.

## 4.1 Architecture

```
 3S Li-ion pack (BMS) ─ fuse 3 A ─ switch ─┬─ VBAT (9.0–12.6 V) ─► TMC2208 ─► NEMA 17 (elevator)
 charge jack ── to pack terminals           │                    └► 100k/33k divider ─► ADC (GPIO39)
                                            └─ buck 5.0 V / 3 A ─┬─► ESP32 DevKitC (VIN)
                                                                 ├─► DRV8833 (VM) ─► 2 × N20 (feed, transport)
                                                                 ├─► 2 × MG90S servo (blades)
                                                                 ├─► 3 × IR LED (switched by NPN)
                                                                 └─► WS2812B pixel
 Sensors (3V3): 3 phototransistors ─► ADC GPIO34/35/36, endstop ─► GPIO32, button ─► GPIO21
```

## 4.2 Component choices

| Function | Part | Why | Alternatives |
|---|---|---|---|
| Controller | ESP32 DevKitC V4 (WROOM-32, 30 pins) | Hardware RNG, plenty of GPIO, LEDC PWM for servos/motors, ADC1 for sensors, USB serial for test dumps, Arduino support | ESP32-S3 DevKitC (same code, pin remap); Raspberry Pi Pico (needs different RNG source and PWM code) |
| Stepper driver | TMC2208 stepstick (standalone mode, 1/16 µstep) | Quiet, 4.75–36 V input so it also works from 2S packs, stall-free at 0.65 A | A4988 (needs ≥ 8 V, noisier), DRV8825 |
| Elevator motor | NEMA 17, 1.5–1.7 A class (17HS4401) or 1.0 A 23 mm (17HS4023) | Ubiquitous 3D-printer part; run at 0.65 A RMS it has far more torque than the 150 g load needs | Integrated lead-screw NEMA 17 (no coupler) |
| DC motor driver | DRV8833 dual H-bridge module | Two channels, 1.5 A peak each, works at 5 V | TB6612FNG |
| Feed / transport motors | N20 6 V 300 rpm micro gearmotors ×2 | Cheap, compact, D-shaft; ~0.25 m/s card speed on Ø27 rollers at ~70 % PWM | N20 200–500 rpm (adjust PWM) |
| Blade servos | MG90S metal gear ×2 | 1.8 kg·cm, 0.1 s/60°, cheap | SG90 (plastic gears; acceptable) |
| Card sensors | 3 mm 940 nm IR LED + phototransistor pairs ×3 | Vertical beams detect any card regardless of back colour; cheap | 3 mm slotted photo-interrupter modules where geometry allows |
| Endstop | KW11/KW12 lever microswitch, NC contact | Wire-break safe | Optical endstop module |
| Battery | 3S Li-ion 11.1 V 2200–2600 mAh pack **with built-in BMS** (18650 ×3) | Commercially assembled and protected; 12.6 V charger adapters are cheap; gives the stepper full speed | See §4.6 for the USB-C option |
| Charger | 12.6 V 1 A CC/CV Li-ion charger with 5.5×2.1 barrel plug | Matches 3S packs; ~2.5 h charge | 12.6 V 2 A |
| 5 V rail | MP1584 or LM2596 buck module set to 5.0 V, ≥ 3 A | Servos and motors need a stiff 5 V | — |
| Indicators | WS2812B pixel + passive piezo | One RGB LED shows every state; beeps for done/jam/battery | 3 discrete LEDs |

## 4.3 ESP32 pin assignment

Strapping pins (0, 2, 12, 15) are avoided. GPIO 34–39 are input-only with no
internal pull-ups (external 10 kΩ pull-downs on the phototransistor
outputs). ADC2 pins are not used for analog input.

| GPIO | Name | Direction | Connected to |
|---|---|---|---|
| 25 | STEP | out | TMC2208 STEP |
| 26 | DIR | out | TMC2208 DIR |
| 27 | STEP_EN | out (active low) | TMC2208 EN |
| 16 | FEED_IN1 | out (PWM) | DRV8833 AIN1 |
| 17 | FEED_IN2 | out | DRV8833 AIN2 |
| 4 | TRANS_IN1 | out (PWM) | DRV8833 BIN1 |
| 5 | TRANS_IN2 | out | DRV8833 BIN2 |
| 23 | DRV_SLEEP | out | DRV8833 nSLEEP (held low until init) |
| 18 | SERVO_L | out (50 Hz PWM) | left MG90S signal |
| 19 | SERVO_R | out (50 Hz PWM) | right MG90S signal |
| 13 | IR_LED_EN | out | NPN base via 1 kΩ (switches all 3 IR LEDs) |
| 14 | PIXEL | out | WS2812B DIN via 330 Ω |
| 33 | BUZZER | out (LEDC tone) | piezo via 100 Ω |
| 34 | BEAM_B | ADC1 | gate beam phototransistor emitter (10 kΩ to GND) |
| 35 | BEAM_W | ADC1 | well-slot beam phototransistor |
| 36 | BEAM_S | ADC1 | stack-top beam phototransistor |
| 39 | VBAT | ADC1 | 100 kΩ / 33 kΩ divider, 100 nF |
| 32 | ENDSTOP | in, pull-up | microswitch NC to GND |
| 21 | BUTTON | in, pull-up | start button to GND |
| 22 | DECK_SENSE | in (optional) | TCRT5000 module DO |

## 4.4 Power budget [EST]

Assumptions: 3S pack at 11.1 V nominal; buck efficiency 85 %; stepper driver
disabled between moves (lead screw is self-locking); one shuffle = 52 cards
in 75 s.

| Load | Condition | Current | Power | Duty during shuffle | Average |
|---|---|---|---|---|---|
| ESP32 DevKitC (Wi-Fi off) | always | 80 mA @ 5 V | 0.40 W | 100 % | 0.40 W |
| IR LEDs (3 × 20 mA, 50 % modulated) | polling | 30 mA @ 5 V | 0.15 W | 100 % | 0.15 W |
| WS2812B | on | 20 mA @ 5 V | 0.10 W | 100 % | 0.10 W |
| NEMA 17 via TMC2208, 0.65 A RMS | moving | ≈0.45 A @ 11.1 V | 5.0 W | 40 % | 2.0 W |
| 2 × MG90S | moving 0.2 s ×2 per card, holding otherwise while extended | 0.3 A / 0.1 A @ 5 V | 1.5 / 0.5 W | 30 % / 40 % | 0.65 W |
| 2 × N20 (0.15 A each running) | feeding 0.5 s per card | 0.3 A @ 5 V | 1.5 W | 35 % | 0.5 W |
| **Total at the 5 V rail** | | | | | 1.8 W → 2.1 W from the pack |
| **Total from the pack while shuffling** | | | | | **≈ 4.1 W** |
| Idle (ESP32 + sensors + pixel) | | | | | ≈ 0.75 W |

* Energy per shuffle: 4.1 W × 75 s ≈ **310 J ≈ 0.085 Wh**.
* Pack energy: 2200 mAh × 11.1 V = 24.4 Wh; usable ≈ 80 % (cutoff 3.2 V/cell,
  ageing) ≈ 19.5 Wh.
* Shuffles per charge, back-to-back: ≈ 19.5 / 0.085 ≈ **≈ 230** (upper
  bound; ignores idle).
* A four-hour game evening with 40 shuffles: 40 × 0.085 + 4 h × 0.75 W =
  3.4 + 3.0 = **6.4 Wh → about 3 evenings per charge**.
* Charge time at 1 A: ≈ 2.5–3 h from empty.

These figures will move with the real duty cycle and motor currents; the
firmware's serial `stats` command prints measured shuffle time and battery
voltage so the estimate can be corrected after the first build.

## 4.5 Peak and stall currents [EST]

| Item | Stall / peak | Notes |
|---|---|---|
| NEMA 17 through TMC2208 | 0.65 A RMS set by Vref (0.9 A peak/phase) | Current is regulated by the driver; a stall does not raise it |
| MG90S | ≈ 0.7 A each at 5 V stall | Blade entry force is < 1 N; stall only on a jam |
| N20 6 V 300 rpm | ≈ 0.8–1.0 A each at 5 V stall | Firmware limits motor-on time (jam timeout) |
| Worst-case simultaneous | stepper move + both servos stalled + both N20 stalled | ≈ 5 W + 5 V × 3.4 A / 0.85 = 25 W ≈ **2.3 A from the pack** |

The 3 A fuse and a 3 A buck cover this. Pack BMS discharge limits of 5 A or
more are typical for 3S 18650 packs; check the listing.

## 4.6 Battery details

* **Pack**: 3S1P 18650 Li-ion, 11.1 V nominal, 2200–2600 mAh, with
  integrated protection board (over-charge, over-discharge, over-current,
  short circuit). Buy a pack that states its BMS discharge current (≥ 5 A)
  and comes with a connector (XT30 or JST-XH); do not build the pack from
  loose cells unless you are equipped to do so safely.
* **Mounting**: the pack sits in the front skirt under the hopper on a
  self-adhesive foam pad and is held by a Velcro strap through two slots
  (see assembly step 20). Keep it away from the motor bracket and route its
  leads to J1 on the perfboard.
* **Charging**: the panel-mounted 5.5×2.1 mm jack is wired to the pack
  terminals (through the BMS) upstream of the power switch; the 12.6 V 1 A
  adapter's own LED shows charge state. Charging with the machine switched
  off is recommended.
* **Low-battery behaviour** (firmware): warning colour below 9.9 V
  (3.3 V/cell); a new shuffle is refused below 9.6 V; if the voltage falls
  below 9.3 V during a shuffle the current card is completed (gap closed,
  blades retracted), the stack is presented, the drivers are disabled and a
  triple beep is issued. The BMS cuts off at about 2.5–3.0 V/cell as the
  last resort.
* **USB-C option (documented, not the primary design)**: replace the pack
  and charger with a USB-C PD power bank (20 W or more) plus a PD trigger
  module set to 12 V feeding the same VBAT bus. This gives USB-C charging
  and mains operation from a PD wall adapter. Drawbacks: the bank is
  external (a pocket in the skirt is easy to add), and many banks switch
  off below ~100 mA load, so the firmware's keep-alive option
  (`KEEPALIVE_MS`) pulses the stepper coils briefly every 10 s when
  enabled. Availability and prices of PD trigger boards were not verified
  in this design pass.
* **2S option**: a 2S pack (7.4 V) works with the TMC2208 but the elevator
  speed roughly halves; the A4988 cannot be used below 8 V.

## 4.7 Safety and shutdown

* The elevator lead screw is self-locking: on power loss the platform stays
  put and the deck does not fall.
* On any jam the firmware stops all motors within 50 ms, retracts the blades,
  disables the stepper driver and puts the DRV8833 to sleep; motors cannot
  restart until the button is pressed.
* The stepper driver is disabled whenever the machine is idle, so nothing
  gets hot.
* The button is the only user control; holding it for 3 s in an error state
  performs a homing cycle and lowers the platform for jam access.
* The rollers are under a lid and the well is open only at the top; no pinch
  point is reachable with a finger while the machine runs, but keep hair
  and clothing away from the open well.

## 4.8 Perfboard layout

A 70 × 50 mm perfboard carries the ESP32 (on female headers), the TMC2208
and DRV8833 (on female headers), the buck module, the NPN LED switch, the
voltage divider, pull-down resistors, and JST-XH headers for every external
connection (J1–J16, listed on the wiring diagram). Suggested placement:
ESP32 along one long edge with USB accessible through the skirt's side
cut-out; TMC2208 near the pack connector with the 100 µF capacitor next to
its VM pin; sensor headers along the opposite edge.
