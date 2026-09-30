// test_core.cpp — host-side unit test for the platform-independent firmware core (wheel version).
//   make run
// Checks: SHA-256 vector, ChaCha20 RFC 8439 block vector, the slot assignment against
// simulation/rng_reference.py --selftest, the predicted output order, sampler uniformity,
// the rejection path, and the entropy health test.
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include "sha256.h"
#include "chacha20.h"
#include "rng.h"
#include "shuffle_core.h"

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } else printf("ok:   %s\n", msg); } while (0)
static std::string hex(const uint8_t* p, size_t n) { char b[3]; std::string s; for (size_t i = 0; i < n; i++) { snprintf(b, 3, "%02x", p[i]); s += b; } return s; }

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    uint8_t h[32]; sha256((const uint8_t*)"abc", 3, h);
    CHECK(hex(h, 32) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256 test vector");

    uint8_t key[32]; for (int i = 0; i < 32; i++) key[i] = i;
    uint8_t nonce[12] = {0,0,0,9,0,0,0,0x4a,0,0,0,0};
    uint32_t blk[16]; chacha20_block(key, 1, nonce, blk);
    CHECK(blk[0] == 0xe4e7f110 && blk[1] == 0x15593bd1 && blk[15] == 0x4e3c50a2, "ChaCha20 RFC 8439 block vector");

    uint8_t tkey[32]; sha256((const uint8_t*)"card-shuffler-test-key", 22, tkey);
    CHECK(hex(tkey, 32) == "1b5093f405094119166c8d4b01f0d753525f4939e192e07e88c6a3993a126870", "test key derivation");

    // wheel slot assignment vs Python reference
    Drbg d; drbg_seed_key(&d, tkey, 1);
    uint8_t slots[MAX_CARDS]; CHECK(slots_assign(&d, 52, 54, slots), "slots_assign accepts 52 cards / 54 slots");
    const int ref_slots[52] = {3,23,40,50,29,13,47,52,22,5,39,43,36,25,20,2,15,53,51,37,11,8,26,16,6,34,1,48,42,4,9,0,49,12,32,17,44,14,45,31,10,38,41,33,27,19,35,46,30,28,21,24};
    bool same = true; for (int i = 0; i < 52; i++) same &= slots[i] == ref_slots[i];
    CHECK(same, "slot assignment matches simulation/rng_reference.py");
    CHECK(d.words_drawn == 52 && d.rejections == 0, "52 words drawn, 0 rejections");
    uint8_t order[MAX_CARDS]; predict_order_from_slots(52, slots, order);
    const int ref_order[52] = {31,26,15,0,29,9,24,21,30,40,20,33,5,37,16,23,35,45,14,50,8,1,51,13,22,44,49,4,48,39,34,43,25,46,12,19,41,10,2,42,28,11,36,38,47,6,27,32,3,18,7,17};
    same = true; for (int i = 0; i < 52; i++) same &= order[i] == ref_order[i];
    CHECK(same, "predicted output order matches simulation/rng_reference.py");

    // every assignment is injective and within range, 54 cards fill all slots
    Drbg d2; drbg_seed_key(&d2, tkey, 7); bool ok = true;
    for (int r = 0; r < 3000; r++) { uint8_t sl[MAX_CARDS]; ok &= slots_assign(&d2, 54, 54, sl); bool seen[54] = {false};
        for (int i = 0; i < 54; i++) { ok &= sl[i] < 54; ok &= !seen[sl[i]]; seen[sl[i]] = true; } }
    CHECK(ok, "3000 full assignments are permutations of 54 slots");
    CHECK(!slots_assign(&d2, 55, 54, slots), "rejects more cards than slots");

    // sampler uniformity
    Drbg d3; drbg_seed_key(&d3, tkey, 99); long cnt[54] = {0}; const long N = 5400000;
    for (long i = 0; i < N; i++) cnt[drbg_uniform(&d3, 54)]++;
    double chi = 0, e = (double)N / 54; for (int i = 0; i < 54; i++) chi += (cnt[i] - e) * (cnt[i] - e) / e;
    printf("      chi2(53 df) = %.1f  (expect ~53 ± 10; >91 would be p<0.001)\n", chi);
    CHECK(chi < 91.0, "uniform(54) chi-square within bounds");

    // first-slot marginal: P(card 0 in slot s) = 1/54 for every s over 540000 shuffles (uses only the first draw)
    Drbg d5; drbg_seed_key(&d5, tkey, 3); long c0[54] = {0};
    for (long r = 0; r < 540000; r++) c0[drbg_uniform(&d5, 54)]++;
    double chi0 = 0; for (int i = 0; i < 54; i++) chi0 += (c0[i] - 10000.0) * (c0[i] - 10000.0) / 10000.0;
    CHECK(chi0 < 91.0, "first-card slot marginal uniform");

    Drbg d4; drbg_seed_key(&d4, tkey, 5); uint32_t n = 0xC0000000u;
    for (int i = 0; i < 100000; i++) { uint32_t x = drbg_uniform(&d4, n); if (x >= n) { printf("out of range\n"); fails++; } }
    printf("      rejections for n=0xC0000000: %lu of 100000 accepted (expect ~33333)\n", (unsigned long)d4.rejections);
    CHECK(d4.rejections > 30500 && d4.rejections < 36500, "rejection path behaves");

    uint8_t hw[64]; memset(hw, 0x5a, 64);
    CHECK(!rng_health_ok(hw), "health test rejects a stuck source");
    for (int i = 0; i < 64; i++) hw[i] = (uint8_t)(i * 37 + 11);
    CHECK(rng_health_ok(hw), "health test accepts a varying source");
    uint8_t jit[32] = {0}; Drbg a, b, c;
    drbg_seed(&a, hw, jit, 1, 10); drbg_seed(&b, hw, jit, 1, 10); drbg_seed(&c, hw, jit, 1, 11);
    CHECK(memcmp(a.key, b.key, 32) == 0 && memcmp(a.key, c.key, 32) != 0, "seed derivation deterministic and counter-sensitive");

    printf("%s (%d failures)\n", fails ? "SOME TESTS FAILED" : "ALL TESTS PASSED", fails);
    return fails ? 1 : 0;
}
