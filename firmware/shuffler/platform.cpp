// platform.cpp
#include "platform.h"
#include "config.h"
#include "esp_system.h"
#if __has_include("esp_random.h")
#include "esp_random.h"
#endif
#if USE_BOOTLOADER_RANDOM
extern "C" {
#include "bootloader_random.h"
}
#endif

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
// ---- core 3.x: pin-based LEDC API
void pwm_attach(int pin, uint32_t freq, uint8_t bits) { ledcAttach(pin, freq, bits); }
void pwm_write(int pin, uint32_t duty) { ledcWrite(pin, duty); }
void tone_start(int pin, uint32_t freq) { ledcWriteTone(pin, freq); }
void tone_stop(int pin) { ledcWriteTone(pin, 0); }
void pixel_write(int pin, uint8_t r, uint8_t g, uint8_t b) { rgbLedWrite(pin, r, g, b); }
#else
// ---- core 2.x: channel-based LEDC API; allocate channels in order of attachment
static int s_chan_of_pin[48];
static int s_next_chan = 0;
static int chan(int pin) { return s_chan_of_pin[pin]; }
void pwm_attach(int pin, uint32_t freq, uint8_t bits) {
    int c = s_next_chan++; s_chan_of_pin[pin] = c;
    ledcSetup(c, freq, bits); ledcAttachPin(pin, c);
}
void pwm_write(int pin, uint32_t duty) { ledcWrite(chan(pin), duty); }
void tone_start(int pin, uint32_t freq) { ledcWriteTone(chan(pin), freq); }
void tone_stop(int pin) { ledcWriteTone(chan(pin), 0); }
void pixel_write(int pin, uint8_t r, uint8_t g, uint8_t b) { neopixelWrite(pin, r, g, b); }
#endif

void adc_setup() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);           // 0..~3.1 V full scale
    analogSetPinAttenuation(PIN_BEAM_B, ADC_11db);
    analogSetPinAttenuation(PIN_BEAM_E, ADC_11db);
    analogSetPinAttenuation(PIN_BEAM_S, ADC_11db);
    analogSetPinAttenuation(PIN_BEAM_X, ADC_11db);
    analogSetPinAttenuation(PIN_VBAT, ADC_11db);
}

void hw_random_bytes(uint8_t* out, size_t n) {
#if USE_BOOTLOADER_RANDOM
    // Espressif: esp_random() is true-random only while the RF subsystem or this ADC-noise source is
    // enabled. The ADC must not be used by the application while the source is enabled, so we enable,
    // draw, disable, and then re-initialise the ADC for the beam sensors.
    bootloader_random_enable();
    delayMicroseconds(200);                   // let the noise source run before the first word
    esp_fill_random(out, n);
    bootloader_random_disable();
    adc_setup();
#else
    esp_fill_random(out, n);                  // pseudo-random unless Wi-Fi/BT is running: NOT recommended
#endif
}
