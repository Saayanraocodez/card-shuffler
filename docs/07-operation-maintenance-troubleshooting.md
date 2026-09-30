# 7. Operation, maintenance, troubleshooting

## 7.1 Operating instructions

1. Switch on. The LED shows green (idle). Amber blinking = battery low
   (still usable until the machine refuses to start).
2. Open the lid, put the deck in the hopper **face down**, long edge along
   the machine, pushed gently toward the gate. Close the lid.
3. Make sure the well is empty (the previous deck removed).
4. Press the button once. Blue LED: shuffling. Expect 60–80 s for 52 cards
   (first builds).
5. Green LED and one beep: done. The elevator presents the deck 3 mm above
   the rim; pinch it through the finger notches and lift it out.
6. Press the button once more to lower the platform (or just load the next
   deck: the next shuffle homes first).

Amber blinking with two beeps after a shuffle: the card count differed from
the expected 52 (or a double feed was suspected). The deck is a valid but
possibly less random ordering; re-run the shuffle if it matters (a re-run
from any order is uniform).

Red blinking with three beeps: error. Serial console shows the code. Press
the button to clear; hold it 3 s to retract the blades, home and lower the
platform for jam access.

Setting 54 cards (jokers): serial `cal cards 54`.

## 7.2 Jam recovery

Cards are reachable without tools:

* **Hopper / gate**: open the lid, lift the deck out. The feed roller is
  visible through the floor slot.
* **Between gate and well** (the most likely place): lift the idler arm's
  front with a finger (it pivots) and pull the card back toward the hopper,
  or push it forward into the well with the gap still open (after an error
  the blades stay in and the gap stays open until you press the button).
* **In the well**: hold the button 3 s → blades out, platform down; lift the
  cards out through the top.

After any jam: put all cards back into the hopper and start again. The
algorithm is input-independent, so a restart from a half-shuffled deck is
just as random as from a fresh deck.

## 7.3 Maintenance

| Interval | Task |
|---|---|
| Every 20 decks | Wipe the roller O-rings and the idler O-rings with isopropyl alcohol on a lint-free cloth (card coating and dust reduce grip). |
| Every 50 decks | Blow dust off the three beam sensors (compressed air through the finger notch and the slot). Check the felt pads. |
| Every 200 decks or when double feeds start | Re-set the gate with the feeler gauge; check the gate lip for polish/grooves and re-round it. |
| Yearly | Replace the roller O-rings (nitrile hardens). Re-run `cal beams`, `cal home`, `cal knifetest`. Check the lead-screw nut for play (replace the brass nut). |
| Battery | Store at 40–60 % charge if unused for months; charge with the supplied 12.6 V adapter only; replace the pack when a full charge no longer gives ~100 shuffles. |

Replaceable soft parts: roller O-rings (M1), idler O-rings (M2), felt pads
(M4), gate block (printed), blades (brass, M3).

## 7.4 Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| ERROR 1 (home) | endstop wiring (NC contact), lever bent, carriage stuck | `status` shows endstop state; check with the carriage pressed against the lever |
| ERROR 2 (no deck) with a deck loaded | feed roller not touching the card, O-rings dirty, gate too low, beam B threshold wrong | `probe` while watching `beams`; `cal beams`; gate +0.05 |
| ERROR 3 jam: pickup | as above, or a warped card | check the deck lies flat |
| ERROR 3 jam: gate/nip | card stuck at the gate (double feed wedge) or idler too tight | gate −0.05; check lip smoothness; weaker springs |
| ERROR 3 jam: well entry | gap not open (blade missed and the upper stack fell), card stopped short, W beam misaligned | `cal knifetest`; raise transport PWM; `beams` |
| ERROR 4 well not empty | cards left in the well, or beam S blocked by dirt | remove cards; clean |
| ERROR 5 RNG | hardware RNG health test failed (very unlikely) | power-cycle; if persistent the ESP32 board is faulty or `USE_BOOTLOADER_RANDOM` failed to link — do not use the machine for play until resolved |
| ERROR 6 battery | pack below 9.6 V | charge |
| ERROR 8 stack measure | beam S never blocked while raising | LED/PT alignment; threshold |
| Deck comes out with the input bottom card still at the bottom repeatedly | gap 0 never used: blade offset too high (systematic +1) | `cal knife -0.05`; run the predicted-vs-actual test |
| Cards stick together when lifted from the well | static (plastic cards) | rub the deck with a dryer sheet; fan the deck before loading |
| Shuffle takes > 100 s | many retries (see `last`), slow servo settle, low transport PWM | fix the feed first; reduce `SERVO_SETTLE_MS` to 150 if the servos are fast |
| Stepper skips (buzzing, then a wrong height) | Vref too low, coupler slipping, rods binding | Vref 0.9 V; tighten coupler; ream the column holes; grease the rods lightly |
| Blades do not fully retract | rubber band lost, bar binding in the loops | replace band; sand the bar |
| Power bank switches off (USB-C option) | idle current below the bank's threshold | set `KEEPALIVE_MS 10000` |

## 7.5 Safety notes

* Lithium pack: use only the protected pack and its matching charger; do
  not charge unattended on flammable surfaces; stop using a pack that
  swells or gets warm while idle.
* Keep fingers out of the well while the elevator moves; the blades and
  platform exert little force but the stepper does not know your finger is
  there.
* The stepper driver disables itself when idle; if the machine is left on,
  it draws about 0.75 W and the pack will empty in about a day.
