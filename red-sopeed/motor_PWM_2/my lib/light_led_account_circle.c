#include "light_led_account_circle.h"
#include "red_reflection.h"
#include "measure_circle.h"  // for MEASURE_CIRCLE()

void LIGHT_LED_ACCOUNT_CIRCLE(void)
{
    uint32_t circle = MEASURE_CIRCLE();

    for (uint32_t i = 0; i < circle; i++)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        HAL_Delay(1000);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        HAL_Delay(1000);
    }
}

void LIGHT_LED_LIVING(void)
{
    if (READ_RED_REFLECTION() == GPIO_PIN_RESET)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        // HAL_Delay(1000);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        // HAL_Delay(1000);
    }
}
