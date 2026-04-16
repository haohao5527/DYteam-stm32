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
    int8_t speed;
    TB6612_DirectionTypeDef direction;
} TB6612_HandleTypeDef;

typedef struct {
    TB6612_HandleTypeDef leftMotor;
    TB6612_HandleTypeDef rightMotor;
} TB6612_DualHandleTypeDef;

HAL_StatusTypeDef TB6612_Init(TB6612_HandleTypeDef* handle,
                               GPIO_TypeDef* IN1_Port, uint16_t IN1_Pin,
                               GPIO_TypeDef* IN2_Port, uint16_t IN2_Pin,
                               TIM_HandleTypeDef* pwmTimer, uint32_t pwmChannel);

HAL_StatusTypeDef TB6612_DeInit(TB6612_HandleTypeDef* handle);

HAL_StatusTypeDef TB6612_Start(TB6612_HandleTypeDef* handle);
HAL_StatusTypeDef TB6612_Stop(TB6612_HandleTypeDef* handle);

HAL_StatusTypeDef TB6612_SetDirection(TB6612_HandleTypeDef* handle, TB6612_DirectionTypeDef direction);
HAL_StatusTypeDef TB6612_SetSpeed(TB6612_HandleTypeDef* handle, int8_t speed);
HAL_StatusTypeDef TB6612_SetSpeedRaw(TB6612_HandleTypeDef* handle, uint16_t pulse);

TB6612_DirectionTypeDef TB6612_GetDirection(TB6612_HandleTypeDef* handle);
int8_t TB6612_GetSpeed(TB6612_HandleTypeDef* handle);
uint8_t TB6612_IsRunning(TB6612_HandleTypeDef* handle);

void TB6612_DualInit(TB6612_DualHandleTypeDef* handle,
                     GPIO_TypeDef* left_IN1_Port, uint16_t left_IN1_Pin,
                     GPIO_TypeDef* left_IN2_Port, uint16_t left_IN2_Pin,
                     TIM_HandleTypeDef* left_pwmTimer, uint32_t left_pwmChannel,
                     GPIO_TypeDef* right_IN1_Port, uint16_t right_IN1_Pin,
                     GPIO_TypeDef* right_IN2_Port, uint16_t right_IN2_Pin,
                     TIM_HandleTypeDef* right_pwmTimer, uint32_t right_pwmChannel);

HAL_StatusTypeDef TB6612_DualStart(TB6612_DualHandleTypeDef* handle);
HAL_StatusTypeDef TB6612_DualStop(TB6612_DualHandleTypeDef* handle);

void TB6612_DualSetSpeed(TB6612_DualHandleTypeDef* handle, int8_t leftSpeed, int8_t rightSpeed);
void TB6612_DualSetDirection(TB6612_DualHandleTypeDef* handle,
                             TB6612_DirectionTypeDef leftDir, TB6612_DirectionTypeDef rightDir);
void TB6612_DualMove(TB6612_DualHandleTypeDef* handle, int8_t speed);

#endif
