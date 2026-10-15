#!/bin/bash
# Runs every collision check in dealer_check.scad (turntable and launcher). Prints CLEAR or OVERLAP <mm3>.
cd "$(dirname "$0")"
T=$(mktemp -d)
for c in arm_chute roller_chute roller_arm motor_chute arm_bases chute_bases tt_rot_foot tt_mesh tt_motor_base; do
  out=$(openscad -D "check=\"$c\"" -o "$T/$c.stl" dealer_check.scad 2>&1)
  if echo "$out" | grep -q "empty"; then printf "%-16s CLEAR\n" "$c"
  else v=$(python3 -c "import trimesh;print(round(abs(trimesh.load('$T/$c.stl').volume),3))" 2>/dev/null); printf "%-16s OVERLAP %s mm3\n" "$c" "$v"; fi
done
rm -rf "$T"
