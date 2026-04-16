#ifndef __VL53L0X_H__
#define __VL53L0X_H__

#include "main.h"
#include "vl53l0x_api.h"

#define VL53L0X_I2C_ADDR   0x52

typedef struct {
    VL53L0X_Dev_t dev;
    I2C_HandleTypeDef* hi2c;
    GPIO_TypeDef* xshut_port;
    uint16_t xshut_pin;
    uint16_t distance_mm;
    uint8_t is_valid;
} VL53L0X_HandleTypeDef;

HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef* handle,
                                I2C_HandleTypeDef* hi2c,
                                GPIO_TypeDef* xshut_port, uint16_t xshut_pin);
HAL_StatusTypeDef VL53L0X_DeInit(VL53L0X_HandleTypeDef* handle);

HAL_StatusTypeDef VL53L0X_ReadDistance(VL53L0X_HandleTypeDef* handle);
uint16_t VL53L0X_GetDistance(VL53L0X_HandleTypeDef* handle);
uint8_t VL53L0X_IsValid(VL53L0X_HandleTypeDef* handle);

#endif
