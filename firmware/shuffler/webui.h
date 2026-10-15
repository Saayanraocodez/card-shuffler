// webui.h — Wi-Fi access point and the phone page (DEALER build). Join "Shuffler-XXXX", open http://192.168.4.1
//
// The page only edits settings and queues commands; the main loop runs them (one at a time). While the machine is
// busy the page still shows status and its Stop button works; other commands are refused with "busy".
// Nothing here can reveal the shuffle: the machine never knows which card is which, and the page never sees the
// RNG key or the slot plan.
#pragma once
#include <Arduino.h>
#include "config.h"
#if DEALER
enum WebCmd : uint8_t { WC_NONE = 0, WC_DEAL, WC_SHUFFLE, WC_STREET, WC_FINISH, WC_UNLOAD, WC_TT_HOME, WC_JOG,
                        WC_AIM_SEAT, WC_AIM_DEALER, WC_SET_SEAT, WC_SET_DEALER, WC_TEST };
struct WebRequest { WebCmd cmd; int seat; float value; };
void web_init();                       // starts the access point (WIFI_ENABLE) and the server
void web_poll();                       // serve pending HTTP requests; call often, also from inside long operations
bool web_take(WebRequest* r);          // the main loop collects a queued command
bool web_rf_on();                      // Wi-Fi radio running (the RNG then uses esp_random() without the ADC source)
#endif
