#ifndef __SPEED_SENSOR_H__
#define __SPEED_SENSOR_H__

#include "main.h"
#include "stm32f1xx_hal.h"

#define SPEED_SENSOR_HOLES_PER_REV    20
#define WHEEL_DIAMETER_CM              6.5f
#define WHEEL_CIRCUMFERENCE_CM         (3.14159f * WHEEL_DIAMETER_CM)
#define SPEED_UPDATE_INTERVAL_MS       100
#define SPEED_FILTER_ALPHA             0.6f

typedef struct {
    GPIO_TypeDef* sensor_port;
    uint16_t sensor_pin;

    volatile uint32_t pulse_count;
    volatile uint32_t last_pulse_count;
    volatile uint32_t last_update_time;

    float current_speed_cm_s;
    float filtered_speed_cm_s;
} SpeedSensor_HandleTypeDef;

HAL_StatusTypeDef SpeedSensor_Init(SpeedSensor_HandleTypeDef* handle,
                                   GPIO_TypeDef* port, uint16_t pin);

void SpeedSensor_Reset(SpeedSensor_HandleTypeDef* handle);
void SpeedSensor_IncrementPulse(SpeedSensor_HandleTypeDef* handle);

void SpeedSensor_Update(SpeedSensor_HandleTypeDef* handle);
float SpeedSensor_GetSpeedCmS(SpeedSensor_HandleTypeDef* handle);
float SpeedSensor_GetFilteredSpeedCmS(SpeedSensor_HandleTypeDef* handle);

#endif