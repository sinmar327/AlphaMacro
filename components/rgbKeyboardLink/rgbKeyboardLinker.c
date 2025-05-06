#include "rgbKeyboardLinker.h"
#include "rgbMatrixDriver.h"

static const char *TAG = "RGB_KEYBOARD_LINKER";

static rgb_matrix_map_t* matrixMap;

esp_err_t check_init()
{
    if(!matrixMap)
        return ESP_ERR_INVALID_STATE;
    return ESP_OK;
}
esp_err_t rgb_keyboard_linker_init(const i2c_master_bus_handle_t *bus_hdl, const SemaphoreHandle_t mutex, rgb_matrix_map_t* map)
{
    if(!rgb_matrix_check_init())
        rgb_matrix_ic_init(bus_hdl, mutex);
    else return ESP_ERR_INVALID_STATE;
    matrixMap = map;
    return ESP_OK;
}

esp_err_t rgb_keyboard_linker_set_key_color(lv_color_t color, uint8_t idx)
{
    ESP_RETURN_ON_ERROR(check_init(), TAG, "The linker was not initialized", NULL);

    rgb_matrix_set_color_single(matrixMap, idx, lv_color_to_int(color));
    ESP_LOGV(TAG, "Set the matrix pos %i to 0x%06X", idx, lv_color_to_int(color));
    return ESP_OK;
}
