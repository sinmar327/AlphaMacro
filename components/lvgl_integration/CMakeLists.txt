
idf_component_register(SRCS "lvgl_integration.c" "lvgl_port_init.c" "lvgl_main_screen.c" "lvgl_screen_linker.c" "lvgl_macro_screen.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "lvgl"
                       PRIV_REQUIRES "driver" "esp_lcd" "esp_lcd_st7796" "esp_lvgl_port" "esp_timer" "esp_lcd_touch_ft5x06" "rgbKeyboardLink")
