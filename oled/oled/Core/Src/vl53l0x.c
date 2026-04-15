#include "vl53l0x.h"

extern I2C_HandleTypeDef hi2c1;

#define VL53L0X_I2C_ADDR   0x52

HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef* handle, I2C_HandleTypeDef* hi2c) {
    uint8_t vhv_settings;
    uint8_t phase_cal;
    uint32_t ref_spad_count;
    uint8_t is_aperture_spads;

    if (handle == NULL) {
        return HAL_ERROR;
    }

    handle->dev.I2cDevAddr = VL53L0X_I2C_ADDR;
    handle->dev.comms_type = 0;
    handle->dev.comms_speed_khz = 400;
    handle->distance_mm = 0;
    handle->is_valid = 0;

    if (VL53L0X_DataInit(&handle->dev) != VL53L0X_ERROR_NONE) {
        return HAL_ERROR;
    }

    VL53L0X_StaticInit(&handle->dev);

    VL53L0X_PerformRefCalibration(&handle->dev, &vhv_settings, &phase_cal);
    VL53L0X_PerformRefSpadManagement(&handle->dev, &ref_spad_count, &is_aperture_spads);

    VL53L0X_SetDeviceMode(&handle->dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);

    VL53L0X_SetGpioConfig(&handle->dev, 0, VL53L0X_DEVICEMODE_SINGLE_RANGING,
                          VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY,
                          VL53L0X_INTERRUPTPOLARITY_LOW);

    VL53L0X_StartMeasurement(&handle->dev);

    return HAL_OK;
}

HAL_StatusTypeDef VL53L0X_ReadDistance(VL53L0X_HandleTypeDef* handle) {
    VL53L0X_RangingMeasurementData_t ranging_data;

    if (handle == NULL) {
        return HAL_ERROR;
    }

    if (VL53L0X_GetRangingMeasurementData(&handle->dev, &ranging_data) == VL53L0X_ERROR_NONE) {
        if (ranging_data.RangeStatus == 0) {
            handle->distance_mm = ranging_data.RangeMilliMeter;
            handle->is_valid = 1;
        } else {
            handle->is_valid = 0;
        }
        VL53L0X_ClearInterruptMask(&handle->dev, 0);
    }

    return HAL_OK;
}

uint16_t VL53L0X_GetDistance(VL53L0X_HandleTypeDef* handle) {
    if (handle == NULL) {
        return 0;
    }
    return handle->distance_mm;
}

uint8_t VL53L0X_IsValid(VL53L0X_HandleTypeDef* handle) {
    if (handle == NULL) {
        return 0;
    }
    return handle->is_valid;
}
