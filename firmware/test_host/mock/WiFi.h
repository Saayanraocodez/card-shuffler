// mock
#pragma once
#include "Arduino.h"
#define WIFI_AP 2
struct IPAddress { String toString() const { return String("192.168.4.1"); } };
struct WiFiClass { void mode(int) {} bool softAP(const char*, const char*) { return true; } IPAddress softAPIP() { return IPAddress(); } };
extern WiFiClass WiFi;
