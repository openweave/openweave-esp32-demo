#ifndef DISPLAY_H
#define DISPLAY_H

#include "esp_system.h"

extern "C" {
#include "tftspi.h"
#include "tft.h"
} // extern "C"

extern bool HaveDisplay;
extern uint16_t DisplayHeight;
extern uint16_t DisplayWidth;

extern esp_err_t InitDisplay();

#endif // DISPLAY_H
