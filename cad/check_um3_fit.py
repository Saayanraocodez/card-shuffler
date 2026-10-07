#!/usr/bin/env python3
"""check_um3_fit.py — does every STL's first layer fit an Ultimaker 3 build plate as Cura lays it out?

Plate model, from Cura 5.8 resources/definitions/ultimaker3.def.json (centred coordinates, mm):
  * machine area 233 x 215; print core 1 reaches x <= 98.5 (215 wide: core 2 sits 18 mm to its right);
  * machine_disallowed_areas: the glass clips (CLIPS below);
  * edge keep-out around the plate and the clips: Cura uses max(travel_avoid_distance, adhesion width).
    travel_avoid_distance is 3 mm whenever both print cores are enabled (the normal UM3 setup),
    and the brim adds ~3.5 mm. Both cases are checked.
Each part is placed (rotated near 0/90/180/270 degrees, translated in 0.5 mm steps) to maximise its
clearance; the report gives that clearance or NO FIT.

  pip install trimesh shapely scipy networkx rtree && python3 check_um3_fit.py [part ...]
"""
import sys
from pathlib import Path

import numpy as np
import trimesh
from shapely import affinity
from shapely.geometry import Polygon, box
from shapely.ops import unary_union

CLIPS = [Polygon(p) for p in (
    [(92.8, -53.4), (92.8, -97.5), (116.5, -97.5), (116.5, -53.4)],
    [(73.8, 107.5), (73.8, 100.5), (116.5, 100.5), (116.5, 107.5)],
    [(74.6, 107.5), (74.6, 100.5), (116.5, 100.5), (116.5, 107.5)],
    [(74.9, -97.5), (74.9, -107.5), (116.5, -107.5), (116.5, -97.5)],
    [(-116.5, -103.5), (-116.5, -107.5), (-100.9, -107.5), (-100.9, -103.5)],
    [(-116.5, 105.8), (-96.9, 105.8), (-96.9, 107.5), (-116.5, 107.5)],
)]
PLATE = (-116.5, -107.5, 98.5, 107.5)
CASES = [("both cores", 3.0), ("with brim", 3.5)]
STL = Path(__file__).parent / "stl"


def free_area(border):
    return box(*PLATE).buffer(-border, join_style=2).difference(unary_union([c.buffer(border, join_style=2) for c in CLIPS]))


def footprint(path):
    m = trimesh.load(path, force="mesh")
    sec = m.section(plane_origin=[0, 0, 0.1], plane_normal=[0, 0, 1])
    flat, _ = sec.to_2D(to_2D=np.eye(4))
    return unary_union([Polygon(p.exterior) for p in flat.polygons_full])   # holes cannot make a fit worse


CORE_CENTRE = (-21.35, 1.15)   # middle of the largest clip-free rectangle (x -116.5..73.8, y -103.5..105.8)


def best_clearance(fp, free):
    for ang in (0, 90):            # small parts: centred placement already clears everything by > 5 mm
        r = affinity.rotate(fp, ang, origin="centroid")
        x0, y0, x1, y1 = r.bounds
        t = affinity.translate(r, CORE_CENTRE[0] - (x0 + x1) / 2, CORE_CENTRE[1] - (y0 + y1) / 2)
        if free.contains(t) and free.boundary.distance(t) > 5:
            return free.boundary.distance(t)
    best = None
    for ang in [a + d for a in (0, 90, 180, 270) for d in range(-6, 7)]:
        r = affinity.rotate(fp, ang, origin="centroid")
        x0, y0, x1, y1 = r.bounds
        for x in np.arange(PLATE[0] - x0, PLATE[2] - x1 + 1e-9, 0.5):
            for y in np.arange(PLATE[1] - y0, PLATE[3] - y1 + 1e-9, 0.5):
                t = affinity.translate(r, x, y)
                if free.contains(t):
                    c = free.boundary.distance(t)
                    if best is None or c > best:
                        best = c
        if best is not None and best > 5:   # plenty of room: no need to search further
            break
    return best


def main(names):
    frees = [(label, free_area(b)) for label, b in CASES]
    print(f"{'part':24s} {'first layer':>13s}  " + "  ".join(f"{label:>11s}" for label, _ in frees))
    ok = True
    for name in names:
        fp = footprint(STL / f"{name}.stl")
        x0, y0, x1, y1 = fp.bounds
        res = [best_clearance(fp, free) for _, free in frees]
        ok &= all(r is not None for r in res)
        cells = "  ".join(("NO FIT" if r is None else "> 5 mm" if r > 5 else f"{r:.2f} mm").rjust(11) for r in res)
        print(f"{name:24s} {x1 - x0:5.1f} x {y1 - y0:5.1f}  {cells}")
    print("ALL PARTS FIT" if ok else "SOME PARTS DO NOT FIT")
    return 0 if ok else 1


if __name__ == "__main__":
    parts = sys.argv[1:] or sorted(p.stem for p in STL.glob("*.stl"))
    sys.exit(main(parts))
