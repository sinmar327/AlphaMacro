#include "lvgl_screen_linker.h"
#include "lvgl.h"
#include <string.h>
#include "esp_log.h"

static lv_obj_t* mainScreen;
static lv_obj_t* macroScreen;

static macroKey_t* macroKey = NULL;

static char* TAG = "LVGL_SCREEN_LINKER";

bool checkMatrixInit()
{
    return macroKey != NULL;
}

void openMacroScreen(lv_event_t* cb)
{
    lv_event_code_t code = lv_event_get_code(cb);
    lv_obj_t* obj = lv_event_get_target(cb);
    //printf(lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj)));
    if(macroKey == NULL)
        macroScreenInit(lv_buttonmatrix_get_selected_button(obj), macroKey);
    else
        macroScreenLoad(lv_buttonmatrix_get_selected_button(obj));
}



void toMainScreen()
{
    loadMainScreen(macroKey);
}

//Screen Inits

void registerMainScreen(lv_obj_t* screen, macroKey_t* macros)
{
    mainScreen = screen;
    macroKey = macros;
}

void registerMacroScreen(lv_obj_t* screen)
{
    macroScreen = screen;
}