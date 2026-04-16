#ifndef __EC11_H__
#define __EC11_H__

#include "main.h"
#include "stm32f1xx_hal.h"

#define EC11_DEBOUNCE_TICKS    5

typedef struct {
    TIM_HandleTypeDef* htim;
    int16_t lastCount;
    uint8_t isStarted;

    GPIO_TypeDef* buttonPort;
    uint16_t buttonPin;
    volatile uint8_t buttonPressed;
    volatile uint8_t buttonLastState;
    volatile uint16_t debounceCounter;
} EC11_HandleTypeDef;

typedef struct {
    EC11_HandleTypeDef leftEncoder;
    EC11_HandleTypeDef rightEncoder;
} EC11_DualHandleTypeDef;

HAL_StatusTypeDef EC11_Init(EC11_HandleTypeDef* handle, TIM_HandleTypeDef* htim);
HAL_StatusTypeDef EC11_InitWithButton(EC11_HandleTypeDef* handle, TIM_HandleTypeDef* htim,
                                       GPIO_TypeDef* buttonPort, uint16_t buttonPin);
HAL_StatusTypeDef EC11_DeInit(EC11_HandleTypeDef* handle);

HAL_StatusTypeDef EC11_Start(EC11_HandleTypeDef* handle);
HAL_StatusTypeDef EC11_Stop(EC11_HandleTypeDef* handle);

int16_t EC11_GetDelta(EC11_HandleTypeDef* handle);
int16_t EC11_GetCount(EC11_HandleTypeDef* handle);
void EC11_ResetCount(EC11_HandleTypeDef* handle);

uint8_t EC11_IsRunning(EC11_HandleTypeDef* handle);

uint8_t EC11_GetButton(EC11_HandleTypeDef* handle);
uint8_t EC11_IsButtonPressed(EC11_HandleTypeDef* handle);
void EC11_UpdateButton(EC11_HandleTypeDef* handle);

void EC11_DualInit(EC11_DualHandleTypeDef* handle,
                  TIM_HandleTypeDef* left_htim, GPIO_TypeDef* left_buttonPort, uint16_t left_buttonPin,
                  TIM_HandleTypeDef* right_htim, GPIO_TypeDef* right_buttonPort, uint16_t right_buttonPin);

HAL_StatusTypeDef EC11_DualStart(EC11_DualHandleTypeDef* handle);
HAL_StatusTypeDef EC11_DualStop(EC11_DualHandleTypeDef* handle);

int16_t EC11_DualGetDelta(EC11_DualHandleTypeDef* handle, uint8_t encoderIndex);
int16_t EC11_DualGetLeftDelta(EC11_DualHandleTypeDef* handle);
int16_t EC11_DualGetRightDelta(EC11_DualHandleTypeDef* handle);

#endif
