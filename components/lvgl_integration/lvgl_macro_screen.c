#include "lvgl_macro_screen.h"
#include "lvgl_screen_linker.h"
#include "lvgl.h"



static int32_t macroScreenGrid_c[] = { LV_GRID_FR(2), LV_GRID_FR(4), LV_GRID_FR(2), LV_GRID_FR(4), LV_GRID_FR(3), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static int32_t macroScreenGrid_r[] = { 30, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), 30,  LV_GRID_TEMPLATE_LAST};
static macroKey_t* macroKey;

static void draw_event_cb(lv_event_t* e)
{
    lv_draw_task_t* draw_task = lv_event_get_draw_task(e);
    lv_draw_dsc_base_t* base_dsc = lv_draw_task_get_draw_dsc(draw_task);
    /*If the cells are drawn...*/
    if (base_dsc->part == LV_PART_ITEMS) {
        uint32_t row = base_dsc->id1;
        uint32_t col = base_dsc->id2;

        /*Make the texts in the first cell center aligned*/
        if (row == 0) {
            lv_draw_label_dsc_t* label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if (label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_CENTER;
            }
            lv_draw_fill_dsc_t* fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if (fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), fill_draw_dsc->color, LV_OPA_20);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
        /*In the first column align the texts to the right*/
        else if (col == 0) {
            lv_draw_label_dsc_t* label_draw_dsc = lv_draw_task_get_label_dsc(draw_task);
            if (label_draw_dsc) {
                label_draw_dsc->align = LV_TEXT_ALIGN_RIGHT;
            }
        }

        /*Make every 2nd row grayish*/
        if ((row != 0 && row % 2) == 0) {
            lv_draw_fill_dsc_t* fill_draw_dsc = lv_draw_task_get_fill_dsc(draw_task);
            if (fill_draw_dsc) {
                fill_draw_dsc->color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), fill_draw_dsc->color, LV_OPA_10);
                fill_draw_dsc->opa = LV_OPA_COVER;
            }
        }
        // Only modify cell items
        if (base_dsc->part == LV_PART_ITEMS) {
            // If this is column 1, disable the border
            lv_draw_border_dsc_t* border = lv_draw_task_get_border_dsc(draw_task);
            if(border!=NULL)
            if (col % 2 == 1) {  // since column index alternates in id
                border->side = LV_BORDER_SIDE_NONE;
                border->width = 0;
            }
        }
    }

}

macroKey_t* macroScreenInit()
{
    macroKey = malloc(sizeof(macroKey_t) * 20);
     //Allocate memory for macroKey
    for (int i = 0; i < 20; i++) //Initialize macroKeyArray
    {
        macroKey[i].keyColor = lv_color_make((uint8_t)0, (uint8_t)0, (uint8_t)255);
        macroKey[i].macro.delayArrayMs = malloc(sizeof(uint32_t) * 5);
        for (int j = 0; j < 5; j++)
        {
            macroKey[i].macro.delayArrayMs[j] = 0;
        }
        macroKey[i].macro.macroArray = malloc(sizeof(char*) * 5);
        for (int j = 0; j < 5; j++)
        {
            macroKey[i].macro.macroArray[j] = malloc(sizeof(char) * 20);
            macroKey[i].macro.macroArray[j] = "";
        }
        macroKey[i].macro.mode = 0;
        macroKey[i].macro.modeData = 0;
    }


    lv_obj_t* macroScreen = lv_obj_create(NULL);
    lv_obj_set_size(macroScreen, 480, 320);
    lv_screen_load(macroScreen);
    lv_obj_t* grid = lv_obj_create(macroScreen);
    lv_obj_center(grid);
    lv_obj_set_size(grid, 480, 320);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(grid, macroScreenGrid_c, macroScreenGrid_r);

    lv_obj_t* backButton = lv_button_create(grid);
    lv_obj_t* backButtonLabel = lv_label_create(backButton);
    //lv_label_set_recolor(backButtonLabel, true);
    lv_label_set_text(backButtonLabel, "  #ff0000 " LV_SYMBOL_LEFT "#");
    //lv_obj_center(backButtonLabel);
    lv_obj_add_event_cb(backButton, toMainScreen, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_grid_cell(backButton, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t* colorLabel = lv_label_create(grid);
    lv_label_set_text(colorLabel, "Color:");
    lv_obj_set_grid_cell(colorLabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* colorSelectButton = lv_button_create(grid);
    static lv_style_t colorSelectStyle;
    lv_style_init(&colorSelectStyle);
    lv_style_set_radius(&colorSelectStyle, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&colorSelectStyle, lv_color_make(255, 255, 255));
    lv_obj_add_style(colorSelectButton, &colorSelectStyle, LV_PART_MAIN);
    lv_obj_set_grid_cell(colorSelectButton, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_size(colorSelectButton, 30, 30);

    lv_obj_t* recordButton = lv_button_create(grid);
    lv_obj_t* recordButtonLabel = lv_label_create(recordButton);
    lv_label_set_text(recordButtonLabel, " Record");
    lv_obj_align(recordButtonLabel, LV_ALIGN_CENTER, -10, 0);

    lv_obj_t* recordCirc = lv_obj_create(recordButton);
    lv_obj_set_size(recordCirc, 15, 15);
    lv_obj_set_scrollbar_mode(recordCirc, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(recordCirc, lv_color_make(0xFF, 0x00, 0x00), 0);
    lv_obj_set_style_radius(recordCirc, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(recordCirc, LV_ALIGN_CENTER, 29, 0);


    lv_obj_t* macroTable = lv_table_create(grid);
    lv_table_set_column_count(macroTable, 2);
    lv_table_set_row_count(macroTable, 11);
    lv_obj_set_grid_cell(macroTable, LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_STRETCH, 1, 6);
    lv_table_set_column_width(macroTable, 1, 100);
    lv_table_set_column_width(macroTable, 0, 220);
    lv_obj_set_size(macroTable, 180, LV_SIZE_CONTENT);

    // Create a style for the right border
    static lv_style_t style_right_border;
    lv_style_init(&style_right_border);
    lv_style_set_border_side(&style_right_border, LV_BORDER_SIDE_RIGHT);
    lv_style_set_border_width(&style_right_border, 4);
    lv_style_set_border_color(&style_right_border, lv_color_lighten(lv_color_black(), 30));

    // Optional: some padding for better spacing
    lv_style_set_pad_left(&style_right_border, 4);
    lv_style_set_pad_right(&style_right_border, 4);
    lv_style_set_pad_top(&style_right_border, 2);
    lv_style_set_pad_bottom(&style_right_border, 2);

    // Apply style to cells in column 0
    for (int r = 0; r < 6; r++) {
        lv_obj_add_style(macroTable, &style_right_border, LV_PART_ITEMS);
    }
    lv_obj_add_flag(macroTable, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
    lv_obj_add_event_cb(macroTable, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, NULL);

    /*
    dsc.color = lv_palette_main(LV_PALETTE_RED);
    dsc.center.x = 10;
    dsc.center.y = 10;
    dsc.width = 10;
    dsc.radius = 10;
    dsc.start_angle = 0;
    dsc.end_angle = 360;
    lv_draw_arc(&layer, &dsc);*/

    lv_obj_set_grid_cell(recordButton, LV_GRID_ALIGN_STRETCH, 4, 2, LV_GRID_ALIGN_STRETCH, 0, 1);
    registerMacroScreen(macroScreen);
    return macroKey;
}