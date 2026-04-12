#include "ec11.h"

HAL_StatusTypeDef EC11_Init(EC11_HandleTypeDef* handle, TIM_HandleTypeDef* htim)
{
    if (handle == NULL || htim == NULL) {
        return HAL_ERROR;
    }

    handle->htim = htim;
    handle->lastCount = 0;
    handle->isStarted = 0;

    __HAL_TIM_SET_COUNTER(htim, 0);

    return HAL_OK;
}

HAL_StatusTypeDef EC11_DeInit(EC11_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    EC11_Stop(handle);

    handle->htim = NULL;
    handle->lastCount = 0;

    return HAL_OK;
}

HAL_StatusTypeDef EC11_Start(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim == NULL) {
        return HAL_ERROR;
    }

    if (handle->isStarted) {
        return HAL_OK;
    }

    if (HAL_TIM_Encoder_Start(handle->htim, TIM_CHANNEL_ALL) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->lastCount = (int16_t)__HAL_TIM_GET_COUNTER(handle->htim);
    handle->isStarted = 1;

    return HAL_OK;
}

HAL_StatusTypeDef EC11_Stop(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim == NULL) {
        return HAL_ERROR;
    }

    if (!handle->isStarted) {
        return HAL_OK;
    }

    HAL_TIM_Encoder_Stop(handle->htim, TIM_CHANNEL_ALL);
    handle->isStarted = 0;

    return HAL_OK;
}

int16_t EC11_GetDelta(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim == NULL) {
        return 0;
    }

    int16_t currentCount = (int16_t)__HAL_TIM_GET_COUNTER(handle->htim);
    int16_t delta = currentCount - handle->lastCount;
    handle->lastCount = currentCount;

    return delta;
}

int16_t EC11_GetCount(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim == NULL) {
        return 0;
    }

    return (int16_t)__HAL_TIM_GET_COUNTER(handle->htim);
}

void EC11_ResetCount(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim == NULL) {
        return;
    }

    __HAL_TIM_SET_COUNTER(handle->htim, 0);
    handle->lastCount = 0;
}

uint8_t EC11_IsRunning(EC11_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return handle->isStarted;
}
