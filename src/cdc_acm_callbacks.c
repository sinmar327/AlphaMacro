#include "main.h"
#include <string.h>
#include <stdio.h>

const static char *TAG = "CDC_ACM_COM_CALLBACK";

const static uint8_t identify_command[] = {73,68,69,78,84,73,70,89,10,0,0,0,0,0,0,0}; //Reads: IDENTIFY\n

static uint8_t rx_buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
static uint8_t tx_buf[CONFIG_TINYUSB_CDC_TX_BUFSIZE +1];


void cdc_acm_rx_cb(int itf, cdcacm_event_t *event) {
    
    esp_err_t err = ESP_OK;
    size_t rx_size = 0;
    err = tinyusb_cdcacm_read(itf, rx_buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size); // Lese empfangene Daten
    rx_buf[rx_size] = '\0';

    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to retrieve Serial Data being send, Error: %d", err);
    }

    for(int i = 0; i < rx_size; i++)
    {
        printf("%c", rx_buf[i]); // Gib die empfangenen Daten in der Konsole aus
    }

    if(rx_size <= 16)
    {
        if(memcmp(identify_command, rx_buf, rx_size) == 0)
        {
            ESP_LOGI(TAG, "Identify command received");
            memcpy(tx_buf, DeviceName, strlen((const char*)DeviceName));
            err = tinyusb_cdcacm_write_queue(itf, tx_buf, strlen((const char*)DeviceName));
            err = tinyusb_cdcacm_write_flush(itf, -1);
            if(err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to answer \"IDENTIFY\" call, Error: %d", err);
            }
        }
    }
}

