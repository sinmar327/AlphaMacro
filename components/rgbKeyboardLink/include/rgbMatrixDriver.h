#pragma once
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "rgbKeyboardTypes.h"


//////////////////
//  Registers  //
/////////////////

#define Chip_en 0x00 //Set to 0x01 to enable


/////////////////////////////////////////////////////////////////////////////////////

/**
 * Register  to define the ICs intial behaviour
 * @param Max_Line_Num_reg: number of scanlines (bit 6-3)
 * @param Data_Ref_Mode_reg: Data refresh mode selection (bit 2-1)
 * @param PWM_Fre_reg: Output PWM frequency (bit 0)
 */
#define Dev_initial 0x01

/**
 * Number of scan lines.
 * @param values 0x1-0xB
 * @param length 4 bit
 * @param default 0xB
 */
#define Max_Line_Num_reg 3

/**
 * Data refresh mode.
 * Datasheet page 19-20
 * @param 0x0 Mode1: 8-bit PWM resolution without VSYNC command required
 * @param 0x1 Mode2: 8-bit PWM resolution with VSYNC command required
 * @param 0x2_0x3 Mode3: 16-bit PWM resolution with VSYNC command required
 * @param length 2 bit
 * @param default 0x3
 */
#define Data_Ref_Mode_reg 1

/**
 * PWM frequency of the LED brightness dimming
 * @param 0x0 125kHz
 * @param 0x1 62.5kHz
 * @param length 1 bit
 * @param default 0x0
 */
#define PWM_Fre_reg 0

/////////////////////////////////////////////////////////////////////////////////////

/**
 * Configuration register 1
 * @param SW_BLK_reg Line switching blanking time
 * @param PWM_Scale_Mode_reg Dimming scale setting of final PWM generator
 * @param PWM_Phase_Shift_reg PWM phase shift selection
 * @param CS_ON_Shift_reg Current sink turn on delay setting
 */
#define Dev_config1 0x02
/**
 * Line switching blanking time
 * @param 0x0 1us
 * @param 0x1 0.5us
 * @param length 1 bit
 * @param default 0x0
 */
#define SW_BLK_reg 3

/**
 * Dimming curve setting of the PWM generator.
 * Datasheet page 18
 * @param 0x0 Linear dimming curve
 * @param 0x1 exponential dimming curve
 * @param length 1 bit
 * @param default 0x0
 */
#define PWM_Scale_Mode_reg 2

/**
 * Selection if a phase shift should occure in the PWM output signal to avoid high instantanious current draw.
 * There are 3 Phases:
 * Phase 1: CS0, CS3, CS6, CS9, CS12, CS15
 * Phase 2: CS1, CS4, CS7, CS10, CS13, CS16
 * Phase 3: CS2, CS5, CS8, CS11, CS14, CS17
 * Datasheet page 17-18
 * @param 0x0 Feature disabled
 * @param 0x1 Feature enabled
 * @param length 1 bit
 * @param default 0x0
 */
#define PWM_Phase_Shift_reg 1

/**
 * Selection if a delay should be added between the turn on of a line and the start of the pwm signal
 * Datasheet page 15 (Figure 8-3)
 * @param 0x0 Feature disabled
 * @param 0x1 Feature enabled
 * @param length 1 bit
 * @param default 0x0
 */
#define CS_ON_Shift_reg 0

/////////////////////////////////////////////////////////////////////////////////////

/**
 * Configuration register 2
 * @param Comp_Group3_reg Low brightness compensation clock shift number setting for group3
 * @param Comp_Group2_reg Low brightness compensation clock shift number setting for group2
 * @param Comp_Group1_reg Low brightness compensation clock shift number setting for group1
 * @param LOD_removal_reg LED Open Detection removal function enable
 * @param LSD_removal_reg LED Short Detection removal function enable
 */
#define Dev_config2 0x03
/**
 * Three groups compensation are implemented to overcome the colorshift and non-uniformity in low brightness conditions
 * @param 0x0 off
 * @param 0x1 1 clock
 * @param 0x2 2 clocks
 * @param 0x3 3 clocks
 * @param length 2 bit
 * @param default 0x0
 */
#define Comp_Group3_reg 6

/**
 * Three groups compensation are implemented to overcome the colorshift and non-uniformity in low brightness conditions
 * @param 0x0 off
 * @param 0x1 1 clock
 * @param 0x2 2 clocks
 * @param 0x3 3 clocks
 * @param length 2 bit
 * @param default 0x0
 */
#define Comp_Group2_reg 4

/**
 * Three groups compensation are implemented to overcome the colorshift and non-uniformity in low brightness conditions
 * @param 0x0 off
 * @param 0x1 1 clock
 * @param 0x2 2 clocks
 * @param 0x3 3 clocks
 * @param length 2 bit
 * @param default 0x0
 */
#define Comp_Group1_reg 2

/**
 * Selection if the LED Open Detection should be disabled
 * @param 0x0 Feature disabled
 * @param 0x1 Feature enabeld
 * @param length 1 bit
 * @param default 0x0
 */
