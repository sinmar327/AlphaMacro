#include <stdio.h>
#include "rgbKeyboardLink.h"
#include "driver/i2c_master.h"
#include <string.h>
#include <freertos/FreeRTOS.h>

static const char *TAG = "rgb_matrix";

static i2c_master_bus_handle_t* bus_hdl;
static i2c_master_dev_handle_t dev_hdl_conf;
static i2c_master_dev_handle_t dev_hdl_pwm;

esp_err_t rgb_matrix_ic_init(i2c_master_bus_handle_t* bus_handle)
{
    bus_hdl = bus_handle;
    i2c_device_config_t dev_conf_conf = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x40,
        .scl_speed_hz = 400 * 1000,
        .scl_wait_us = 5,
    };
    
    i2c_master_bus_add_device(*bus_hdl, &dev_conf_conf, &dev_hdl_conf);
    i2c_device_config_t dev_conf_pwm = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x42,
        .scl_speed_hz = 400 * 1000,
        .scl_wait_us = 5,
    };
    #define Dev_initial 0x01
    #define PWM_Scale_Mode 2

    
    i2c_master_bus_add_device(*bus_hdl, &dev_conf_pwm, &dev_hdl_pwm);
    uint8_t dev_conf_init[] = {Dev_initial, 0x0};
    SET_REGISTER(dev_conf_init, Max_Line_Num_reg, 4, 0x4);
    SET_REGISTER(dev_conf_init, Data_Ref_Mode_reg, 2, 0x0);
    SET_REGISTER(dev_conf_init, PWM_Fre_reg, 1, 0x0);

    uint8_t dev_conf1[] = {Dev_config1, 0x0};
    SET_REGISTER(dev_conf1, SW_BLK_reg, 1, 0x1);
    SET_REGISTER(dev_conf1, PWM_Scale_Mode_reg, 1, 0x1);
    SET_REGISTER(dev_conf1, PWM_Phase_Shift_reg, 1, 0x1);
    SET_REGISTER(dev_conf1, CS_ON_Shift_reg, 1, 0x1);

    uint8_t dev_conf2[] = {Dev_config2, 0x0};
    SET_REGISTER(dev_conf2 ,Comp_Group3_reg, 2, 0x0);
    SET_REGISTER(dev_conf2, Comp_Group2_reg, 2, 0x0);
    SET_REGISTER(dev_conf2, Comp_Group1_reg, 2, 0x0);
    SET_REGISTER(dev_conf2, LOD_removal_reg, 1, 0x0);
    SET_REGISTER(dev_conf2, LSD_removal_reg, 1, 0x0);

    uint8_t dev_conf3[] = {Dev_config3, 0x0};
    SET_REGISTER(dev_conf3, Down_Deghost_reg, 2, 0x3);
    SET_REGISTER(dev_conf3, Up_Deghost_reg, 2, 0x1);
    SET_REGISTER(dev_conf3, Maximum_Current_reg, 3, 0x7);


    uint8_t conf_array[][2] = {
        {Chip_en, 0x1}, //Enable chip
        {dev_conf_init[0], dev_conf_init[1]},
        {dev_conf1[0], dev_conf1[1]},
        {dev_conf2[0], dev_conf2[1]},
        {dev_conf3[0], dev_conf3[1]},
        {R_current_set, 0x46},
        {G_current_set, 0x7F},
        {B_current_set, 0x4C},
    };
    //esp_err_t err = ESP_OK;
    /*do {
        uint8_t buf[2] = {0xA9, 0xFF};
        err = i2c_master_transmit(dev_hdl_conf, buf, 2, 100);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    while(err != ESP_OK);*/
    
    for(int i = 0; i < 8; i++)
    {
        printf("%x %x\n", conf_array[i][0], conf_array[i][1]);
        ESP_RETURN_ON_ERROR(i2c_master_transmit(dev_hdl_conf, conf_array[i], 2, -1), TAG, "unexpected error in ic_init while transmitting the configuration data");
    }
    for(int i = 0; i <= 0x47; i++)
    {
        uint8_t buf[] = {i, 0xFF};
        ESP_RETURN_ON_ERROR(i2c_master_transmit(dev_hdl_pwm, buf, 2, -1), TAG, "unexpected error in ic_init while transmitting the initial pwm configuration");
    }
    /*uint8_t rx_buf[255];
    uint8_t tx_buf[1] = {0};
    i2c_master_transmit_receive(dev_hdl_conf, tx_buf, 1, rx_buf, 255, -1);
    for(int i = 0; i < 0xFF; i++)
    {
        printf("%x: %x\n", i, rx_buf[i]);
    }*/
    return ESP_OK;
}
esp_err_t rgb_matrix_set_color_single(rgb_matrix_map_t* map, uint8_t led, uint32_t color)
{
    rgb_matrix_map_t ledPos;
    for(int i = 0; i <= 0xFF; i++)
    {
        if(map[i].map == led)
        {
            //printf("%i\n", i);
            ledPos.map = map[i].map;
            ledPos.pos = map[i].pos;
            break;
        }
    }
    uint8_t buf[] = {ledPos.pos, ((color & 0xFF0000) >> 16), ((color & 0x00FF00) >> 8), (color & 0x0000FF)};
    ESP_RETURN_ON_ERROR(i2c_master_transmit(dev_hdl_pwm, buf, 4, -1), TAG, "unexpected error in set_color_single while transmitting the color data");
    map[ledPos.pos].value = color & 0xFFFFFF;
    //uint8_t rx_buf[3]; 
    //i2c_master_transmit_receive(dev_hdl_pwm, buf, 1, rx_buf, 3, -1);
    //printf("%x %x %x\n", rx_buf[0], rx_buf[1], rx_buf[2]);
    return ESP_OK;
}
esp_err_t rgb_matrix_auto_register_conf(uint8_t length, uint8_t startRegL, uint8_t lines, rgb_matrix_map_t* map)
{
    // Allocate memory for the map
    ESP_RETURN_ON_FALSE(map, ESP_ERR_NO_MEM, TAG, "no memory for auto_register_conf memory allocation");

    uint8_t columns = length / lines; // Calculate the number of columns
    for (uint8_t i = 0; i < length; i++) {
        uint8_t line = i / columns; // Determine the row (line)
        uint8_t column = i % columns; // Determine the column

        map[i].map = i; // Set the map index
        map[i].pos = (startRegL * (1 + line)) + (column * 3) + (line * columns * 3); // Calculate the register position
        map[i].value = 0x00;
        
    }
    
    return ESP_OK;
}
esp_err_t rgb_matrix_set_color_seq(rgb_matrix_map_t* map, uint32_t* color, uint8_t firstLed, uint8_t writeCount)
{
    uint32_t* colors = NULL;
    uint8_t firstLedIndex = -1;
    for(int i = 0; i <= 0xFF; i++) //Determine index of the given led
    {
        if(map[i].map == firstLed)
        {
            firstLedIndex = i;
            break;
        }
    }
    if(firstLedIndex == -1) //Return if the led with the given identifier was not found in the map 
    {
        return ESP_ERR_NOT_FOUND;
    }
    if(color[0] == map[firstLedIndex].value) //Check if the first color value for the given led has already been written to the matrix. This is done to save i2c bus transaction time
    {
        for(int i = 0; i < writeCount; i++)
        {
            if(color[i] != map[firstLedIndex].value) //Set the colors so that the first time there is a different value in the map (looking from the left) the colors are really set
            {
                firstLedIndex += i;
                writeCount -= i;
                for(int j = writeCount; j <= 0; j--) //Do the same as above from the other side. For that it recurses from the other side and when the same, sets writeCount to the value of j. The first time, there is a different value it leaves the loop
                {
                    if(color[j] == map[firstLedIndex + j].value)
                        writeCount = j;
                    else break;
                }
                colors = heap_caps_malloc(sizeof(uint32_t) * writeCount, MALLOC_CAP_DEFAULT); //Allocate memory for the necessary color array
                memcpy(colors, color + i, writeCount * (sizeof(uint32_t)/sizeof(uint8_t)));
                break;
            }
        }
    }
    else
    {
        colors = heap_caps_malloc(writeCount * sizeof(uint32_t), MALLOC_CAP_DEFAULT);
        memcpy(colors, color, writeCount * (sizeof(uint32_t)/sizeof(uint8_t)));
    }
    ESP_RETURN_ON_FALSE(colors, ESP_ERR_NO_MEM, TAG, "no memory for write_seq memory allocation");
    uint8_t firstRegister = map[firstLedIndex].pos;
    uint8_t lastRegister = map[firstLedIndex+writeCount].pos;

    uint8_t byteCount = 1 + lastRegister - firstRegister;
    uint8_t* buf = malloc(byteCount * sizeof(uint8_t));
    buf[0] = map[firstLedIndex].pos;
    uint8_t lastPos = map[firstLedIndex].pos;
    uint8_t totalOffset = 0;
    for(int i = 0; i < writeCount; i++) //Goes through every register pwm in the map in the given area and determines if there are any holes in the between the registers for unused LEDs
    {
        if(i == 0) //Sets the first 3 LED values because the structure of lastPos doesn't allow for doing this differently
        {
            buf[i+1] = ((colors[i] & 0xFF0000) >> 16);
            buf[i+2] = ((colors[i] & 0x00FF00) >> 8);
            buf[i+3] = (colors[i] & 0x0000FF);
            continue;
        }
        if(map[firstLedIndex+i].pos == lastPos + 3) //If the current LED is consecutive to the previous LED, write the current led data to the buffer
        {
            buf[(i * 3) + 1 + totalOffset] = ((colors[i] & 0xFF0000) >> 16);
            buf[(i * 3) + 2 + totalOffset] = ((colors[i] & 0x00FF00) >> 8);
            buf[(i * 3) + 3 + totalOffset] = (colors[i] & 0x0000FF);
            /*for(int j = 0; j < 3; j++)
            {
                printf("data at %i: %x\n", (i*3)+j+1, buf[(i*3)+j+1+totalOffset]);
            }*/
            lastPos = map[firstLedIndex+i].pos;
            
        }
        else //Else determine where the next led starts
        {
            //printf("running offset at i: %i", i);
            uint8_t offset = map[firstLedIndex+i].pos - lastPos - 3;
            totalOffset += offset;
            for(int j = 0; j < offset; j++)
            {
                buf[(i * 3) + j] = 0x0;
            }
            buf[(i * 3) + 1 + totalOffset] = ((colors[i] & 0xFF0000) >> 16);
            buf[(i * 3) + 2 + totalOffset] = ((colors[i] & 0x00FF00) >> 8);
            buf[(i * 3) + 3 + totalOffset] = (colors[i] & 0x0000FF);
            lastPos = map[firstLedIndex+i].pos;
        }
        //printf("5 at i=%i: %x\n", i, buf[5]);
    }
    /*for(int i = 0; i < byteCount; i++)
    {
        printf("%i: %x\n", i, buf[i]);
    }*/
    ESP_RETURN_ON_ERROR(i2c_master_transmit(dev_hdl_pwm, buf, byteCount, -1), TAG, "unexpected error in set_color_single while transmitting the color data");
    for(int i = 0; i < writeCount; i++) //Updata the values for the current LED output, so time saving mechanisms work correctly
    {
        map[firstLedIndex + i].value = colors[i];
    }
    heap_caps_free(buf);
    return ESP_OK;
}
esp_err_t rgb_matrix_rainbow(rgb_matrix_map_t* map)
{
    esp_err_t err = ESP_OK;
    uint32_t cmap[] = {0xFF0000, 0xFFA500, 0xFFFF00, 0x00FF00, 0x0000FF};
    while (true)
    {
        
        
        for(int j = 0; j < 4; j++)
        {
            for(int i = 0; i < 5; i++)
            {
                err = rgb_matrix_set_color_single(map, (i+(j*5)), cmap[i]);
            }
        }
        int temp = cmap[4];  // store the last element

        for (int i = 4; i > 0; i--) {
            cmap[i] = cmap[i - 1];  // shift each element right
        }

        cmap[0] = temp;  // move the last element to the first position
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
    return err;
}
