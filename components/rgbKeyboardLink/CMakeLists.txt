
idf_component_register(SRCS "rgbKeyboardLinker.c" "rgbMatrixDriver.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "driver"
                       PRIV_REQUIRES "driver" "lvgl")
