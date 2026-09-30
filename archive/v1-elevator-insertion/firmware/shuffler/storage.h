// storage.h — calibration record in NVS (Preferences).
#pragma once
#include "config.h"
void cal_defaults(Calibration& c);
void cal_load(Calibration& c);     // defaults if none stored
void cal_save(const Calibration& c);
void cal_print(const Calibration& c);
