# 7. Operation, maintenance, troubleshooting

## 7.1 Operating instructions

1. Switch on: green LED. Amber blinking = battery low (usable until refused).
2. Open the lid; put the deck in the hopper **face down**, short edge toward
   the wheel, resting against the gate; close the lid.
3. The chute must be empty (previous deck removed).
4. Press the button. Blue = loading (the wheel indexes for each card;
   about 26 s). Cyan = unloading (the wheel steps through the slots and
   cards slide into the chute; about 12 s).
5. Green + one beep: done. Lift the deck out of the chute.

Amber blinking + two beeps: the counts did not match (fewer cards loaded
than expected, fewer ejected than loaded, or a double-feed suspicion). The
deck is a valid ordering; re-run if it matters (re-running from any order
is uniform). Red blinking + three beeps: error; the serial console shows
the code. Press to clear. Hold 3 s: "unload all" — the wheel is scanned
and every occupied slot is emptied into the chute (also the way to retrieve
cards after a mid-shuffle stop).

`cal cards 54` for decks with jokers.

## 7.2 Jam recovery

* **Hopper / gate**: open the lid, lift the deck. The feed roller is under
  the floor slot.
* **Between gate and wheel**: lift the front of the entry idler arm
  against its springs and pull the card back, or push it fully into the
  slot. The wheel will not turn while a card bridges the mouth (beam S), so
  nothing moves while your fingers are there.
* **In the wheel**: hold the button 3 s (unload all). If a slot will not
  eject, run `shutter open` and pull the card out of the window by its
  edge. Or open the lid and reach in from the hopper side.
* **In the exit nip / chute**: lift the exit idler arm and pull the card
  through.

After any jam put all cards back in the hopper and start again.

## 7.3 Maintenance

| Interval | Task |
|---|---|
| Every 20 decks | Wipe the three rollers' O-rings and the four idler O-rings with isopropyl alcohol. |
| Every 50 decks | Blow dust out of the slot mouths (compressed air from the hopper side with the lid open, wheel turned by hand); blow off the three beams and the index sensor. |
| Every 200 decks or on double feeds | Re-set the gate with the feeler gauge; re-round the lip if grooved. |
| Yearly | Replace O-rings; check the shroud interior for polish marks (a strip of self-adhesive felt inside the shroud is an optional upgrade); check the wheel nuts are tight; re-run `cal beams`, `cal home`, `cal entry`, `cal exit`. |
| Battery | Store at 40–60 %; charge only with the 12.6 V adapter; replace when a charge no longer gives ~150 shuffles. |

Replaceable soft parts: roller and idler O-rings, felt strips, gate block.

## 7.4 Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| ERROR 1 index not found | index LED or phototransistor wiring and alignment, tab broken | `beams` shows `index=TAB` when the tab is in the beam; turn the wheel by hand past 12 o'clock |
| ERROR 2 no deck | roller not touching, O-rings dirty, gate too low, beam B threshold | `probe` while watching `beams`; `cal beams`; gate +0.05 |
| ERROR 3 jam (feed) | wedge at the gate, idler too tight, or a card stuck in the slot mouth (beam S blocked; the wheel will not turn) | pull the card back or push it in; then see 6.9, `cal entry`, `cal pwm nipe` |
| ERROR 4 cards left in the wheel | previous run stopped | hold 3 s (unload all) |
| ERROR 5 RNG health | hardware RNG stuck | power-cycle; if persistent do not use for play |
| ERROR 6 battery | < 9.6 V | charge; hold 3 s to unload cards left in the wheel |
| ERROR 8 lost card | card left the feeder but is not in the slot or its neighbours | check the fin mouths; the card may be lying on the shroud: unload all |
| ERROR 9 eject problem | a card is stuck in the exit nip, or no card came out of a slot the map says is full | lift the exit idler; look in the exit window; `cal pwm nipx`, `cal exit` |
| ERROR 11 map | a slot that should be empty holds a card even after re-homing | hold 3 s to unload, then shuffle again; if it repeats, check the index and `cal entry` |
| Corrections logged in one direction (`last`) | entry plane trim | `cal entry ±0.1` |
| Cards rub the shroud loudly | shroud too close (print) | shim the shroud tabs with washers |
| Unload leaves a card in a slot | card seated too deep after sliding on the shroud, or exit trim | `cal exit`; check the slot mouth; the count warning tells you |
| Output deck not square | chute end wall felt missing, cards bouncing | felt; reduce `cal pwm nipx` |
| Shuffle > 60 s | retries (see `last`), slow wheel (`WHEEL_ACC_DPS2` still 900 from an old build), long timeouts | fix feeding first; acceleration 5000°/s² with Vref ≈ 1.4 V |
| Stepper stalls on long moves | Vref low, wheel rubbing the shroud, nut loose | Vref ≈ 1.4 V; lower `WHEEL_ACC_DPS2` to 3500 (≈ 0.3 s per index) |

## 7.5 Safety notes

* Lithium pack: protected pack + matching charger only; do not charge
  unattended on flammable surfaces; stop using a swollen or warm pack.
* Keep fingers out of the chute while unloading; the wheel and nips have
  little force but the stepper does not sense a finger.
* Idle draw ≈ 0.8 W: switch off after the game.
