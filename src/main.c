#include "main.h"
#include "esp_check.h"
#include <device/usbd.h>
#include "tusb_console.h" // Include the header file for esp_tusb_deinit_console
#include "unity.h"
#include "tusb_cdc_acm.h"
#include "esp_log.h"

//////////
//HID_ASCII_TO_KEYCODE
/////////

const tinyusb_config_t tusb_cfg = {
    .device_descriptor = NULL,
    .string_descriptor = NULL,
    .external_phy = false,
};

const tinyusb_config_cdcacm_t cdcacm_cfg = {
    .usb_dev = TINYUSB_USBDEV_0,
    .cdc_port = TINYUSB_CDC_ACM_0,
    .rx_unread_buf_sz = 64,
    .callback_rx = &cdc_acm_rx_cb, // Verknüpfe die Callback-Funktion
    .callback_line_coding_changed = NULL,
    .callback_line_state_changed = NULL,
    .callback_rx_wanted_char = NULL
};

void app_main() {
    esp_log_set_vprintf(vprintf);
    printf("HI");
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&cdcacm_cfg));
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(TINYUSB_CDC_ACM_0, CDC_EVENT_RX, &cdc_acm_rx_cb));
    ESP_LOGI("CDC Info","TinyUSB CDC ACM initialized.\n");
    uint8_t* buffer = malloc(8*64);
    for(int i = 0; i<64; i++)
    {
        buffer[i] = i;
    }
    tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0,buffer,64);
    tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, -1);
}
/*static const char *TAG = "example";
static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];

void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    // initialization
    size_t rx_size = 0;

    // read //
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Data from channel %d:", itf);
        ESP_LOG_BUFFER_HEXDUMP(TAG, buf, rx_size, ESP_LOG_INFO);
    } else {
        ESP_LOGE(TAG, "Read error");
    }

    // write back
    tinyusb_cdcacm_write_queue(itf, buf, rx_size);
    tinyusb_cdcacm_write_flush(itf, 0);
}

void tinyusb_cdc_line_state_changed_callback(int itf, cdcacm_event_t *event)
{
    int dtr = event->line_state_changed_data.dtr;
    int rts = event->line_state_changed_data.rts;
    ESP_LOGI(TAG, "Line state changed on channel %d: DTR:%d, RTS:%d", itf, dtr, rts);
}

void app_main(void)
{
    
    ESP_LOGI(TAG, "USB initialization");
    const tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };

    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    
    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback, // the first way to register a callback
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL
    };
    //ESP_LOGI("CDC Status", "%d", tusb_cdc_acm_deinit(TINYUSB_CDC_ACM_0));
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    // the second way to register a callback 
    ESP_ERROR_CHECK(tinyusb_cdcacm_register_callback(
                        TINYUSB_CDC_ACM_0,
                        CDC_EVENT_LINE_STATE_CHANGED,
                        &tinyusb_cdc_line_state_changed_callback));


    ESP_LOGI(TAG, "USB initialization DONE");
}*/

