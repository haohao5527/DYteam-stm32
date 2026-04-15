#include "vl53l0x_platform.h"
#include "vl53l0x_def.h"
#include "i2c.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define I2C_BUFFER_CONFIG 1

#if I2C_BUFFER_CONFIG == 0
uint8_t i2c_global_buffer[64];
#define VL53L0X_GetLocalBuffer(Dev, n_byte)  i2c_global_buffer
#elif I2C_BUFFER_CONFIG == 1
uint8_t LocBuffer[64];
#define VL53L0X_GetLocalBuffer(Dev, n_byte)  LocBuffer
#endif

VL53L0X_Error VL53L0X_LockSequenceAccess(VL53L0X_DEV Dev) {
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_UnlockSequenceAccess(VL53L0X_DEV Dev) {
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {
    uint8_t *buffer;
    uint8_t deviceAddress = Dev->I2cDevAddr;

    if (count >= 64) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    buffer = VL53L0X_GetLocalBuffer(Dev, 1);
    buffer[0] = index;
    memcpy(&buffer[1], pdata, count);

    if (HAL_I2C_Master_Transmit(&hi2c1, deviceAddress, buffer, count + 1, 100) != HAL_OK) {
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata, uint32_t count) {
    uint8_t *buffer;
    uint8_t deviceAddress = Dev->I2cDevAddr;

    if ((count + 1) > 64) {
        return VL53L0X_ERROR_INVALID_PARAMS;
    }

    buffer = VL53L0X_GetLocalBuffer(Dev, 1);
    buffer[0] = index;

    if (HAL_I2C_Master_Transmit(&hi2c1, deviceAddress, buffer, 1, 100) != HAL_OK) {
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }

    if (HAL_I2C_Master_Receive(&hi2c1, deviceAddress, pdata, count, 100) != HAL_OK) {
        return VL53L0X_ERROR_CONTROL_INTERFACE;
    }
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data) {
    return VL53L0X_WriteMulti(Dev, index, &data, 1);
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *pdata) {
    return VL53L0X_ReadMulti(Dev, index, pdata, 1);
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data) {
    uint8_t buf[2];
    buf[0] = (data >> 8) & 0xFF;
    buf[1] = data & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buf, 2);
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *pdata) {
    uint8_t buf[2];
    VL53L0X_Error status = VL53L0X_ReadMulti(Dev, index, buf, 2);
    *pdata = ((uint16_t)buf[0] << 8) | buf[1];
    return status;
}

VL53L0X_Error VL53L0X_WrDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t data) {
    uint8_t buf[4];
    buf[0] = (data >> 24) & 0xFF;
    buf[1] = (data >> 16) & 0xFF;
    buf[2] = (data >> 8) & 0xFF;
    buf[3] = data & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buf, 4);
}

VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *pdata) {
    uint8_t buf[4];
    VL53L0X_Error status = VL53L0X_ReadMulti(Dev, index, buf, 4);
    *pdata = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
    return status;
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData) {
    uint8_t data;
    VL53L0X_Error status = VL53L0X_RdByte(Dev, index, &data);
    if (status != VL53L0X_ERROR_NONE) return status;
    data = (data & AndData) | OrData;
    return VL53L0X_WrByte(Dev, index, data);
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev) {
    HAL_Delay(3);
    return VL53L0X_ERROR_NONE;
}
