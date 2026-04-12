#ifndef __TB6612_H__
#define __TB6612_H__

#include "main.h"
#include "stm32f1xx_hal.h"

typedef enum {
    TB6612_DIR_FORWARD = 0,
    TB6612_DIR_BACKWARD = 1,
    TB6612_DIR_BRAKE = 2,
    TB6612_DIR_STOP = 3
} TB6612_DirectionTypeDef;

typedef struct {
    GPIO_TypeDef* IN1_Port;
    uint16_t IN1_Pin;
    GPIO_TypeDef* IN2_Port;
    uint16_t IN2_Pin;
    TIM_HandleTypeDef* pwmTimer;
    uint32_t pwmChannel;
    uint8_t isStarted;
} TB6612_HandleTypeDef;

HAL_StatusTypeDef TB6612_Init(TB6612_HandleTypeDef* handle,
                               GPIO_TypeDef* IN1_Port, uint16_t IN1_Pin,
                               GPIO_TypeDef* IN2_Port, uint16_t IN2_Pin,
                               TIM_HandleTypeDef* pwmTimer, uint32_t pwmChannel);

HAL_StatusTypeDef TB6612_DeInit(TB6612_HandleTypeDef* handle);

HAL_StatusTypeDef TB6612_Start(TB6612_HandleTypeDef* handle);
HAL_StatusTypeDef TB6612_Stop(TB6612_HandleTypeDef* handle);

HAL_StatusTypeDef TB6612_SetDirection(TB6612_HandleTypeDef* handle, TB6612_DirectionTypeDef direction);
HAL_StatusTypeDef TB6612_SetSpeed(TB6612_HandleTypeDef* handle, uint8_t speed);

TB6612_DirectionTypeDef TB6612_GetDirection(TB6612_HandleTypeDef* handle);
uint8_t TB6612_GetSpeed(TB6612_HandleTypeDef* handle);
uint8_t TB6612_IsRunning(TB6612_HandleTypeDef* handle);

#endif
