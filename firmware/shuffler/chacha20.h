// chacha20.h — ChaCha20 block function per RFC 8439 (portable).
#pragma once
#include <stdint.h>

// Fills out[16] with the keystream words of one block (little-endian word order = RFC byte order).
void chacha20_block(const uint8_t key[32], uint32_t counter, const uint8_t nonce[12], uint32_t out[16]);
