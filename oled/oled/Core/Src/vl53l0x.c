#include "vl53l0x.h"
#include "gpio.h"

static void VL53L0X_Reset(VL53L0X_HandleTypeDef* handle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (handle == NULL || handle->xshut_port == NULL) return;

    GPIO_InitStruct.Pin = handle->xshut_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(handle->xshut_port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(handle->xshut_port, handle->xshut_pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(handle->xshut_port, handle->xshut_pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef* handle,
                                I2C_HandleTypeDef* hi2c,
                                GPIO_TypeDef* xshut_port, uint16_t xshut_pin)
{
    uint8_t vhv_settings;
    uint8_t phase_cal;
    uint32_t ref_spad_count;
    uint8_t is_aperture_spads;

    if (handle == NULL || hi2c == NULL) return HAL_ERROR;

    handle->hi2c = hi2c;
    handle->xshut_port = xshut_port;
    handle->xshut_pin = xshut_pin;
    handle->distance_mm = 0;
    handle->is_valid = 0;

    VL53L0X_Reset(handle);

    handle->dev.I2cDevAddr = VL53L0X_I2C_ADDR;
    handle->dev.comms_type = 0;
    handle->dev.comms_speed_khz = 400;

    if (VL53L0X_DataInit(&handle->dev) != VL53L0X_ERROR_NONE)
        return HAL_ERROR;

    VL53L0X_StaticInit(&handle->dev);
    VL53L0X_PerformRefCalibration(&handle->dev, &vhv_settings, &phase_cal);
    VL53L0X_PerformRefSpadManagement(&handle->dev, &ref_spad_count, &is_aperture_spads);
    VL53L0X_SetDeviceMode(&handle->dev, VL53L0X_DEVICEMODE_SINGLE_RANGING);

    return HAL_OK;
}

HAL_StatusTypeDef VL53L0X_DeInit(VL53L0X_HandleTypeDef* handle)
{
    if (handle == NULL) return HAL_ERROR;

    handle->hi2c = NULL;
    handle->xshut_port = NULL;
    handle->xshut_pin = 0;
    handle->distance_mm = 0;
    handle->is_valid = 0;

    return HAL_OK;
}

HAL_StatusTypeDef VL53L0X_ReadDistance(VL53L0X_HandleTypeDef* handle)
{
    VL53L0X_RangingMeasurementData_t ranging_data;

    if (handle == NULL) return HAL_ERROR;

    VL53L0X_StartMeasurement(&handle->dev);

    do {
        HAL_Delay(10);
        if (VL53L0X_GetRangingMeasurementData(&handle->dev, &ranging_data) == VL53L0X_ERROR_NONE) {
            if (ranging_data.RangeStatus == 0) {
                handle->distance_mm = ranging_data.RangeMilliMeter;
                handle->is_valid = 1;
            } else {
                handle->is_valid = 0;
            }
            VL53L0X_ClearInterruptMask(&handle->dev, 0);
            break;
        }
    } while (1);

    return HAL_OK;
}

uint16_t VL53L0X_GetDistance(VL53L0X_HandleTypeDef* handle)
{
    if (handle == NULL) return 0;
    return handle->distance_mm;
}

uint8_t VL53L0X_IsValid(VL53L0X_HandleTypeDef* handle)
{
    if (handle == NULL) return 0;
    return handle->is_valid;
}
