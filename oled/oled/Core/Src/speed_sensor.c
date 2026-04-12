#include "speed_sensor.h"

HAL_StatusTypeDef SpeedSensor_Init(SpeedSensor_HandleTypeDef* handle,
                                   GPIO_TypeDef* port, uint16_t pin)
{
    if (handle == NULL || port == NULL) {
        return HAL_ERROR;
    }

    handle->sensor_port = port;
    handle->sensor_pin = pin;
    handle->pulse_count = 0;
    handle->last_pulse_count = 0;
    handle->last_update_time = 0;
    handle->current_speed_cm_s = 0.0f;
    handle->filtered_speed_cm_s = 0.0f;

    return HAL_OK;
}

void SpeedSensor_Reset(SpeedSensor_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    handle->pulse_count = 0;
    handle->last_pulse_count = 0;
    handle->last_update_time = 0;
    handle->current_speed_cm_s = 0.0f;
    handle->filtered_speed_cm_s = 0.0f;
}

void SpeedSensor_IncrementPulse(SpeedSensor_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    handle->pulse_count++;
}

void SpeedSensor_Update(SpeedSensor_HandleTypeDef* handle)
{
    uint32_t current_time;
    uint32_t time_delta_ms;
    uint32_t pulse_delta;
    float speed;

    if (handle == NULL) {
        return;
    }

    current_time = HAL_GetTick();
    time_delta_ms = current_time - handle->last_update_time;

    if (time_delta_ms >= SPEED_UPDATE_INTERVAL_MS) {
        pulse_delta = handle->pulse_count - handle->last_pulse_count;

        handle->current_speed_cm_s = (float)pulse_delta * WHEEL_CIRCUMFERENCE_CM * 1000.0f / (float)time_delta_ms / (float)SPEED_SENSOR_HOLES_PER_REV;

        speed = handle->current_speed_cm_s;

        handle->filtered_speed_cm_s = handle->filtered_speed_cm_s * (1.0f - SPEED_FILTER_ALPHA) + speed * SPEED_FILTER_ALPHA;

        handle->last_pulse_count = handle->pulse_count;
        handle->last_update_time = current_time;
    }
}

float SpeedSensor_GetSpeedCmS(SpeedSensor_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0.0f;
    }

    return handle->current_speed_cm_s;
}

float SpeedSensor_GetFilteredSpeedCmS(SpeedSensor_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0.0f;
    }

    return handle->filtered_speed_cm_s;
}