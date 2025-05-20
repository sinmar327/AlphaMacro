#include "lvgl_macro_screen.h"
#include "lvgl_screen_linker.h"
#include "lvgl.h"
#include "esp_log.h"
#include "rgbKeyboardLinker.h"

void setMacroScreen(uint8_t idx);

static const int32_t macroScreenGrid_c[] = { LV_GRID_FR(2), LV_GRID_FR(4), LV_GRID_FR(2), LV_GRID_FR(4), LV_GRID_FR(3), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const int32_t macroScreenGrid_r[] = { 30, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), 30,  LV_GRID_TEMPLATE_LAST};

static const int32_t delayRollerGrid_c[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const int32_t delayRollerGrid_r[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

static const int32_t colorPickerGrid_c[] = {LV_GRID_FR(1), LV_GRID_FR(10), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
static const int32_t colorPickerGrid_r[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

static const char *TAG = "MACRO_SCREEN";

static macroKey_t* macroKey;
static const char* modesString = "Click\nRepeat Toggle\nRepeat times\nHold";
static const char* modeArray[] = {"Click", "Repeat Toggle", "Repeat times", "Hold"};
static char* modeSelectDropdownText;

static uint8_t idxCurrent;

/////////////////////////////////////////////
/////////    Component variables    /////////    (TODO: sort)
/////////////////////////////////////////////
static lv_obj_t* macroScreen;
static lv_obj_t* grid;
static lv_obj_t* backButton;
static lv_obj_t* backButtonLabel;
static lv_obj_t* colorLabel;
static lv_obj_t* colorSelectButton;
static lv_style_t colorSelectStyle;
static lv_obj_t* recordButton;
static lv_obj_t* recordButtonLabel;
static lv_obj_t* recordCirc;
static lv_obj_t* macroTable;
static lv_style_t style_right_border;
static lv_obj_t* editButton;
static lv_obj_t* editButtonLabel;
static lv_obj_t* addButton;
static lv_obj_t* addButtonLabel;
static lv_obj_t* delayLable;
static lv_obj_t* delayRollerGrid;
static lv_obj_t** delayRollerArr;
static const char* delayRollerSelectionTxt;
static lv_obj_t* saveButton;
static lv_obj_t* saveButtonLabel;
static lv_obj_t* deleteButton;
static lv_obj_t* deleteButtonLabel;
static lv_obj_t* modeSelectDropdown;
static lv_obj_t* colorPickerGrid;
static lv_obj_t** colorPickerObj;
static uint32_t colorNow = 0xFFFFFF;
static lv_color_t colorNowT;

//Callback for table formating for better readability
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

//Callback for toggling the color picker
static void colorPickerToggleOpa(lv_event_t* e)
{
    if(lv_obj_has_flag(colorPickerGrid, LV_OBJ_FLAG_HIDDEN) == true)
    {
        lv_obj_remove_flag(colorPickerGrid, LV_OBJ_FLAG_HIDDEN);
    }
    else lv_obj_add_flag(colorPickerGrid, LV_OBJ_FLAG_HIDDEN);
}

//Function for setting the color of the color picker and handling the storage of variables 
static void colorSet(lv_color_t color)
{
    
    lv_obj_set_style_bg_color(colorSelectButton, color, LV_PART_MAIN);
    char colorText[17];
    sprintf(colorText, "Color: 0x%06X", (unsigned int)lv_color_to_int(color) & 0xFFFFFF);
    lv_label_set_text(colorPickerObj[3], colorText);
    lv_obj_invalidate(colorPickerObj[3]);
}

//Callback for color picker color change 
static void colorPickerColorChanged(lv_event_t* e)
{
    lv_obj_t* picker = lv_event_get_target_obj(e);
    uint32_t sliderColor = lv_color_to_int(lv_obj_get_style_bg_color(picker, LV_PART_MAIN));
    uint8_t value = lv_slider_get_value(picker);
    colorNow &= ~sliderColor;
    colorNow |= value << (__builtin_ctz(sliderColor));
    lv_color_t color = lv_color_hex(colorNow);
    colorNowT = color;
    colorSet(color);
}

//Button to save all settings set on the screen (TODO: implement nvs here)
static void saveButtonCb(lv_event_t* e)
{
    ESP_LOGV(TAG, "Save button pressed");
    ESP_RETURN_VOID_ON_ERROR(rgb_keyboard_linker_set_key_color(colorNowT, idxCurrent), TAG, "Failed to set the rgb value to the matrix");
    macroKey[idxCurrent].keyColor = colorNowT;
}

//Make this function a timer so the dropdown updates instantly
static void modeSelectDropdownTextCb(lv_event_t* e)
{
    if(modeSelectDropdownText == NULL)
    {
        modeSelectDropdownText = heap_caps_calloc(70, sizeof(char), MALLOC_CAP_DEFAULT);
    }
    lv_obj_t* dropdown = lv_event_get_target_obj(e);
    sprintf(modeSelectDropdownText, "Mode: %s", modeArray[lv_dropdown_get_selected(dropdown)]);
    lv_dropdown_set_text(dropdown, modeSelectDropdownText);
    lv_obj_invalidate(dropdown);
}

//Function for deinitializing the macro screen
static void deinitMacroScreen()
{
    lv_obj_add_flag(colorPickerGrid, LV_OBJ_FLAG_HIDDEN);
    toMainScreen();
}

//Init an array of macros used for macro storage (TODO: implement nvs here)
macroKey_t* initMacroKeyArray(const uint8_t size, const uint8_t macroCount)
{
    macroKey_t* arr = heap_caps_calloc(size, sizeof(macroKey_t), MALLOC_CAP_SPIRAM);
     //Allocate memory for macroKey
    for (int i = 0; i < size; i++) //Initialize macroKeyArray
    {
        arr[i].keyColor = lv_color_make((uint8_t)0, (uint8_t)0, (uint8_t)0);
        arr[i].macro.delayArrayMs = heap_caps_calloc(macroCount, sizeof(uint32_t), MALLOC_CAP_SPIRAM);
        ESP_ERROR_CHECK(!arr[i].macro.delayArrayMs);
        arr[i].macro.macroArray = heap_caps_calloc(macroCount, sizeof(char*), MALLOC_CAP_SPIRAM);
        ESP_ERROR_CHECK(!arr[i].macro.macroArray);
        for (int j = 0; j < macroCount; j++)
        {
            arr[i].macro.macroArray[j] = heap_caps_calloc(20, sizeof(char), MALLOC_CAP_SPIRAM);
        }
        arr[i].macro.mode = 0;
        arr[i].macro.modeData = 0;
    }
    return arr;
}

//Init the macro screen when first opening it
void macroScreenInit(uint8_t idx, macroKey_t* macros)
{
    lv_lock();
    idxCurrent = idx;
    macroKey = macros;
                    //Init Grid
    macroScreen = lv_obj_create(NULL);
    lv_obj_set_size(macroScreen, 480, 320);
    lv_screen_load(macroScreen);
    grid = lv_obj_create(macroScreen);
    lv_obj_center(grid);
    lv_obj_set_size(grid, 480, 320);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(grid, macroScreenGrid_c, macroScreenGrid_r);
                    //Init back button
    backButton = lv_button_create(grid);
    backButtonLabel = lv_label_create(backButton);
    //lv_label_set_recolor(backButtonLabel, true);
    lv_label_set_text(backButtonLabel, "  #ff0000 " LV_SYMBOL_LEFT "#");
    //lv_obj_center(backButtonLabel);
    lv_obj_add_event_cb(backButton, deinitMacroScreen, LV_EVENT_PRESSED, NULL);
    lv_obj_set_grid_cell(backButton, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
                    //Color select label text
    colorLabel = lv_label_create(grid);
    lv_label_set_text(colorLabel, "Color:");
    lv_obj_set_grid_cell(colorLabel, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
                    //Init Color select button (TODO: add key color and main screen color)
    colorSelectButton = lv_button_create(grid);
    
    lv_style_init(&colorSelectStyle);
    lv_style_set_radius(&colorSelectStyle, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&colorSelectStyle, lv_color_make(0, 0, 0));
    lv_obj_add_style(colorSelectButton, &colorSelectStyle, LV_PART_MAIN);
    lv_obj_set_grid_cell(colorSelectButton, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_size(colorSelectButton, 30, 30);
                    //Init record button (add change symbol to square on record)
    recordButton = lv_button_create(grid);
    recordButtonLabel = lv_label_create(recordButton);
    lv_label_set_text(recordButtonLabel, " Record");
    lv_obj_align(recordButtonLabel, LV_ALIGN_CENTER, -10, 0);
    lv_obj_set_grid_cell(recordButton, LV_GRID_ALIGN_STRETCH, 4, 2, LV_GRID_ALIGN_STRETCH, 0, 1);
                    //Circle for record button
    recordCirc = lv_obj_create(recordButton);
    lv_obj_set_size(recordCirc, 15, 15);
    lv_obj_set_scrollbar_mode(recordCirc, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(recordCirc, lv_color_make(0xFF, 0x00, 0x00), 0);
    lv_obj_set_style_radius(recordCirc, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(recordCirc, LV_ALIGN_CENTER, 29, 0);

                    //Init Macro table (TODO: Select column)
    macroTable = lv_table_create(grid);
    lv_table_set_column_count(macroTable, 2);
    lv_table_set_row_count(macroTable, 11);
    lv_obj_set_grid_cell(macroTable, LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_STRETCH, 1, 6);
    lv_table_set_column_width(macroTable, 1, 90);
    lv_table_set_column_width(macroTable, 0, 220);
    lv_obj_set_size(macroTable, 180, LV_SIZE_CONTENT);

    // Create a style for the right border
    
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
    
                    //Init edit button (TODO: add functionality)
    editButton = lv_button_create(grid);
    editButtonLabel = lv_label_create(editButton);
    lv_label_set_text(editButtonLabel, "Edit " LV_SYMBOL_EDIT);
    lv_obj_center(editButtonLabel);
    lv_obj_set_grid_cell(editButton, LV_GRID_ALIGN_STRETCH, 4, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
                    //Init add button (TODO: add functionality)
    addButton = lv_button_create(grid);
    addButtonLabel = lv_label_create(addButton);
    //Add color of + sign when updating to LVGL 9.3
    lv_label_set_text(addButtonLabel, "Add +");
    lv_obj_center(addButtonLabel);
    lv_obj_set_grid_cell(addButton, LV_GRID_ALIGN_STRETCH, 4, 2, LV_GRID_ALIGN_STRETCH, 2, 1);
                    //Init label for delay setting (TODO: add functionality)
    delayLable = lv_label_create(grid);
    lv_label_set_text(delayLable, "Delay [ms]:");
    
    lv_obj_set_grid_cell(delayLable, LV_GRID_ALIGN_STRETCH, 4, 2, LV_GRID_ALIGN_STRETCH, 3, 1);
                    //Init roller grid
    delayRollerGrid = lv_obj_create(grid);
    lv_obj_set_grid_dsc_array(delayRollerGrid, delayRollerGrid_c, delayRollerGrid_r);
    lv_obj_set_grid_cell(delayRollerGrid, LV_GRID_ALIGN_STRETCH, 4, 2, LV_GRID_ALIGN_STRETCH, 4, 3);
    lv_obj_set_style_pad_column(delayRollerGrid, 0, 0);
    lv_obj_set_style_pad_row(delayRollerGrid, 0, 0);
    lv_obj_set_style_pad_all(delayRollerGrid, 0, 0);
    
    delayRollerArr = heap_caps_malloc(sizeof(lv_obj_t*)*5, MALLOC_CAP_SPIRAM);
    delayRollerSelectionTxt = "0\n1\n2\n3\n4\n5\n6\n7\n8\n9";
                    //No bar style for roller to prevent bar over selected Value
    static lv_style_t style_no_bar;
    lv_style_init(&style_no_bar);

    // Fully transparent backgrounds
    lv_style_set_bg_opa(&style_no_bar, LV_OPA_TRANSP);

    // No borders, outlines, shadows, radius or padding
    lv_style_set_border_width(&style_no_bar, 0);
    lv_style_set_outline_width(&style_no_bar, 0);
    lv_style_set_shadow_width(&style_no_bar, 0);
    lv_style_set_radius(&style_no_bar, 0);
    lv_style_set_pad_all(&style_no_bar, 0);
    
    for(int i = 0; i < 5; i++)
    {
        delayRollerArr[i] = lv_roller_create(delayRollerGrid);
        lv_roller_set_options(delayRollerArr[i], delayRollerSelectionTxt, LV_ROLLER_MODE_INFINITE);
        lv_roller_set_visible_row_cnt(delayRollerArr[i], 2);
        lv_roller_set_selected(delayRollerArr[i], 0, LV_ANIM_OFF);
        lv_obj_set_grid_cell(delayRollerArr[i], LV_GRID_ALIGN_STRETCH, i, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        //Set main part style to circumvent gray bar over the roller values
        lv_obj_add_style(delayRollerArr[i], &style_no_bar, LV_PART_MAIN | LV_STATE_DEFAULT);    // Main body :contentReference[oaicite:6]{index=6}
    }
                    //Init save button (TODO: add functionality)
    saveButton = lv_button_create(grid);
    saveButtonLabel = lv_label_create(saveButton);
    lv_label_set_text(saveButtonLabel, "Save " LV_SYMBOL_SAVE);
    lv_obj_center(saveButtonLabel);
    lv_obj_add_event_cb(saveButton, saveButtonCb, LV_EVENT_PRESSED, NULL);
    lv_obj_set_grid_cell(saveButton, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 7, 1);
                    //Init delete button (TODO: add functionality)
    deleteButton = lv_button_create(grid);
    deleteButtonLabel = lv_label_create(deleteButton);
    lv_label_set_text(deleteButtonLabel, LV_SYMBOL_TRASH);
    lv_obj_center(deleteButtonLabel);
    lv_obj_set_grid_cell(deleteButton, LV_GRID_ALIGN_STRETCH, 5, 1, LV_GRID_ALIGN_STRETCH, 7, 1);
                    //Init mode selection dropdown (TODO: add functionality, change text when mode is changed)
    modeSelectDropdown = lv_dropdown_create(grid);
    lv_dropdown_set_text(modeSelectDropdown, "Mode: Click");

    lv_dropdown_set_options_static(modeSelectDropdown, modesString);
    lv_dropdown_set_dir(modeSelectDropdown, LV_DIR_TOP);
    lv_dropdown_set_symbol(modeSelectDropdown, LV_SYMBOL_UP);
    lv_obj_set_grid_cell(modeSelectDropdown, LV_GRID_ALIGN_STRETCH, 0, 3, LV_GRID_ALIGN_STRETCH, 7, 1);
    lv_obj_add_event_cb(modeSelectDropdown, modeSelectDropdownTextCb, LV_EVENT_VALUE_CHANGED, NULL);
                    //Init color picker grid
    colorPickerGrid = lv_obj_create(grid);
    lv_obj_set_grid_dsc_array(colorPickerGrid, colorPickerGrid_c, colorPickerGrid_r);
    lv_obj_set_grid_cell(colorPickerGrid, LV_GRID_ALIGN_STRETCH, 0, 4, LV_GRID_ALIGN_STRETCH, 1, 6);

    colorPickerObj = heap_caps_calloc(4, sizeof(lv_obj_t*), MALLOC_CAP_DEFAULT);
    colorPickerObj[3] = lv_label_create(colorPickerGrid);
    lv_label_set_text(colorPickerObj[3], "Color: 0x000000");
    lv_obj_set_grid_cell(colorPickerObj[3], LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);
    lv_color_t rgbColorArray[] = {lv_color_hex(0xFF0000), lv_color_hex(0x00FF00), lv_color_hex(0x0000FF)};
    for(int i = 0; i<3; i++)
    {
        colorPickerObj[i] = lv_slider_create(colorPickerGrid);
        lv_slider_set_range(colorPickerObj[i], 0, 255);
        lv_obj_set_grid_cell(colorPickerObj[i], LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, i, 1);
        lv_obj_set_style_bg_opa(colorPickerObj[i], LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_obj_set_style_bg_color(colorPickerObj[i], rgbColorArray[i], LV_PART_MAIN);
        lv_obj_set_style_bg_opa(colorPickerObj[i], LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_add_event_cb(colorPickerObj[i], colorPickerColorChanged, LV_EVENT_VALUE_CHANGED, NULL);
        lv_slider_set_value(colorPickerObj[i],255, LV_ANIM_OFF);
    }
    lv_obj_add_event_cb(colorSelectButton, colorPickerToggleOpa, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(colorPickerGrid, LV_OBJ_FLAG_HIDDEN);

    registerMacroScreen(macroScreen);
    lv_unlock();
}

//Sets the screen content before loading (TODO: implement nvs here)
void setMacroScreen(uint8_t idx)
{
    colorNow = lv_color_to_int(macroKey[idx].keyColor);
    colorNowT = macroKey[idx].keyColor;
    colorSet(macroKey[idx].keyColor);
    for(int i = 0; i < 3; i++)
    {
        uint8_t colorHex = 
            (lv_color_to_u32(macroKey[idx].keyColor)
            & lv_color_to_u32(lv_obj_get_style_bg_color(colorPickerObj[i], LV_PART_MAIN)))
            >> __builtin_ctz(lv_color_to_u32(lv_obj_get_style_bg_color(colorPickerObj[i], LV_PART_MAIN)));
        
        lv_slider_set_value(colorPickerObj[i], colorHex, LV_ANIM_OFF);
        lv_obj_invalidate(colorPickerObj[i]);
        //ESP_LOGI(TAG, "Color for Slider %i set to %x", i, colorHex & 0xFF);   
    }
    
}

//Function for the screen_linker to load the screen when initialisation was already done
void macroScreenLoad(uint8_t idx)
{
    idxCurrent = idx;
    lv_lock();
    lv_screen_load(macroScreen);

    setMacroScreen(idx);

    lv_unlock();
}