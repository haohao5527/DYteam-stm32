#include "speed_sensor.h"

static volatile uint32_t g_tim4_counter = 0;

void SpeedSensor_SetTimCounter(uint32_t value)
{
    g_tim4_counter = value;
}

uint32_t SpeedSensor_GetTimCounter(void)
{
    return g_tim4_counter;
}

HAL_StatusTypeDef SpeedSensor_Init(SpeedSensor_HandleTypeDef* handle,
                                   GPIO_TypeDef* port, uint16_t pin, IRQn_Type exti_irqn)
{
    if (handle == NULL || port == NULL) {
        return HAL_ERROR;
    }

    handle->sensor_port = port;
    handle->sensor_pin = pin;
    handle->exti_irqn = exti_irqn;
    handle->pulse_count = 0;
    handle->last_pulse_count = 0;
    handle->last_update_time = 0;
    handle->last_interrupt_time = 0;
    handle->current_speed_cm_s = 0.0f;
    handle->filtered_speed_cm_s = 0.0f;
    handle->isInitialized = 1;

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
    handle->last_interrupt_time = 0;
    handle->current_speed_cm_s = 0.0f;
    handle->filtered_speed_cm_s = 0.0f;
}

void SpeedSensor_IncrementPulse(SpeedSensor_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    uint32_t current_time = SpeedSensor_GetTimCounter();
    if (current_time - handle->last_interrupt_time > SPEED_SENSOR_DEBOUNCE_US) {
        handle->pulse_count++;
        handle->last_interrupt_time = current_time;
    }
}

void SpeedSensor_Update(SpeedSensor_HandleTypeDef* handle)
{
    uint32_t current_time_ms;
    uint32_t time_delta_ms;
    uint32_t pulse_delta;
    float speed;

    if (handle == NULL || !handle->isInitialized) {
        return;
    }

    current_time_ms = HAL_GetTick();
    time_delta_ms = current_time_ms - handle->last_update_time;

    if (time_delta_ms >= SPEED_UPDATE_INTERVAL_MS) {
        pulse_delta = handle->pulse_count - handle->last_pulse_count;

        handle->current_speed_cm_s = (float)pulse_delta * WHEEL_CIRCUMFERENCE_CM * 1000.0f /
                                     (float)time_delta_ms / (float)SPEED_SENSOR_HOLES_PER_REV;

        speed = handle->current_speed_cm_s;
        handle->filtered_speed_cm_s = handle->filtered_speed_cm_s * (1.0f - SPEED_FILTER_ALPHA) +
                                       speed * SPEED_FILTER_ALPHA;

        handle->last_pulse_count = handle->pulse_count;
        handle->last_update_time = current_time_ms;
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

uint32_t SpeedSensor_GetPulseCount(SpeedSensor_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return handle->pulse_count;
}

void SpeedSensor_DualInit(SpeedSensor_DualHandleTypeDef* handle,
                         GPIO_TypeDef* leftPort, uint16_t leftPin, IRQn_Type leftIrqn,
                         GPIO_TypeDef* rightPort, uint16_t rightPin, IRQn_Type rightIrqn)
{
    if (handle == NULL) {
        return;
    }

    SpeedSensor_Init(&handle->leftSensor, leftPort, leftPin, leftIrqn);
    SpeedSensor_Init(&handle->rightSensor, rightPort, rightPin, rightIrqn);
    handle->isDualMode = 1;
}

void SpeedSensor_DualReset(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    SpeedSensor_Reset(&handle->leftSensor);
    SpeedSensor_Reset(&handle->rightSensor);
}

void SpeedSensor_DualUpdate(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    SpeedSensor_Update(&handle->leftSensor);
    SpeedSensor_Update(&handle->rightSensor);
}

float SpeedSensor_DualGetLeftSpeedCmS(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0.0f;
    }

    return SpeedSensor_GetSpeedCmS(&handle->leftSensor);
}

float SpeedSensor_DualGetRightSpeedCmS(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0.0f;
    }

    return SpeedSensor_GetSpeedCmS(&handle->rightSensor);
}

float SpeedSensor_DualGetFilteredLeftSpeedCmS(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0.0f;
    }

    return SpeedSensor_GetFilteredSpeedCmS(&handle->leftSensor);
}

float SpeedSensor_DualGetFilteredRightSpeedCmS(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0.0f;
    }

    return SpeedSensor_GetFilteredSpeedCmS(&handle->rightSensor);
}

uint32_t SpeedSensor_DualGetLeftPulseCount(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return SpeedSensor_GetPulseCount(&handle->leftSensor);
}

uint32_t SpeedSensor_DualGetRightPulseCount(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return SpeedSensor_GetPulseCount(&handle->rightSensor);
}

void SpeedSensor_DualIncrementLeft(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    SpeedSensor_IncrementPulse(&handle->leftSensor);
}

void SpeedSensor_DualIncrementRight(SpeedSensor_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return;
    }

    SpeedSensor_IncrementPulse(&handle->rightSensor);
}
