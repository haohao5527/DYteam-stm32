#include "red_reflection.h"

uint8_t READ_RED_REFLECTION(void)
{
    uint8_t GPIOC_PIN_13_STATE = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    return GPIOC_PIN_13_STATE;
}
