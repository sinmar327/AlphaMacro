#include "sdkconfig.h"
#include "tinyusb.h"
#include "tusb.h"
#include "tusb_cdc_acm.h"
#include "FreeRTOS.h"
#include "esp_log.h"
#include "class/hid/hid_device.h"

static const uint8_t* DeviceName = (const uint8_t*)"AlphaMacro";

/*#include <stdint.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "class/hid/hid_device.h"
#include "class/hid/hid.h"
#include "sdkconfig.h"*/

extern void cdc_acm_rx_cb(int itf, cdcacm_event_t *event); // Deklariere die Callback-Funktion