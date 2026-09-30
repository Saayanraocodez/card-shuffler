#!/bin/bash
# Renders every printable part to STL and the documentation views to PNG.
# Usage: ./render.sh            (all)     ./render.sh stl     ./render.sh png
set -e
cd "$(dirname "$0")"
PARTS="feeder_deck gate_block roller motor_bracket motor_retainer bushing_bracket idler_arm idler_wheel lid well_deck well_sleeve knife_bar_lower knife_bar_upper servo_bracket column carriage platform skirt_front skirt_rear"
if [ "${1:-all}" != "png" ]; then
  for p in $PARTS; do
    echo "== $p"; openscad -q -D "part=\"$p\"" -o "stl/$p.stl" shuffler.scad 2>&1 | grep -E "WARNING|ERROR" || true
  done
fi
if [ "${1:-all}" != "stl" ]; then
  R="xvfb-run -a openscad --projection=p --imgsize=1800,1200 --colorscheme=Tomorrow"
  $R -D 'part="assembly"' -D explode=0  --camera=40,0,80,62,0,32,720  -o png/assembly.png shuffler.scad
  $R -D 'part="assembly"' -D explode=35 --camera=40,0,110,62,0,32,900 -o png/exploded.png shuffler.scad
  $R -D 'part="assembly"' -D section=1  --camera=40,0,80,90,0,0,620   -o png/section.png shuffler.scad
  $R -D 'part="feeder_module"' -D explode=25 --camera=-20,0,20,60,0,30,420 -o png/feeder_exploded.png shuffler.scad
  $R -D 'part="well_module"'   -D explode=25 --camera=110,0,20,60,0,30,480 -o png/well_exploded.png shuffler.scad
  $R -D 'part="well_module"'   -D explode=0  --camera=110,0,10,90,0,0,330  -o png/well_section_front.png shuffler.scad
fi
echo done
