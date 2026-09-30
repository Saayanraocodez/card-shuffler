// Minimal Arduino/ESP32 mock so the hardware modules can be SYNTAX-CHECKED on the host (g++ -fsyntax-only).
// It provides declarations only; nothing here is meant to run.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2
enum { ADC_11db = 3 };
#define ESP_ARDUINO_VERSION_VAL(a,b,c) (((a)<<16)|((b)<<8)|(c))
#define ESP_ARDUINO_VERSION ESP_ARDUINO_VERSION_VAL(3,0,0)
inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int) { return 1; }
inline void delay(unsigned long) {}
inline void delayMicroseconds(unsigned int) {}
inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }
inline int analogRead(int) { return 0; }
inline uint32_t analogReadMilliVolts(int) { return 0; }
inline void analogReadResolution(int) {}
inline void analogSetAttenuation(int) {}
inline void analogSetPinAttenuation(int, int) {}
inline bool ledcAttach(int, uint32_t, uint8_t) { return true; }
inline void ledcWrite(int, uint32_t) {}
inline void ledcWriteTone(int, uint32_t) {}
inline void rgbLedWrite(int, uint8_t, uint8_t, uint8_t) {}
inline uint32_t esp_random() { return 4; }
inline void esp_fill_random(void* p, size_t n) { memset(p, 0, n); }
template <typename T> T constrain(T v, T lo, T hi) { return v < lo ? lo : v > hi ? hi : v; }
inline int constrain(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
inline uint16_t constrain(uint16_t v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }
template <typename T> T min(T a, T b) { return a < b ? a : b; }
template <typename T> T max(T a, T b) { return a > b ? a : b; }
struct SerialClass { void begin(long) {} int available() { return 0; } int read() { return -1; }
    void print(const char*) {} void print(int) {} void print(char) {} void println(const char* = "") {} void println(int) {}
    int printf(const char*, ...) { return 0; } };
extern SerialClass Serial;
