#pragma once
#include <stddef.h>
struct Preferences { bool begin(const char*, bool = false) { return true; } void end() {}
    size_t getBytes(const char*, void*, size_t n) { return 0; } size_t putBytes(const char*, const void*, size_t n) { return n; } };
