// mock
#pragma once
#include "Arduino.h"
enum HTTPMethod { HTTP_GET, HTTP_POST };
class WebServer { public:
    explicit WebServer(int) {}
    void on(const char*, HTTPMethod, void (*)()) {} void onNotFound(void (*)()) {} void begin() {} void handleClient() {}
    bool hasArg(const char*) { return false; } String arg(const char*) { return String(); }
    void send(int, const char*, const String&) {} void send_P(int, const char*, const char*) {} };
