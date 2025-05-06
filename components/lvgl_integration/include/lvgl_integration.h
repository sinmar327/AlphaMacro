#pragma once
#include "lvgl.h"
#include "driver/i2c_types.h"
#include "lvgl_intergration_types.h"


#define EXAMPLE_LCD_H_RES   (320)
#define EXAMPLE_LCD_V_RES   (480)

lv_obj_t* lvglStartup(lv_disp_t* disp);
esp_err_t app_lvgl_init(i2c_master_bus_handle_t* bus_handle, SemaphoreHandle_t i2c);

