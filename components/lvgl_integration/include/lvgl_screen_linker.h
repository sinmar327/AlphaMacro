#pragma once
#include "lvgl_macro_screen.h"
#include "lvgl.h"

void openMacroScreen(lv_event_t* cb);
void toMainScreen();
void registerMainScreen(lv_obj_t* screen);
void registerMacroScreen(lv_obj_t* screen);
void loadMacroScreen();
