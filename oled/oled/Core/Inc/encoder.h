#ifndef __ENCODER_H__
#define __ENCODER_H__

#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct {
    TIM_HandleTypeDef* htim_encoder;
    uint32_t last_count;
    int32_t position;
    uint32_t last_update_time;
    float speed_rpm;
    float filtered_speed_rpm;
    uint8_t initialized;
} ENCODER_HandleTypeDef;

HAL_StatusTypeDef ENCODER_Init(ENCODER_HandleTypeDef* handle, TIM_HandleTypeDef* htim_encoder);
HAL_StatusTypeDef ENCODER_DeInit(ENCODER_HandleTypeDef* handle);

HAL_StatusTypeDef ENCODER_Start(ENCODER_HandleTypeDef* handle);
HAL_StatusTypeDef ENCODER_Stop(ENCODER_HandleTypeDef* handle);

int32_t ENCODER_GetPosition(ENCODER_HandleTypeDef* handle);
float ENCODER_GetSpeedRPM(ENCODER_HandleTypeDef* handle);
float ENCODER_GetFilteredSpeedRPM(ENCODER_HandleTypeDef* handle);

void ENCODER_Update(ENCODER_HandleTypeDef* handle);

#endif