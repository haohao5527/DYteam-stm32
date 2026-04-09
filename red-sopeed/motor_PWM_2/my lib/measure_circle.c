#include "measure_circle.h"
#include "red_reflection.h"

uint32_t MEASURE_CIRCLE(void)
{
    uint32_t circle = 0;
    while (READ_RED_REFLECTION() == GPIO_PIN_RESET)
    {
        circle++;
    }
    return circle;
}