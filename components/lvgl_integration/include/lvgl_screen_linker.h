#pragma once
#include "lvgl_macro_screen.h"
#include "lvgl.h"
#include "rgbKeyboardTypes.h"
#include "lvgl_intergration_types.h"


void openMacroScreen(lv_event_t* cb);
void toMainScreen();
void registerMainScreen(lv_obj_t* screen, macroKey_t* macros);
void registerMacroScreen(lv_obj_t* screen);
void loadMacroScreen();
void loadMainScreen(macroKey_t* macros);
