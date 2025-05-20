#include "lvgl_integration.h"
#include "lvgl_screen_linker.h"
#include "esp_check.h"

static int32_t mainScreenGrid_c[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), 40, LV_GRID_TEMPLATE_LAST};
static int32_t mainScreenGrid_r[] = {40, LV_GRID_FR(2), LV_GRID_FR(2), LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

static const char* TAG = "MAIN_SCREEN";

static macroKey_t* macroMatrix = NULL;
static lv_obj_t* screen;

void draw_btnmatrix_cb(lv_event_t * e)
{
    if(macroMatrix == NULL) return;

    lv_obj_t * obj = lv_event_get_target(e);
    lv_draw_task_t * draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t * base_dsc = lv_draw_task_get_draw_dsc(draw_task);

    // We're only interested in the button cells
    if (base_dsc->part != LV_PART_ITEMS) return;
    //ESP_LOGI(TAG, "%i", base_dsc->id1);
    bool pressed = false;
    if(lv_buttonmatrix_get_selected_button(obj) == base_dsc->id1 && lv_obj_has_state(obj, LV_STATE_PRESSED))
        pressed = true;
    // Example: Change color of button with index
    lv_draw_fill_dsc_t * fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
    if(fill_draw_dsc) {
        fill_draw_dsc->radius = 0;
        if(pressed) fill_draw_dsc->color = lv_color_darken(macroMatrix[base_dsc->id1].keyColor, 50);
        else fill_draw_dsc->color = macroMatrix[base_dsc->id1].keyColor;
    }
    lv_draw_box_shadow_dsc_t * box_shadow_draw_dsc = lv_draw_task_get_box_shadow_dsc(draw_task);
    if(box_shadow_draw_dsc) {
        box_shadow_draw_dsc->width = 6;
        box_shadow_draw_dsc->ofs_x = 3;
        box_shadow_draw_dsc->ofs_y = 3;
    }
    lv_draw_label_dsc_t * label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
    if(label_draw_dsc) {
        if(lv_color_brightness(macroMatrix[base_dsc->id1].keyColor) < 128)
            label_draw_dsc->color = lv_color_white();
        else
            label_draw_dsc->color = lv_color_black();
    }
}

lv_obj_t* lvglStartup(lv_disp_t* disp)
{
    macroMatrix = initMacroKeyArray(20,10);
    //esp_timer_handle_t wdt_task;
    //const esp_timer_create_args_t wdt_task_create_args = {
    //    .name = "lv_tick_inc_task",
    //    .callback = mainScreen_wdt_tick,
    //};
    //esp_timer_create(&wdt_task_create_args, &wdt_task);
    //esp_timer_start_periodic(wdt_task, 1000);
    lv_lock();
    lv_display_set_rotation(disp, LV_DISP_ROTATION_270);
    lv_color_t backgroundColor = lv_color_make(100, 100, 100);
    lv_color_t backgroundColor2 = lv_color_make(200, 200, 200);
    lv_theme_t* th = lv_theme_default_init(disp, backgroundColor, backgroundColor2, true, LV_FONT_DEFAULT);

    lv_display_set_theme(disp, th);
    //lv_display_set_rotation(disp, LV_DISP_ROTATION_270);
    //lv_obj_t* topLayer = lv_screen_active();
    lv_obj_t* mainScreen = lv_obj_create(NULL);
    lv_screen_load(mainScreen);
    //lv_obj_del(topLayer);


    /*lv_obj_set_style_bg_color(lv_screen_active(), lv_palette_lighten(LV_PALETTE_RED, 5), 0);
    LV_DRAW_BUF_DEFINE_STATIC(draw_buf, 480, 320, LV_COLOR_FORMAT_RGB565);
    LV_DRAW_BUF_INIT_STATIC(draw_buf);
    lv_obj_t* background = lv_canvas_create(mainScreen);
    lv_canvas_set_draw_buf(background, &draw_buf);
    lv_obj_align(background, LV_ALIGN_CENTER, 0, 0);
    lv_canvas_fill_bg(background, backgroundColor, 0);


    lv_obj_set_size(background, 480, 320); //swapped because of display rotation*/
    lv_obj_t* mainScreenGrid = lv_obj_create(mainScreen);
    lv_obj_center(mainScreenGrid);
    lv_obj_set_size(mainScreenGrid, 480, 320);
    lv_obj_set_layout(mainScreenGrid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(mainScreenGrid, mainScreenGrid_c, mainScreenGrid_r);
    lv_obj_t* buttonMatrix = lv_buttonmatrix_create(mainScreenGrid);
    static const char* buttonMap[] = {
        "1", "2", "3", "4", "5", "\n",
        "6", "7", "8", "9", "10", "\n",
        "11", "12", "13", "14", "15", "\n",
        "16", "17", "18", "19", "20", "",
    };
    lv_buttonmatrix_set_map(buttonMatrix, buttonMap);

    lv_obj_add_event_cb(buttonMatrix, draw_btnmatrix_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);
    lv_obj_add_flag(buttonMatrix, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    lv_obj_set_grid_cell(buttonMatrix, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_STRETCH, 1, 5);
    lv_obj_t* statusBar = lv_label_create(mainScreenGrid);
    lv_obj_set_grid_cell(statusBar, LV_GRID_ALIGN_STRETCH, 0, 6, LV_GRID_ALIGN_START, 0, 1);
    lv_label_set_text(statusBar, "Status Bar");
    lv_obj_add_event_cb(buttonMatrix, openMacroScreen, LV_EVENT_VALUE_CHANGED, NULL);
    lv_unlock();
    registerMainScreen(mainScreen, macroMatrix);
    screen = mainScreen;
    return mainScreen;
}

void loadMainScreen()
{
    ESP_LOGI(TAG, "Returning to main screen");
    lv_screen_load(screen);
}