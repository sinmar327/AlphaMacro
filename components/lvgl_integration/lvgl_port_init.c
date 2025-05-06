#include "lvgl_integration.h"
#include "esp_lvgl_port_touch.h"
#include "esp_lcd_touch_ft5x06.h"
#include "driver/gpio.h"
#include "driver/gpio_filter.h"
#include "freertos/semphr.h"

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


//#include "components/lcd_touch/esp_lcd_touch_ft5x06"

typedef struct {
    esp_lcd_touch_handle_t  handle;     /* LCD touch IO handle */
    lv_indev_t              *indev;     /* LVGL input device driver */
} lvgl_port_touch_ctx_t;

/* LCD size */


/* LCD settings */
#define EXAMPLE_LCD_SPI_NUM         (SPI2_HOST)
#define EXAMPLE_LCD_PIXEL_CLK_HZ    (6 * 1000 * 1000)
#define EXAMPLE_LCD_CMD_BITS        (8)
#define EXAMPLE_LCD_PARAM_BITS      (8)
#define EXAMPLE_LCD_COLOR_SPACE     (ESP_LCD_COLOR_SPACE_RGB)
#define EXAMPLE_LCD_BITS_PER_PIXEL  (16)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE (1)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT (50)
#define EXAMPLE_LCD_BL_ON_LEVEL     (1)

/* LCD pins */
#define EXAMPLE_LCD_GPIO_SCLK       (GPIO_NUM_12)
#define EXAMPLE_LCD_GPIO_MOSI       (GPIO_NUM_11)
#define EXAMPLE_LCD_GPIO_MISO       (GPIO_NUM_13)
#define EXAMPLE_LCD_GPIO_RST        (GPIO_NUM_47)
#define EXAMPLE_LCD_GPIO_DC         (GPIO_NUM_9)
#define EXAMPLE_LCD_GPIO_CS         (GPIO_NUM_10)

/* Touch settings */
#define EXAMPLE_TOUCH_I2C_NUM       (0)
#define EXAMPLE_TOUCH_I2C_CLK_HZ    (400000)

/* LCD touch pins */
#define EXAMPLE_TOUCH_I2C_SCL       (GPIO_NUM_0)
#define EXAMPLE_TOUCH_I2C_SDA       (GPIO_NUM_1)
#define EXAMPLE_TOUCH_GPIO_INT      (GPIO_NUM_4)
#define EXAMPLE_TOUCH_RST           (GPIO_NUM_5)

//#define SPI_TRANS_MODE_DIO 1

static char *TAG = "LVGL Main Integration";

/* LCD IO and panel */
static esp_lcd_panel_io_handle_t lcd_io = NULL;
static esp_lcd_panel_handle_t lcd_panel = NULL;
static esp_lcd_panel_io_handle_t tp_io_handle = NULL;
static esp_lcd_touch_handle_t touch_handle = NULL;

/* LVGL display and touch */
static lv_display_t *lvgl_disp = NULL;
static lv_indev_t *lvgl_touch_indev = NULL;

static SemaphoreHandle_t i2c_mutex = NULL;

static esp_err_t app_lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = EXAMPLE_LCD_GPIO_SCLK,
        .mosi_io_num = EXAMPLE_LCD_GPIO_MOSI,
        .miso_io_num = EXAMPLE_LCD_GPIO_MISO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT * sizeof(uint16_t),
        .flags = SPICOMMON_BUSFLAG_IOMUX_PINS,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(EXAMPLE_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = EXAMPLE_LCD_GPIO_DC,
        .cs_gpio_num = EXAMPLE_LCD_GPIO_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_LCD_SPI_NUM, &io_config, &lcd_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = EXAMPLE_LCD_GPIO_RST,
        .color_space = EXAMPLE_LCD_COLOR_SPACE,
        .bits_per_pixel = EXAMPLE_LCD_BITS_PER_PIXEL,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7796(lcd_io, &panel_config, &lcd_panel), err, TAG, "New panel failed");
    
    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);
    esp_lcd_panel_mirror(lcd_panel, true, true);
    esp_lcd_panel_disp_on_off(lcd_panel, true);

    return ret;

err:
    if (lcd_panel) {
        esp_lcd_panel_del(lcd_panel);
    }
    if (lcd_io) {
        esp_lcd_panel_io_del(lcd_io);
    }
    spi_bus_free(EXAMPLE_LCD_SPI_NUM);
    return ret;
}

static esp_err_t app_touch_init(i2c_master_bus_handle_t* bus_handle)


{
    //Initilize I2C
    

    //Initialize touch HW
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_NC, // Shared with LCD reset
        .int_gpio_num = EXAMPLE_TOUCH_GPIO_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 1,
            .mirror_y = 1,
        },
        
    };
    const esp_lcd_panel_io_i2c_config_t tp_io_config = /*ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();*/
    {                                       
            .dev_addr = ESP_LCD_TOUCH_IO_I2C_FT5x06_ADDRESS,
            .control_phase_bytes = 1,
            .dc_bit_offset = 0,
            .lcd_cmd_bits = 8,
            .flags =
            {
                .disable_control_phase = 1,
            },
            .scl_speed_hz = 100 * 1000,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_i2c(*bus_handle, &tp_io_config, &tp_io_handle), TAG, "");
    
    return esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &touch_handle);
}

