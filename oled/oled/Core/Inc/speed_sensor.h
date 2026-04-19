#ifndef __SPEED_SENSOR_H
#define __SPEED_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

typedef struct {
    GPIO_TypeDef* Port;
    uint16_t Pin;
    volatile uint32_t pulse_count;
    uint8_t pulse_divider;
    volatile uint8_t raw_counter;
    volatile uint32_t last_pulse_count;
    float speed_cm_s;
    float speed_buffer[4];
    uint8_t speed_buf_idx;
    uint8_t use_filter;
} SpeedSensor_HandleTypeDef;

void SpeedSensor_Init(SpeedSensor_HandleTypeDef* hsensor, GPIO_TypeDef* Port, uint16_t Pin, uint8_t pulse_divider, uint8_t use_filter);
void SpeedSensor_IncrementPulse(SpeedSensor_HandleTypeDef* hsensor);
uint32_t SpeedSensor_GetPulseCount(SpeedSensor_HandleTypeDef* hsensor);
void SpeedSensor_ResetPulseCount(SpeedSensor_HandleTypeDef* hsensor);
void SpeedSensor_UpdateSpeed(SpeedSensor_HandleTypeDef* hsensor, float cm_per_pulse);

#ifdef __cplusplus
}
#endif

#endif
