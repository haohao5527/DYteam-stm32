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
    handle->speed = 0;
    handle->direction = TB6612_DIR_STOP;

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

    handle->direction = direction;
    return HAL_OK;
}

HAL_StatusTypeDef TB6612_SetSpeed(TB6612_HandleTypeDef* handle, int8_t speed)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return HAL_ERROR;
    }

    if (speed < -100) speed = -100;
    if (speed > 100) speed = 100;

    if (speed < 0) {
        TB6612_SetDirection(handle, TB6612_DIR_BACKWARD);
        speed = -speed;
    } else if (speed > 0) {
        TB6612_SetDirection(handle, TB6612_DIR_FORWARD);
    }

    __HAL_TIM_SET_COMPARE(handle->pwmTimer, handle->pwmChannel, (uint16_t)speed);
    handle->speed = speed;

    return HAL_OK;
}

HAL_StatusTypeDef TB6612_SetSpeedRaw(TB6612_HandleTypeDef* handle, uint16_t pulse)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return HAL_ERROR;
    }

    __HAL_TIM_SET_COMPARE(handle->pwmTimer, handle->pwmChannel, pulse);
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

int8_t TB6612_GetSpeed(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL || handle->pwmTimer == NULL) {
        return 0;
    }

    return handle->speed;
}

uint8_t TB6612_IsRunning(TB6612_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return handle->isStarted;
}

void TB6612_DualInit(TB6612_DualHandleTypeDef* handle,
                     GPIO_TypeDef* left_IN1_Port, uint16_t left_IN1_Pin,
                     GPIO_TypeDef* left_IN2_Port, uint16_t left_IN2_Pin,
                     TIM_HandleTypeDef* left_pwmTimer, uint32_t left_pwmChannel,
                     GPIO_TypeDef* right_IN1_Port, uint16_t right_IN1_Pin,
                     GPIO_TypeDef* right_IN2_Port, uint16_t right_IN2_Pin,
                     TIM_HandleTypeDef* right_pwmTimer, uint32_t right_pwmChannel)
{
    if (handle == NULL) {
        return;
    }

    TB6612_Init(&handle->leftMotor, left_IN1_Port, left_IN1_Pin,
                left_IN2_Port, left_IN2_Pin, left_pwmTimer, left_pwmChannel);
    TB6612_Init(&handle->rightMotor, right_IN1_Port, right_IN1_Pin,
                right_IN2_Port, right_IN2_Pin, right_pwmTimer, right_pwmChannel);
}

HAL_StatusTypeDef TB6612_DualStart(TB6612_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status1 = TB6612_Start(&handle->leftMotor);
    HAL_StatusTypeDef status2 = TB6612_Start(&handle->rightMotor);

    if (status1 != HAL_OK || status2 != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef TB6612_DualStop(TB6612_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    TB6612_Stop(&handle->leftMotor);
    TB6612_Stop(&handle->rightMotor);

    return HAL_OK;
}

void TB6612_DualSetSpeed(TB6612_DualHandleTypeDef* handle, int8_t leftSpeed, int8_t rightSpeed)
{
    if (handle == NULL) {
        return;
    }

    TB6612_SetSpeed(&handle->leftMotor, leftSpeed);
    TB6612_SetSpeed(&handle->rightMotor, rightSpeed);
}

void TB6612_DualSetDirection(TB6612_DualHandleTypeDef* handle,
                             TB6612_DirectionTypeDef leftDir, TB6612_DirectionTypeDef rightDir)
{
    if (handle == NULL) {
        return;
    }

    TB6612_SetDirection(&handle->leftMotor, leftDir);
    TB6612_SetDirection(&handle->rightMotor, rightDir);
}

void TB6612_DualMove(TB6612_DualHandleTypeDef* handle, int8_t speed)
{
    if (handle == NULL) {
        return;
    }

    TB6612_SetSpeed(&handle->leftMotor, speed);
    TB6612_SetSpeed(&handle->rightMotor, speed);
}
