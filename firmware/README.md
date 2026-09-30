# Firmware (ESP32 DevKitC, Arduino framework)

```
shuffler/
  shuffler.ino     main state machine, shuffle procedure, button, low-battery behaviour
  config.h         pins, mechanical constants, defaults, calibration record
  platform.*       core 2.x / 3.x shims (LEDC, NeoPixel, hardware RNG with the ADC entropy source)
  hardware.*       stepper, servos, DRV8833 motors, IR beams, endstop, button, battery, LED, buzzer
  motion.*         elevator (home, move, find beam S) and feeder (probe, feed one card, reverse pulse)
  rng.*            entropy health test, SHA-256 conditioning, ChaCha20 DRBG, rejection sampling
  sha256.*  chacha20.*   portable primitives
  shuffle_core.*   gap drawing + permutation prediction (pure, host-tested)
  storage.*        calibration in NVS
  cli.cpp          serial console (115200): calibration, tests, RNG dumps
test_host/         `make run` — host unit test of the RNG/algorithm core against the Python reference
platformio.ini     `pio run -e esp32dev`
```

Build with PlatformIO (`pio run -e esp32dev -t upload`) or the Arduino IDE
(board "ESP32 Dev Module"; open `shuffler/shuffler.ino`; no external
libraries are needed — the NeoPixel is driven by the core's RMT helper).
Serial console at 115200 baud; type `help`.

## Randomness pipeline

See `docs/02-mathematics.md` §2.3. In short: `bootloader_random_enable()` →
64 bytes of hardware random → health test → SHA-256 with a jitter pool
(sensor edge timestamps, button timing, ADC noise) and counters → ChaCha20
keystream → rejection-sampled gap indices. `test ref` must print the same
52 gaps as `simulation/rng_reference.py --selftest`.

## Bring-up order (see docs/06-assembly.md §6.9 for details)

1. `bat` → set `cal vbat <multimeter volts>`.
2. `beams` with nothing in the machine; `cal beams`.
3. `home`; `z 0`; `z 10`; `cal home` (empty well).
4. `blades in` / `blades out`; adjust `cal servo …` until the blades protrude 7 mm / sit 1 mm inside the wall.
5. `probe`, `feed` with a deck loaded; adjust `cal pwm …` and the gate.
6. `cal knifetest 10` with a deck in the well; adjust `cal knife ±0.025`.
7. `test fixed 1`, press the button, compare with `simulation/analyze_physical.py --compare`.

## Status of this code

Written for this design; the platform-independent core (`rng`, `sha256`,
`chacha20`, `shuffle_core`) is unit-tested on the host and matches the
Python reference bit for bit. The hardware modules follow the Arduino-ESP32
APIs of both core 2.x and 3.x but **have not been compiled for the ESP32**:
the authoring environment could not download the Espressif toolchain (see
`docs/10-deliverables-and-gaps.md`). Expect first-compile fixes, and expect
to tune every timeout and PWM value on real hardware.
