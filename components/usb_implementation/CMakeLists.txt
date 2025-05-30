
idf_component_register(SRCS "network_macro_server.c" "hid_driver.c" "usb_implementation.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "tusb"
                       PRIV_REQUIRES "tusb" "freertos")
