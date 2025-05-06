#include "lvgl_screen_linker.h"
#include "lvgl.h"
#include <string.h>

static lv_obj_t* mainScreen;
static lv_obj_t* macroScreen;
static macroKey_t* macroKey = NULL;

void openMacroScreen(lv_event_t* cb)
{
    lv_event_code_t code = lv_event_get_code(cb);
    lv_obj_t* obj = lv_event_get_target(cb);
    printf(lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj)));
    if(macroKey == NULL)
        macroKey = macroScreenInit();
    //else
    //    macroScreenLoad(macroScreen, macroKey, strtol(lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj)), NULL, 10));
}



void toMainScreen()
{
    lv_scr_load(mainScreen);
}

//Screen Inits

void registerMainScreen(lv_obj_t* screen)
{
    mainScreen = screen;
}

void registerMacroScreen(lv_obj_t* screen)
{
    macroScreen = screen;
}