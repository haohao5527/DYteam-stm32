#include "encoder.h"

#define ENCODER_FILTER_ALPHA 0.7f

HAL_StatusTypeDef ENCODER_Init(ENCODER_HandleTypeDef* handle, TIM_HandleTypeDef* htim_encoder)
{
    if (handle == NULL || htim_encoder == NULL) {
        return HAL_ERROR;
    }

    handle->htim_encoder = htim_encoder;
    handle->last_count = 0;
    handle->position = 0;
    handle->last_update_time = 0;
    handle->speed_rpm = 0.0f;
    handle->filtered_speed_rpm = 0.0f;
    handle->initialized = 1;

    return HAL_OK;
}

HAL_StatusTypeDef ENCODER_DeInit(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    ENCODER_Stop(handle);

    handle->htim_encoder = NULL;
    handle->last_count = 0;
    handle->position = 0;
    handle->last_update_time = 0;
    handle->speed_rpm = 0.0f;
    handle->filtered_speed_rpm = 0.0f;
    handle->initialized = 0;

    return HAL_OK;
}

HAL_StatusTypeDef ENCODER_Start(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim_encoder == NULL) {
        return HAL_ERROR;
    }

    if (HAL_TIM_Encoder_Start(handle->htim_encoder, TIM_CHANNEL_ALL) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->last_count = __HAL_TIM_GET_COUNTER(handle->htim_encoder);
    handle->position = 0;
    handle->last_update_time = HAL_GetTick();

    return HAL_OK;
}

HAL_StatusTypeDef ENCODER_Stop(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL || handle->htim_encoder == NULL) {
        return HAL_ERROR;
    }

    HAL_TIM_Encoder_Stop(handle->htim_encoder, TIM_CHANNEL_ALL);

    return HAL_OK;
}

int32_t ENCODER_GetPosition(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0;
    }

    return handle->position;
}

float ENCODER_GetSpeedRPM(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0.0f;
    }

    return handle->speed_rpm;
}

float ENCODER_GetFilteredSpeedRPM(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0.0f;
    }

    return handle->filtered_speed_rpm;
}

void ENCODER_Update(ENCODER_HandleTypeDef* handle)
{
    if (handle == NULL || !handle->initialized) {
        return;
    }

    uint32_t current_count = __HAL_TIM_GET_COUNTER(handle->htim_encoder);
    uint32_t current_time = HAL_GetTick();
    uint32_t time_diff = current_time - handle->last_update_time;

    if (time_diff >= 10) {
        int32_t delta = (int32_t)current_count - (int32_t)handle->last_count;

        if (delta > 32768) {
            delta -= 65536;
        } else if (delta < -32768) {
            delta += 65536;
        }

        handle->position += delta;

        float pulses_per_second = (float)delta * 1000.0f / (float)time_diff;

        handle->speed_rpm = pulses_per_second * 60.0f / 1000.0f;

        handle->filtered_speed_rpm = ENCODER_FILTER_ALPHA * handle->speed_rpm +
                                      (1.0f - ENCODER_FILTER_ALPHA) * handle->filtered_speed_rpm;

        handle->last_count = current_count;
        handle->last_update_time = current_time;
    }
}