static void IRAM_ATTR lvgl_port_touch_interrupt_callback(esp_lcd_touch_handle_t tp)
{
    lvgl_port_touch_ctx_t *touch_ctx = (lvgl_port_touch_ctx_t *) tp->config.user_data;

    /* Wake LVGL task, if needed */
    lvgl_port_task_wake(LVGL_PORT_EVENT_TOUCH, touch_ctx->indev);
}

static void lvgl_port_touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    assert(indev_drv);
    lvgl_port_touch_ctx_t *touch_ctx = (lvgl_port_touch_ctx_t *)lv_indev_get_driver_data(indev_drv);
    assert(touch_ctx);
    assert(touch_ctx->handle);

    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read data from touch controller into memory */
    esp_lcd_touch_read_data(touch_ctx->handle);

    /* Read data from touch controller */
    bool touchpad_pressed = esp_lcd_touch_get_coordinates(touch_ctx->handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        data->point.x = touchpad_x[0];
        data->point.y = touchpad_y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static void lvgl_touchpad_read_mutex(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    
    i2c_loop:
    if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        ESP_LOGV(TAG, "Reading touch pad");
        lvgl_port_touchpad_read(indev_drv, data);  // Call the original function
        xSemaphoreGive(i2c_mutex);
    } else {
        ESP_LOGW("LVGL_TOUCH", "Could not take I2C mutex, looping");
        goto i2c_loop;
    }
}

lv_indev_t *lvgl_custom_add_touch(const lvgl_port_touch_cfg_t *touch_cfg)
{
    esp_err_t ret = ESP_OK;
    lv_indev_t *indev = NULL;
    assert(touch_cfg != NULL);
    assert(touch_cfg->disp != NULL);
    assert(touch_cfg->handle != NULL);

    /* Touch context */
    lvgl_port_touch_ctx_t *touch_ctx = malloc(sizeof(lvgl_port_touch_ctx_t));
    if (touch_ctx == NULL) {
        ESP_LOGE(TAG, "Not enough memory for touch context allocation!");
        return NULL;
    }
    touch_ctx->handle = touch_cfg->handle;

    if (touch_ctx->handle->config.int_gpio_num != GPIO_NUM_NC) {
        /* Register touch interrupt callback */
        ret = esp_lcd_touch_register_interrupt_callback_with_data(touch_ctx->handle, lvgl_port_touch_interrupt_callback, touch_ctx);
        ESP_GOTO_ON_ERROR(ret, err, TAG, "Error in register touch interrupt.");
    }

    lvgl_port_lock(0);
    /* Register a touchpad input device */
    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    /* Event mode can be set only, when touch interrupt enabled */
    if (touch_ctx->handle->config.int_gpio_num != GPIO_NUM_NC) {
        lv_indev_set_mode(indev, LV_INDEV_MODE_EVENT);
    }
    lv_indev_set_read_cb(indev, lvgl_touchpad_read_mutex);
    lv_indev_set_disp(indev, touch_cfg->disp);
    lv_indev_set_driver_data(indev, touch_ctx);
    touch_ctx->indev = indev;
    lvgl_port_unlock();

err:
    if (ret != ESP_OK) {
        if (touch_ctx) {
            free(touch_ctx);
        }
    }

    return indev;
}



esp_err_t app_lvgl_init(i2c_master_bus_handle_t* bus_handle, SemaphoreHandle_t i2c)
{
    i2c_mutex = i2c;
    app_lcd_init();
    ESP_ERROR_CHECK(app_touch_init(bus_handle));
    gpio_config_t touchGPIOIntConf = {
        .pull_up_en = true,
        .pull_down_en = false,
        .pin_bit_mask = 1ULL << GPIO_NUM_4,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_LOW_LEVEL, //IMPORTANT: Set intr_type after gpio setup is done so it really triggers on the interrupt
    };
    gpio_config(&touchGPIOIntConf);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 7,         /* LVGL task priority */
        .task_stack = 24576,         /* LVGL task stack size */
        .task_affinity = -1,        /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500,   /* Maximum sleep in LVGL task */
        .timer_period_ms = 5        /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = 4095,//SPI DMA max buffer size
        .trans_size = 4095,
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        //.color_format = LV_COLOR_FORMAT_RGB565,
        /* Rotation values must be same as used in esp_lcd for initial settings of the screen */
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = true,
        },
        .flags = {
            .buff_dma = false,
            .buff_spiram = true, 
#if LVGL_VERSION_MAJOR >= 9
            .swap_bytes = true,
#endif
        },

    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
    
    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = touch_handle,
    };
    lvgl_touch_indev = lvgl_custom_add_touch(&touch_cfg);

    /*
    touch_sem = xSemaphoreCreateBinary();
    ESP_ERROR_CHECK(gpio_isr_handler_add(
        EXAMPLE_TOUCH_GPIO_INT,
        touch_gpio_isr,     // Your ISR function below
        NULL                // Optional argument
    ));
    xTaskCreate(touch_task, "touch_task", 2048, NULL, 10, NULL);*/
    lvglStartup(lvgl_disp);

    return ESP_OK;
}
