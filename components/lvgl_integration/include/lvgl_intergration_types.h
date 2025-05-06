#pragma once
#include "esp_types.h"
#include "lvgl.h"

typedef struct {
    char** macroArray;
    uint32_t* delayArrayMs;
    uint8_t mode;
    uint16_t modeData;
} macro_t;

typedef struct {
    macro_t macro;
    lv_color_t keyColor;
} macroKey_t;
