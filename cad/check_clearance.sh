#!/bin/bash
# Runs every collision check in check_clearance.scad. An empty intersection = OK.
cd "$(dirname "$0")"
CHECKS="sweep_feeder sweep_motors sweep_exit sweep_towers sweep_shroud sweep_shutter_closed sweep_shutter_open sweep_index motors_towers brackets_towers idler_deck idler_chute shutter_open_vs_fixed shutter_closed_vs_fixed shutter_mid_vs_fixed chute_shroud deck_shroud chute_towers deck_towers shroud_towers index_towers lid_deck motor_mount_tower"
fail=0
for c in ${1:-$CHECKS}; do
  out=$(openscad -D "check=\"$c\"" -o /tmp/clear_$c.stl check_clearance.scad 2>&1)
  if echo "$out" | grep -q "top level object is empty"; then echo "OK         $c"
  elif echo "$out" | grep -qi "error"; then echo "ERROR      $c"; echo "$out" | grep -i error | head -3; fail=1
  else
    # mating parts may touch on a face: a zero-volume intersection is contact, not a collision
    v=$(python3 -c "
import re
mn=[1e9]*3; mx=[-1e9]*3; vol=0.0; tri=[]
for l in open('/tmp/clear_$c.stl'):
    if 'vertex' in l:
        p=list(map(float,l.split()[1:4])); tri.append(p)
        for i in range(3): mn[i]=min(mn[i],p[i]); mx[i]=max(mx[i],p[i])
        if len(tri)==3:
            a,b,c=tri; vol+=(a[0]*(b[1]*c[2]-b[2]*c[1])-a[1]*(b[0]*c[2]-b[2]*c[0])+a[2]*(b[0]*c[1]-b[1]*c[0]))/6; tri=[]
print(f'{abs(vol):.2f} {mn[0]:.1f}..{mx[0]:.1f} y {mn[1]:.1f}..{mx[1]:.1f} z {mn[2]:.1f}..{mx[2]:.1f}')")
    vol=${v%% *}
    if python3 -c "import sys; sys.exit(0 if $vol < 0.5 else 1)"; then echo "CONTACT    $c (faces touch, ${vol} mm3)"
    else echo "COLLISION  $c: ${vol} mm3, bbox x ${v#* }"; fail=1; fi
  fi
done
exit $fail
