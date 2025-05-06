#pragma once
#include "esp_types.h"


/**
 * Set this as follows:
 * Every item should have the offset to the corresponding LED in the pwm register set as pos
 * Only the register for the R value of the LED is of relevance for the pos value
 * The individual LEDs can then be called by their identifyer value so every LED should have a to the matrix number
 * @param map The identifier value
 */

typedef struct
{
    uint8_t map;
    uint8_t pos;
    uint32_t value;
}rgb_matrix_map_t;