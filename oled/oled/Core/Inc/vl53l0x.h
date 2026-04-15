#ifndef __VL53L0X_H__
#define __VL53L0X_H__

#include "main.h"
#include "vl53l0x_api.h"

typedef struct {
    VL53L0X_Dev_t dev;
    uint16_t distance_mm;
    uint8_t is_valid;
} VL53L0X_HandleTypeDef;

HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef* handle, I2C_HandleTypeDef* hi2c);
HAL_StatusTypeDef VL53L0X_ReadDistance(VL53L0X_HandleTypeDef* handle);
uint16_t VL53L0X_GetDistance(VL53L0X_HandleTypeDef* handle);
uint8_t VL53L0X_IsValid(VL53L0X_HandleTypeDef* handle);

#endif
