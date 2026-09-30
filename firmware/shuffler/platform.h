// platform.h — thin shims over the Arduino-ESP32 core (works with core 2.x and 3.x).
#pragma once
#include <Arduino.h>
#include <stdint.h>

void pwm_attach(int pin, uint32_t freq, uint8_t bits);   // one LEDC channel per pin
void pwm_write(int pin, uint32_t duty);
void tone_start(int pin, uint32_t freq);
void tone_stop(int pin);
void pixel_write(int pin, uint8_t r, uint8_t g, uint8_t b);

// Fills `out` with n bytes from the ESP32 hardware RNG with the ADC-noise entropy source enabled
// for the duration of the call (bootloader_random_enable/disable). Re-initialises the ADC afterwards.
void hw_random_bytes(uint8_t* out, size_t n);
void adc_setup();
