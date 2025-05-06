#pragma once
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lvgl_port.h"
#include "esp_lcd_st7796.h"
#include "esp_lvgl_port_touch.h"
#include "lvgl.h"
#include "demos/lv_demos.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
//#include "hal/wdt_hal.h"


#define EXAMPLE_LCD_H_RES   (320)
#define EXAMPLE_LCD_V_RES   (480)

lv_obj_t* lvglStartup(lv_disp_t* disp);
esp_err_t app_lvgl_init(i2c_master_bus_handle_t* bus_handle);
