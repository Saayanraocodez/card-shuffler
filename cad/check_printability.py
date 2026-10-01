#!/usr/bin/env python3
"""check_printability.py — slicer-style sanity checks on every STL in stl/, in its exported orientation.

    pip install trimesh shapely scipy networkx rtree
    python3 check_printability.py            # all parts
    python3 check_printability.py wheel      # one part, listing each problem area

Per part:
  * mesh: watertight, consistent winding, positive volume, resting on Z = 0
  * bodies: every separate shell must touch the bed (no floating pieces); sealed internal
    cavities are reported (harmless to print, but usually a modelling slip such as a blind hole
    that never reaches a surface)
  * fits a 220 x 220 mm bed; first-layer contact area
  * support-free printing, checked the way a slicer sees it: the part is sliced every LAYER mm and
    each layer's new area that is not over the previous layer (allowing a 45 deg step) is
      - fine         if every point of it is within LEDGE mm of the layer below (short ledge,
                     45 deg chamfer, top of a small horizontal hole);
      - a bridge     if it is anchored on two or more separate edges and every point is within
                     BRIDGE/2 of an anchor;
      - a PROBLEM    otherwise (cantilever / floating island needing support).
"""
import glob, os, sys
import numpy as np
import trimesh
from shapely.geometry import Polygon, MultiPolygon, GeometryCollection
from shapely.ops import unary_union

BED = 220.0
LAYER = 0.2
LEDGE = 2.5        # mm of unsupported ledge accepted without support
BRIDGE = 25.0      # longest bridge accepted (PETG at sane speed)
HERE = os.path.dirname(os.path.abspath(__file__))


def layer_poly(mesh, z):
    sec = mesh.section(plane_origin=[0, 0, z], plane_normal=[0, 0, 1])
    if sec is None:
        return Polygon()
    planar, T = sec.to_2D()
    polys = [p for p in planar.polygons_full if p.is_valid and p.area > 1e-4]
    if not polys:
        return Polygon()
    g = unary_union(polys)
    # back to world XY (to_2D applies a transform; undo it)
    from shapely import affinity
    M = T  # 4x4 from planar to 3D
    return affinity.affine_transform(g, [M[0, 0], M[0, 1], M[1, 0], M[1, 1], M[0, 3], M[1, 3]])


def parts(g):
    if g.is_empty:
        return []
    if isinstance(g, (MultiPolygon, GeometryCollection)):
        return [p for p in g.geoms if isinstance(p, Polygon) and not p.is_empty]
    return [g] if isinstance(g, Polygon) else []


DIRS = [np.array([np.cos(a), np.sin(a)]) for a in np.radians(np.arange(0, 180, 15))]


def is_bridge(isl, sup):
    from shapely.geometry import LineString, Point
    minx, miny, maxx, maxy = isl.bounds
    rng = np.random.default_rng(1)
    pts = [isl.representative_point()]
    while len(pts) < 40:
        p = Point(rng.uniform(minx, maxx), rng.uniform(miny, maxy))
        if isl.contains(p):
            pts.append(p)
        if len(pts) < 40 and rng.random() < 0.002:
            break
    L = BRIDGE + 1
    good, bad = [], []
    for p in pts:
        ok = False
        for d in DIRS:
            tot = 0.0
            for sgn in (1, -1):
                ray = LineString([(p.x, p.y), (p.x + sgn * d[0] * L, p.y + sgn * d[1] * L)])
                hit = ray.intersection(sup)
                if hit.is_empty:
                    tot = np.inf
                    break
                tot += p.distance(hit)
            if tot <= BRIDGE:
                ok = True
                break
        (good if ok else bad).append(p)
    # a point that is not itself on a straight bridge but lies within LEDGE of one is the bridge's
    # edge (e.g. the curved rim of a disc between fin tips): fine
    return all(good and min(q.distance(g) for g in good) <= LEDGE for q in bad)


def support_problems(mesh):
    zmax = mesh.bounds[1][2]
    zs = np.arange(0.1371, zmax, LAYER)        # off the round numbers the CAD uses, so no slice lands on a face
    prev = None
    bad = []
    step = LAYER  # 45 deg: each layer may step out by one layer height
    for z in zs:
        cur = layer_poly(mesh, z)
        if prev is not None and not cur.is_empty:
            sup = prev.buffer(step + 0.05)
            new = cur.difference(sup)
            for isl in parts(new):
                if isl.area < 0.5:
                    continue
                # short ledge: everything within LEDGE of the supported area
                if isl.difference(prev.buffer(LEDGE)).area < 0.5:
                    continue
                # bridge: through every point there is a line that meets the layer below on BOTH sides
                # within BRIDGE in total (a hole roof, a slot ceiling, a beam between two walls)
                if is_bridge(isl, prev.buffer(0.3)):
                    continue
                n_anchor = 0
                c = isl.centroid
                bad.append((z, isl.area, c.x, c.y, n_anchor))
        prev = cur if not cur.is_empty else prev
    return bad


def check(path, verbose=False):
    m = trimesh.load(path)
    name = os.path.basename(path)[:-4]
    ext = m.extents
    problems, notes = [], []
    if not m.is_watertight:
        problems.append("not watertight")
    if not m.is_winding_consistent:
        problems.append("inconsistent winding")
    if m.volume <= 0:
        problems.append("negative volume")
    if abs(m.bounds[0][2]) > 1e-3:
        problems.append(f"not on the bed (zmin {m.bounds[0][2]:.3f})")
    if sorted(ext[:2])[1] > BED:
        problems.append("larger than a 220 mm bed")
    bodies = m.split(only_watertight=False)
    solids = [b for b in bodies if b.volume > 0]
    voids = [b for b in bodies if b.volume <= 0]
    floating = [b for b in solids if b.bounds[0][2] > 0.01]
    if floating:
        problems.append(f"{len(floating)} body(ies) not touching the bed")
    if voids:
        notes.append(f"{len(voids)} sealed cavity(ies)")
    nz = m.face_normals[:, 2]
    contact = m.area_faces[(nz < -0.999) & (m.triangles_center[:, 2] < 0.01)].sum()
    bad = support_problems(m)
    if bad:
        problems.append(f"{len(bad)} unsupported area(s) on {len({round(b[0], 1) for b in bad})} layer(s)")
    print(f"{name:24s} {ext[0]:5.0f} x {ext[1]:4.0f} x {ext[2]:4.0f}  {m.volume/1000:6.1f} cm3  contact {contact:6.0f} mm2  "
          + ("OK" if not problems else "CHECK: " + "; ".join(problems)) + ("  (" + "; ".join(notes) + ")" if notes else ""))
    if verbose:
        for b in floating:
            print(f"    floating body  bounds {np.round(b.bounds, 1).tolist()}")
        for v in voids:
            print(f"    sealed cavity  bounds {np.round(v.bounds, 1).tolist()}")
        for z, a, x, y, n in bad[:60]:
            print(f"    unsupported {a:7.1f} mm2 at z {z:6.1f}  centre ({x:6.1f}, {y:6.1f})  anchors {n}")
    return not problems


if __name__ == "__main__":
    want = sys.argv[1:]
    files = sorted(glob.glob(os.path.join(HERE, "stl", "*.stl")))
    if want:
        files = [f for f in files if os.path.basename(f)[:-4] in want]
    ok = True
    for f in files:
        ok &= check(f, verbose=bool(want))
    print("ALL PARTS OK" if ok else "SOME PARTS NEED A LOOK")
    sys.exit(0 if ok else 1)
