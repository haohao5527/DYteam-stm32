#include "ir_tracking.h"

HAL_StatusTypeDef IR_TrackSensor_Init(IR_TrackSensor_HandleTypeDef* handle,
                                       GPIO_TypeDef* port, uint16_t pin)
{
    if (handle == NULL || port == NULL) {
        return HAL_ERROR;
    }

    handle->sensorPort = port;
    handle->sensorPin = pin;
    handle->status = IR_TRACK_LINE_NONE;
    handle->lastStatus = IR_TRACK_LINE_NONE;
    handle->lastChangeTime = 0;
    handle->isInitialized = 1;

    return HAL_OK;
}

void IR_TrackSensor_Update(IR_TrackSensor_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->isInitialized) {
        return;
    }

    GPIO_PinState state = HAL_GPIO_ReadPin(handle->sensorPort, handle->sensorPin);
    IR_TrackStatusTypeDef newStatus = (state == GPIO_PIN_RESET) ? IR_TRACK_LINE_DETECTED : IR_TRACK_LINE_NONE;

    if (newStatus != handle->status) {
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - handle->lastChangeTime >= IR_TRACKING_DEBOUNCE_MS) {
            handle->lastStatus = handle->status;
            handle->status = newStatus;
            handle->lastChangeTime = currentTime;
        }
    }
}

IR_TrackStatusTypeDef IR_TrackSensor_GetStatus(IR_TrackSensor_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->isInitialized) {
        return IR_TRACK_LINE_NONE;
    }

    return handle->status;
}

uint8_t IR_TrackSensor_IsLineDetected(IR_TrackSensor_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->isInitialized) {
        return 0;
    }

    return handle->status == IR_TRACK_LINE_DETECTED;
}

void IR_TrackSensor_ArrayInit(IR_TrackSensor_ArrayHandleTypeDef* handle,
                              GPIO_TypeDef* leftPort, uint16_t leftPin,
                              GPIO_TypeDef* middlePort, uint16_t middlePin,
                              GPIO_TypeDef* rightPort, uint16_t rightPin)
{
    if (handle == NULL) {
        return;
    }

    IR_TrackSensor_Init(&handle->leftSensor, leftPort, leftPin);
    IR_TrackSensor_Init(&handle->middleSensor, middlePort, middlePin);
    IR_TrackSensor_Init(&handle->rightSensor, rightPort, rightPin);
    handle->linePattern = IR_TRACK_PATTERN_NONE;
}

void IR_TrackSensor_ArrayUpdate(IR_TrackSensor_ArrayHandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    IR_TrackSensor_Update(&handle->leftSensor);
    IR_TrackSensor_Update(&handle->middleSensor);
    IR_TrackSensor_Update(&handle->rightSensor);

    handle->linePattern = 0;
    if (handle->leftSensor.status == IR_TRACK_LINE_DETECTED) {
        handle->linePattern |= IR_TRACK_PATTERN_LEFT;
    }
    if (handle->middleSensor.status == IR_TRACK_LINE_DETECTED) {
        handle->linePattern |= IR_TRACK_PATTERN_MIDDLE;
    }
    if (handle->rightSensor.status == IR_TRACK_LINE_DETECTED) {
        handle->linePattern |= IR_TRACK_PATTERN_RIGHT;
    }
}

uint8_t IR_TrackSensor_ArrayGetPattern(IR_TrackSensor_ArrayHandleTypeDef* handle)
{
    if (handle == NULL) {
        return IR_TRACK_PATTERN_NONE;
    }

    return handle->linePattern;
}