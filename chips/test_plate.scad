// One plate for a borrowed printer: the three poker chip tests + the 608ZZ bearing fit test.
// One pause (see README.md); the bearing coupon just waits during it.
use <poker_chip_test.scad>

chip_set();
translate([-6.5, -50, 0]) import("../cad/stl/bearing_fit_test.stl");
