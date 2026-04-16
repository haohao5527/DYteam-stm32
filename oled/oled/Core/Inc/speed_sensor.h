#ifndef __SPEED_SENSOR_H__
#define __SPEED_SENSOR_H__

#include "main.h"
#include "stm32f1xx_hal.h"

#define SPEED_SENSOR_HOLES_PER_REV    20
#define WHEEL_DIAMETER_CM              6.5f
#define WHEEL_CIRCUMFERENCE_CM         (3.14159f * WHEEL_DIAMETER_CM)
#define SPEED_UPDATE_INTERVAL_MS       100
#define SPEED_FILTER_ALPHA             0.6f
#define SPEED_SENSOR_DEBOUNCE_US       1000

typedef struct {
    GPIO_TypeDef* sensor_port;
    uint16_t sensor_pin;
    IRQn_Type exti_irqn;

    volatile uint32_t pulse_count;
    volatile uint32_t last_pulse_count;
    volatile uint32_t last_update_time;
    volatile uint32_t last_interrupt_time;

    float current_speed_cm_s;
    float filtered_speed_cm_s;
    uint8_t isInitialized;
} SpeedSensor_HandleTypeDef;

typedef struct {
    SpeedSensor_HandleTypeDef leftSensor;
    SpeedSensor_HandleTypeDef rightSensor;
    uint8_t isDualMode;
} SpeedSensor_DualHandleTypeDef;

void SpeedSensor_SetTimCounter(uint32_t value);
uint32_t SpeedSensor_GetTimCounter(void);

HAL_StatusTypeDef SpeedSensor_Init(SpeedSensor_HandleTypeDef* handle,
                                   GPIO_TypeDef* port, uint16_t pin, IRQn_Type exti_irqn);

void SpeedSensor_Reset(SpeedSensor_HandleTypeDef* handle);
void SpeedSensor_IncrementPulse(SpeedSensor_HandleTypeDef* handle);

void SpeedSensor_Update(SpeedSensor_HandleTypeDef* handle);
float SpeedSensor_GetSpeedCmS(SpeedSensor_HandleTypeDef* handle);
float SpeedSensor_GetFilteredSpeedCmS(SpeedSensor_HandleTypeDef* handle);
uint32_t SpeedSensor_GetPulseCount(SpeedSensor_HandleTypeDef* handle);

void SpeedSensor_DualInit(SpeedSensor_DualHandleTypeDef* handle,
                         GPIO_TypeDef* leftPort, uint16_t leftPin, IRQn_Type leftIrqn,
                         GPIO_TypeDef* rightPort, uint16_t rightPin, IRQn_Type rightIrqn);

void SpeedSensor_DualReset(SpeedSensor_DualHandleTypeDef* handle);
void SpeedSensor_DualUpdate(SpeedSensor_DualHandleTypeDef* handle);

float SpeedSensor_DualGetLeftSpeedCmS(SpeedSensor_DualHandleTypeDef* handle);
float SpeedSensor_DualGetRightSpeedCmS(SpeedSensor_DualHandleTypeDef* handle);
float SpeedSensor_DualGetFilteredLeftSpeedCmS(SpeedSensor_DualHandleTypeDef* handle);
float SpeedSensor_DualGetFilteredRightSpeedCmS(SpeedSensor_DualHandleTypeDef* handle);

uint32_t SpeedSensor_DualGetLeftPulseCount(SpeedSensor_DualHandleTypeDef* handle);
uint32_t SpeedSensor_DualGetRightPulseCount(SpeedSensor_DualHandleTypeDef* handle);

void SpeedSensor_DualIncrementLeft(SpeedSensor_DualHandleTypeDef* handle);
void SpeedSensor_DualIncrementRight(SpeedSensor_DualHandleTypeDef* handle);

#endif
