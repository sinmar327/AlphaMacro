#pragma once
#include "lvgl.h"
#include "esp_types.h"

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

// Deklaration der Funktion
macroKey_t* macroScreenInit();
void macroScreenLoad(lv_obj_t* screen, macroKey_t* macros, uint8_t idx);