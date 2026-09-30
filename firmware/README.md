# Firmware (ESP32 DevKitC, Arduino framework) — wheel shuffler

```
shuffler/
  shuffler.ino     state machine: LOAD (random empty slot per card, verified by beam E), UNLOAD (slot order)
  config.h         pins, wheel geometry, speeds, timeouts, calibration record
  platform.*       core 2.x / 3.x shims, hardware RNG with the ADC entropy source
  hardware.*       wheel stepper, shutter servo, three N20 motors, beams B/E/X, index, battery, LED, buzzer
  motion.*         wheel home / goto / fin-to-entry / fin-to-exit / scan; feeder; eject
  rng.* sha256.* chacha20.*   entropy → SHA-256 → ChaCha20 → rejection sampling (portable)
  shuffle_core.*   empty-slot assignment + output prediction (pure, host-tested)
  storage.*        calibration in NVS
  cli.cpp          serial console (115200): `help`
test_host/         make run (bit-exact test vs simulation/rng_reference.py); ./syntax_check.sh (Arduino mock)
platformio.ini     pio run -e esp32dev
```

## Status

The platform-independent core is unit-tested and bit-exact with the Python
reference. The hardware modules were syntax-checked with `g++` against a
minimal Arduino mock; they have **not** been compiled with the real ESP32
core (toolchain download blocked in the authoring environment). Expect
first-compile fixes and tune every timeout and PWM on hardware.

## Bring-up (see docs/06 §6.7)

`bat` → `cal vbat`; `beams` → `cal beams`; `home`; `entry 0` + straight edge →
`cal home`; `shutter open/close` → `cal shutter`; `feed` with `entry k`;
`cal entry`; `exit k` + `eject` → `cal exit`; `test fixed 1` + button →
`analyze_physical.py --compare`; `last` → `analyze_physical.py --log`.
