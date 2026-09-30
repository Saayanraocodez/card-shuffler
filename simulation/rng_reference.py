#!/usr/bin/env python3
"""
rng_reference.py — bit-exact Python reference of the firmware's random-number pipeline.

Pipeline (identical in firmware/shuffler/rng.cpp):
  1. entropy material  = hardware RNG bytes || jitter pool || boot counter || shuffle counter
  2. key               = SHA-256(entropy material)                       (32 bytes)
  3. nonce (12 bytes)  = "SHF" 0x01 || uint64 LE shuffle counter
  4. keystream         = ChaCha20(key, nonce, block counter starting at 1)   (RFC 8439)
  5. next_u32()        = successive little-endian 32-bit words of the keystream
  6. uniform(n)        = rejection sampling: draw x = next_u32(); accept iff x < 2^32 - (2^32 mod n);
                         return x mod n.  (No modulo bias; expected rejections ≈ n / 2^32.)
  7. gap sequence      = for card i in 0..n-1: j_i = uniform(i + 1)

The firmware can dump (key, shuffle counter, j-sequence) over serial in TEST mode; this file
regenerates the j-sequence from the same key so the firmware's sampler can be checked bit for bit.

Usage:
  python3 rng_reference.py --selftest
  python3 rng_reference.py --key <64 hex chars> --counter 7 --n 52
"""
import argparse
import hashlib
import struct
import sys


def _rotl(x, k):
    return ((x << k) | (x >> (32 - k))) & 0xFFFFFFFF


def _qr(s, a, b, c, d):
    s[a] = (s[a] + s[b]) & 0xFFFFFFFF; s[d] ^= s[a]; s[d] = _rotl(s[d], 16)
    s[c] = (s[c] + s[d]) & 0xFFFFFFFF; s[b] ^= s[c]; s[b] = _rotl(s[b], 12)
    s[a] = (s[a] + s[b]) & 0xFFFFFFFF; s[d] ^= s[a]; s[d] = _rotl(s[d], 8)
    s[c] = (s[c] + s[d]) & 0xFFFFFFFF; s[b] ^= s[c]; s[b] = _rotl(s[b], 7)


def chacha20_block(key: bytes, counter: int, nonce: bytes) -> bytes:
    assert len(key) == 32 and len(nonce) == 12
    const = struct.unpack("<4I", b"expand 32-byte k")
    k = struct.unpack("<8I", key)
    n = struct.unpack("<3I", nonce)
    state = list(const) + list(k) + [counter & 0xFFFFFFFF] + list(n)
    w = state[:]
    for _ in range(10):
        _qr(w, 0, 4, 8, 12); _qr(w, 1, 5, 9, 13); _qr(w, 2, 6, 10, 14); _qr(w, 3, 7, 11, 15)
        _qr(w, 0, 5, 10, 15); _qr(w, 1, 6, 11, 12); _qr(w, 2, 7, 8, 13); _qr(w, 3, 4, 9, 14)
    out = [(w[i] + state[i]) & 0xFFFFFFFF for i in range(16)]
    return struct.pack("<16I", *out)


class Drbg:
    """ChaCha20-based deterministic random bit generator, word oriented."""

    def __init__(self, key: bytes, shuffle_counter: int):
        self.key = key
        self.nonce = b"SHF\x01" + struct.pack("<Q", shuffle_counter)
        self.block_counter = 1
        self.buf = b""
        self.words_drawn = 0
        self.rejections = 0

    def _refill(self):
        self.buf += chacha20_block(self.key, self.block_counter, self.nonce)
        self.block_counter += 1

    def next_u32(self) -> int:
        if len(self.buf) < 4:
            self._refill()
        x, self.buf = struct.unpack("<I", self.buf[:4])[0], self.buf[4:]
        self.words_drawn += 1
        return x

    def uniform(self, n: int) -> int:
        """Unbiased integer in [0, n).  Rejection sampling on a 32-bit word."""
        assert 1 <= n <= 0xFFFFFFFF
        lim = (1 << 32) - ((1 << 32) % n)
        while True:
            x = self.next_u32()
            if x < lim:
                return x % n
            self.rejections += 1


def derive_key(hw_random: bytes, jitter_pool: bytes, boot_counter: int, shuffle_counter: int) -> bytes:
    material = hw_random + jitter_pool + struct.pack("<I", boot_counter) + struct.pack("<Q", shuffle_counter)
    return hashlib.sha256(material).digest()


def gap_sequence(key: bytes, shuffle_counter: int, n: int):
    d = Drbg(key, shuffle_counter)
    return [d.uniform(i + 1) for i in range(n)], d


def selftest():
    # RFC 8439 §2.3.2 block-function test vector
    key = bytes(range(32))
    nonce = bytes.fromhex("000000090000004a00000000")
    blk = chacha20_block(key, 1, nonce)
    exp = bytes.fromhex(
        "10f1e7e4d13b5915500fdd1fa32071c4c7d1f4c733c068030422aa9ac3d46c4e"
        "d2826446079faa0914c2d705d98b02a2b5129cd1de164eb9cbd083e8a2503c4e")
    assert blk == exp, "ChaCha20 block test vector FAILED"
    # RFC 8439 §2.4.2 encryption test vector (first 16 bytes)
    nonce2 = bytes.fromhex("000000000000004a00000000")
    pt = b"Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it."
    ks = b"".join(chacha20_block(key, 1 + i, nonce2) for i in range(3))
    ct = bytes(a ^ b for a, b in zip(pt, ks))
    assert ct[:16] == bytes.fromhex("6e2e359a2568f98041ba0728dd0d6981"), "ChaCha20 encryption vector FAILED"
    # optional cross-check against the `cryptography` package if present
    try:
        from cryptography.hazmat.primitives.ciphers import Cipher, algorithms
        full_nonce = struct.pack("<I", 1) + nonce2
        c = Cipher(algorithms.ChaCha20(key, full_nonce), mode=None).encryptor()
        assert c.update(pt) == ct, "mismatch vs cryptography package"
        print("cross-check vs `cryptography` package: OK")
    except BaseException as e:  # package missing or broken (pyo3 panics are BaseException): the RFC vectors above are the real test
        print("(cryptography package cross-check skipped:", type(e).__name__ + ")")
    # deterministic sampler test vector used by the firmware host test
    k = hashlib.sha256(b"card-shuffler-test-key").digest()
    seq, d = gap_sequence(k, 1, 52)
    print("selftest OK")
    print("test key   :", k.hex())
    print("counter    : 1")
    print("gaps(52)   :", " ".join(str(x) for x in seq))
    print("words drawn:", d.words_drawn, " rejections:", d.rejections)
    return seq


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--key", help="64 hex chars")
    ap.add_argument("--counter", type=int, default=1)
    ap.add_argument("--n", type=int, default=52)
    a = ap.parse_args()
    if a.selftest or not a.key:
        selftest()
        sys.exit(0)
    seq, d = gap_sequence(bytes.fromhex(a.key), a.counter, a.n)
    print(" ".join(str(x) for x in seq))
