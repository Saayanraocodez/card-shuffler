#!/bin/bash
# Renders every printable part to STL and the documentation views to PNG (OpenSCAD 2021.01).
set -e
cd "$(dirname "$0")"
PARTS="wheel hub_spacer tower_R tower_L motor_mount shroud_A shroud_B shutter index_bracket feeder_deck gate_block roller_feed roller_nip feeder_motor_bracket feeder_bushing_bracket idler_arm idler_wheel spring_bar lid chute base_front base_rear base_cover_front base_cover_rear chute_dealer throw_arm tt_cover_front tt_foot tt_pinion"
if [ "${1:-all}" != "png" ]; then
  for p in $PARTS; do echo "== $p"; openscad -q -D "part=\"$p\"" -o "stl/$p.stl" shuffler.scad 2>&1 | grep -E "WARNING|ERROR" || true; done
fi
if [ "${1:-all}" != "stl" ]; then
  R="xvfb-run -a openscad --projection=p --imgsize=1800,1200 --colorscheme=Tomorrow"
  $R -D 'part="assembly"' -D explode=0  --camera=-20,0,150,60,0,30,950  -o png/assembly.png shuffler.scad
  $R -D 'part="assembly"' -D explode=30 --camera=-20,0,150,60,0,30,1150 -o png/exploded.png shuffler.scad
  $R -D 'part="assembly"' -D section=1  --camera=-20,0,150,90,0,0,820   -o png/section.png shuffler.scad
  $R -D 'part="wheel_module"'  -D explode=25 --camera=0,0,0,60,0,40,700    -o png/wheel_exploded.png shuffler.scad
  $R -D 'part="feeder_module"' -D explode=20 --camera=90,0,100,60,0,20,420 -o png/feeder_exploded.png shuffler.scad
  $R -D 'part="exit_module"'   -D explode=20 --camera=-110,0,-60,60,0,40,420 -o png/exit_exploded.png shuffler.scad
  $R -D 'part="assembly"' --camera=-20,0,150,90,0,90,820 -o png/front.png shuffler.scad
  # dealer build (docs/11-dealer.md)
  $R -D 'part="assembly"' --camera=-40,0,130,62,0,35,1050 -o png/dealer_assembly.png shuffler.scad
  $R -D 'part="assembly"' -D section=1 --camera=-40,0,120,90,0,0,900 -o png/dealer_section.png shuffler.scad
  $R -D 'part="exit_module"' --camera=-150,0,-75,55,0,320,330 -o png/dealer_launcher.png shuffler.scad
  xvfb-run -a openscad --projection=p --imgsize=1800,1000 --colorscheme=Tomorrow -D 'part="turntable_parts"' --camera=0,0,0,40,0,0,620 -o png/dealer_turntable.png shuffler.scad
fi
echo done
