#include <stdio.h>
#include "lvgl_integration.h"
#include "esp_task_wdt.h"
#include "rgbKeyboardLink.h"
#include "driver/i2c_master.h"

uint32_t colorArray[] = {0xFF0000, 0xFF00, 0xFF, 0xFF0000, 0xFF00, 0xFF, 0xFF0000, 0xFF00, 0xFF}; 

i2c_master_bus_config_t i2c_conf = {
    .scl_io_num = 0,
    .sda_io_num = 1,
    .glitch_ignore_cnt = 7,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .i2c_port = I2C_NUM_0,
    .flags = {
        .enable_internal_pullup = true
    },
};
static i2c_master_bus_handle_t bus_hdl;


void app_main(void)
{
    i2c_new_master_bus(&i2c_conf, &bus_hdl);

    rgb_matrix_map_t* rgb_map = heap_caps_malloc(sizeof(rgb_matrix_map_t) * 20, MALLOC_CAP_DEFAULT);;
    ESP_ERROR_CHECK(rgb_matrix_auto_register_conf(20, 0x03, 4, rgb_map));
    //task_i2cscanner(NULL);
    app_lvgl_init(&bus_hdl);
    rgb_matrix_ic_init(&bus_hdl);
	//rgb_matrix_set_color_single(rgb_map, 4, 0x0707FF);
    //rgb_matrix_rainbow(rgb_map);
    //rgb_matrix_set_color_seq(rgb_map, colorArray, 0, 9);
}