#include "tb6612.h"

HAL_StatusTypeDef TB6612_Init(TB6612_HandleTypeDef* handle,
                               GPIO_TypeDef* IN1_Port, uint16_t IN1_Pin,
                               GPIO_TypeDef* IN2_Port, uint16_t IN2_Pin,
                               TIM_HandleTypeDef* pwmTimer, uint32_t pwmChannel)
{
    if (handle == NULL || IN1_Port == NULL || IN2_Port == NULL || pwmTimer == NULL) {
        return HAL_ERROR;
    }

    handle->IN1_Port = IN1_Port;
    handle->IN1_Pin = IN1_Pin;
    handle->IN2_Port = IN2_Port;
    handle->IN2_Pin = IN2_Pin;
    handle->pwmTimer = pwmTimer;
    handle->pwmChannel = pwmChannel;
    handle->isStarted = 0;

    HAL_GPIO_WritePin(IN1_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_Port, IN2_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(pwmTimer, pwmChannel, 0);

    return HAL_OK;
}

HAL_StatusTypeDef TB6612_DeInit(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    TB6612_Stop(handle);

    handle->IN1_Port = NULL;
    handle->IN1_Pin = 0;
    handle->IN2_Port = NULL;
    handle->IN2_Pin = 0;
    handle->pwmTimer = NULL;
    handle->pwmChannel = 0;

    return HAL_OK;
}

HAL_StatusTypeDef TB6612_Start(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return HAL_ERROR;
    }

    if (handle->isStarted) {
        return HAL_OK;
    }

    if (HAL_TIM_PWM_Start(handle->pwmTimer, handle->pwmChannel) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->isStarted = 1;
    return HAL_OK;
}

HAL_StatusTypeDef TB6612_Stop(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return HAL_ERROR;
    }

    if (!handle->isStarted) {
        return HAL_OK;
    }

    HAL_TIM_PWM_Stop(handle->pwmTimer, handle->pwmChannel);
    TB6612_SetDirection(handle, TB6612_DIR_STOP);

    handle->isStarted = 0;
    return HAL_OK;
}

HAL_StatusTypeDef TB6612_SetDirection(TB6612_HandleTypeDef* handle, TB6612_DirectionTypeDef direction)
{
    if (handle == NULL || handle->IN1_Port == NULL || handle->IN2_Port == NULL) {
        return HAL_ERROR;
    }

    switch (direction) {
        case TB6612_DIR_FORWARD:
            HAL_GPIO_WritePin(handle->IN1_Port, handle->IN1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(handle->IN2_Port, handle->IN2_Pin, GPIO_PIN_RESET);
            break;
        case TB6612_DIR_BACKWARD:
            HAL_GPIO_WritePin(handle->IN1_Port, handle->IN1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(handle->IN2_Port, handle->IN2_Pin, GPIO_PIN_SET);
            break;
        case TB6612_DIR_BRAKE:
            HAL_GPIO_WritePin(handle->IN1_Port, handle->IN1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(handle->IN2_Port, handle->IN2_Pin, GPIO_PIN_SET);
            break;
        case TB6612_DIR_STOP:
        default:
            HAL_GPIO_WritePin(handle->IN1_Port, handle->IN1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(handle->IN2_Port, handle->IN2_Pin, GPIO_PIN_RESET);
            break;
    }

    return HAL_OK;
}

HAL_StatusTypeDef TB6612_SetSpeed(TB6612_HandleTypeDef* handle, uint8_t speed)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return HAL_ERROR;
    }

    if (speed > 100) {
        speed = 100;
    }

    __HAL_TIM_SET_COMPARE(handle->pwmTimer, handle->pwmChannel, speed);
    return HAL_OK;
}

TB6612_DirectionTypeDef TB6612_GetDirection(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL || handle->IN1_Port == NULL || handle->IN2_Port == NULL) {
        return TB6612_DIR_STOP;
    }

    GPIO_PinState state1 = HAL_GPIO_ReadPin(handle->IN1_Port, handle->IN1_Pin);
    GPIO_PinState state2 = HAL_GPIO_ReadPin(handle->IN2_Port, handle->IN2_Pin);

    if (state1 == GPIO_PIN_SET && state2 == GPIO_PIN_RESET) {
        return TB6612_DIR_FORWARD;
    } else if (state1 == GPIO_PIN_RESET && state2 == GPIO_PIN_SET) {
        return TB6612_DIR_BACKWARD;
    } else if (state1 == GPIO_PIN_SET && state2 == GPIO_PIN_SET) {
        return TB6612_DIR_BRAKE;
    } else {
        return TB6612_DIR_STOP;
    }
}

uint8_t TB6612_GetSpeed(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return 0;
    }

    return (uint8_t)__HAL_TIM_GET_COMPARE(handle->pwmTimer, handle->pwmChannel);
}

uint8_t TB6612_IsRunning(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return handle->isStarted;
}
