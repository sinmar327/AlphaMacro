#pragma once
#include "lvgl.h"
#include "esp_types.h"
#include "lvgl_intergration_types.h"


// Deklaration der Funktion
void macroScreenInit(uint8_t idx, macroKey_t* macros);
void macroScreenLoad(uint8_t idx);
macroKey_t* initMacroKeyArray(const uint8_t size, const uint8_t macroCount);