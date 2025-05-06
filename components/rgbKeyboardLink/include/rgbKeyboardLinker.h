#pragma once
#include "lvgl.h"
#include "esp_check.h"
#include "driver/i2c_types.h"
#include "freertos/semphr.h"
#include "rgbKeyboardTypes.h"



esp_err_t rgb_keyboard_linker_init(const i2c_master_bus_handle_t *bus_hdl, const SemaphoreHandle_t mutex, rgb_matrix_map_t* map);

esp_err_t rgb_keyboard_linker_set_key_color(lv_color_t color, uint8_t idx);