#define LOD_removal_reg 1

/**
 * Selection if the LED Short Detection should be disabled
 * @param 0x0 Feature disabled
 * @param 0x1 Feature enabeld
 * @param length 1 bit
 * @param default 0x0
 */
#define LSD_removal_reg 0

/////////////////////////////////////////////////////////////////////////////////////

/**
 * Configuration register 3
 * @param Down_Deghost_reg Downside deghosting level selection
 * @param Up_Deghost_reg Scan line clamp voltage of upside deghosting
 * @param Maximum_Current_reg Maximum current cetting (MC)
 */
#define Dev_config3 0x04
/**
 * Downside deghosting: Setting if each current sink should be precharged when it is off in 3 intensitys 
 * @param 0x0 No deghosting
 * @param 0x1 Weak deghosting
 * @param 0x2 Medium deghosting
 * @param 0x3 Strong deghosting
 * @param length 2 bit
 * @param default 0x1
 */
#define Down_Deghost_reg 6

/**
 * Upside deghosting: Discharges each scan line in its off state and clamps it to the set voltage
 * @param 0x0 U = VLED - 2V
 * @param 0x1 U = VLED - 2.5V
 * @param 0x2 U = VLED - 3V
 * @param 0x3 U = GND
 * @param length 2 bit
 * @param default 0x1
 */
#define Up_Deghost_reg 4

/**
 * Maximum LED current
 * @param 0x0 3mA
 * @param 0x1 5mA
 * @param 0x2 10mA
 * @param 0x3 15mA
 * @param 0x4 20mA
 * @param 0x5 30mA
 * @param 0x6 40mA
 * @param 0x7 50mA
 * @param length 8 bit
 * @param default 0x3
 */
#define Maximum_Current_reg 1

/**
 * Enable for Upside deghosting
 * @param 0x0 Feature disabled
 * @param 0x1 Feature enabled
 * @param length 1 bit
 * @param default 0x1
 */
#define Up_Deghost_enable_reg 0

/**
 * Global PWM dimming
 * @param length 8 bit
 * @param default 0xFF
 */
#define Global_bri 0x05
/**7-bit value of the percentage, the colors current is reduced in relation to Imax set in Maximum_Current_reg register */
#define R_current_set 0x09
/**7-bit value of the percentage, the colors current is reduced in relation to Imax set in Maximum_Current_reg register */
#define G_current_set 0x0A
/**7-bit value of the percentage, the colors current is reduced in relation to Imax set in Maximum_Current_reg register */
#define B_current_set 0x0B
#define Dot_onoff0 0x43

/////////////////////////////////////////////////////////////////////////////////////

#define SET_REGISTER(regArray, valueLocation, valueLen, value) \
    do { \
        (regArray)[1] &= (unsigned char)(~(((1U << (valueLen)) - 1) << (valueLocation))); \
        (regArray)[1] |= (unsigned char)(((value) & ((1U << (valueLen)) - 1)) << (valueLocation)); \
    } while(0)





/**
 * Initializes the rgb matrix ic
 * @param bus_handle The i2c master bus handle to use
 * @param mutex A semathore handle needed to secure propper i2c communication of multiple devices on the same bus
 */
esp_err_t rgb_matrix_ic_init(const i2c_master_bus_handle_t* bus_handle, const SemaphoreHandle_t mutex);
/**
 * Automatically sets a map for a symetrical matrix. The elements in the map are sorted in register order.
 * @param lenght The number of LEDs to register
 * @param startRegL The first register that is connected to the first element of the map
 * @param lines The number of scanlines
 */
esp_err_t rgb_matrix_auto_register_conf(const uint8_t length, const uint8_t startRegL, const uint8_t lines, rgb_matrix_map_t* map);
/**
 * Sets the color of a single LED in the matrix
 * @param map A rgb_matrix_map_t* which contains all elements of the matrix
 * @param led The number of the LED whom color should be set
 * @param color An 24-bit value of RGB with 8 bits per color (e.g. 0xA0B1C2)
 * @param map An empty map variable that will be set to a corrosponding map
 */
esp_err_t rgb_matrix_set_color_single(rgb_matrix_map_t* map, const uint8_t led, const uint32_t color);
/** 
 * Makes the led Matrix blink in a simple rainbow pattern of {0xFF0000, 0xFFA500, 0xFFFF00, 0x00FF00, 0x0000FF}
 * @param map A rgb_matrix_map_t* which contains all elements of the matrix
*/
esp_err_t rgb_matrix_rainbow(rgb_matrix_map_t* map);
/**
 * Sets the color of a continuous seqence of colors. The map must be set accordingly to support this.
 * @param map A rgb_matrix_map_t* which contains all elements of the matrix
 * @param color An array of all colors in the sequence to be set (color must be in the format 0x123456)
 * @param firstLedIndex The map index of the first led in the sequence that should be changed
 */
esp_err_t rgb_matrix_set_color_seq(rgb_matrix_map_t* map, const uint32_t* color, const int8_t firstLedIndex, const uint8_t writeCount);

bool rgb_matrix_check_init();