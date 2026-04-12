#ifndef __EC11_H__
#define __EC11_H__

#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct {
    TIM_HandleTypeDef* htim;
    int16_t lastCount;
    uint8_t isStarted;
} EC11_HandleTypeDef;

HAL_StatusTypeDef EC11_Init(EC11_HandleTypeDef* handle, TIM_HandleTypeDef* htim);
HAL_StatusTypeDef EC11_DeInit(EC11_HandleTypeDef* handle);

HAL_StatusTypeDef EC11_Start(EC11_HandleTypeDef* handle);
HAL_StatusTypeDef EC11_Stop(EC11_HandleTypeDef* handle);

int16_t EC11_GetDelta(EC11_HandleTypeDef* handle);
int16_t EC11_GetCount(EC11_HandleTypeDef* handle);
void EC11_ResetCount(EC11_HandleTypeDef* handle);

uint8_t EC11_IsRunning(EC11_HandleTypeDef* handle);

#endif
