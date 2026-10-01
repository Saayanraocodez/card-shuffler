#!/bin/bash
# Runs every collision check in check_clearance.scad. An empty intersection = OK.
cd "$(dirname "$0")"
CHECKS="sweep_feeder sweep_motors sweep_exit sweep_towers sweep_shroud sweep_shutter_closed sweep_shutter_open sweep_index motors_towers brackets_towers idler_deck idler_chute shutter_open_vs_fixed shutter_closed_vs_fixed shutter_mid_vs_fixed chute_shroud deck_shroud"
fail=0
for c in ${1:-$CHECKS}; do
  out=$(openscad -D "check=\"$c\"" -o /tmp/clear_$c.stl check_clearance.scad 2>&1)
  if echo "$out" | grep -q "top level object is empty"; then echo "OK         $c"
  elif echo "$out" | grep -qi "error"; then echo "ERROR      $c"; echo "$out" | grep -i error | head -3; fail=1
  else
    v=$(python3 -c "
import sys
mn=[1e9]*3; mx=[-1e9]*3; n=0
for l in open('/tmp/clear_$c.stl'):
    if 'vertex' in l:
        p=list(map(float,l.split()[1:4])); n+=1
        for i in range(3): mn[i]=min(mn[i],p[i]); mx[i]=max(mx[i],p[i])
print(f'{n} vertices, bbox x {mn[0]:.1f}..{mx[0]:.1f} y {mn[1]:.1f}..{mx[1]:.1f} z {mn[2]:.1f}..{mx[2]:.1f}')")
    echo "COLLISION  $c: $v"; fail=1
  fi
done
exit $fail
