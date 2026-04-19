#include "ec11.h"

HAL_StatusTypeDef EC11_Init(EC11_HandleTypeDef* handle, TIM_HandleTypeDef* htim)
{
    if (handle == NULL || htim == NULL) {
        return HAL_ERROR;
    }

    handle->htim = htim;
    handle->lastCount = 0;
    handle->isStarted = 0;
    handle->buttonPort = NULL;
    handle->buttonPin = 0;
    handle->buttonPressed = 0;
    handle->buttonLastState = 1;
    handle->debounceCounter = 0;

    __HAL_TIM_SET_COUNTER(htim, 0);

    return HAL_OK;
}

HAL_StatusTypeDef EC11_InitWithButton(EC11_HandleTypeDef* handle, TIM_HandleTypeDef* htim,
                                       GPIO_TypeDef* buttonPort, uint16_t buttonPin)
{
    if (handle == NULL || htim == NULL) {
        return HAL_ERROR;
    }

    handle->htim = htim;
    handle->lastCount = 0;
    handle->isStarted = 0;
    handle->buttonPort = buttonPort;
    handle->buttonPin = buttonPin;
    handle->buttonPressed = 0;
    handle->buttonLastState = 1;
    handle->debounceCounter = 0;

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

    // TI12模式下，每格有4个脉冲，除以4更自然
    return delta / 4;
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

uint8_t EC11_GetButton(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->buttonPort == NULL) {
        return 0;
    }

    return HAL_GPIO_ReadPin(handle->buttonPort, handle->buttonPin) == GPIO_PIN_RESET;
}

uint8_t EC11_IsButtonPressed(EC11_HandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return handle->buttonPressed;
}

void EC11_UpdateButton(EC11_HandleTypeDef* handle)
{
    if (handle == NULL || handle->buttonPort == NULL) {
        return;
    }

    uint8_t currentState = HAL_GPIO_ReadPin(handle->buttonPort, handle->buttonPin) == GPIO_PIN_RESET;

    if (currentState != handle->buttonLastState) {
        handle->debounceCounter++;
        if (handle->debounceCounter >= EC11_DEBOUNCE_TICKS) {
            handle->buttonLastState = currentState;
            handle->debounceCounter = 0;
            if (currentState == 1) {
                handle->buttonPressed = 1;
            }
        }
    } else {
        handle->debounceCounter = 0;
    }
}

void EC11_DualInit(EC11_DualHandleTypeDef* handle,
                  TIM_HandleTypeDef* left_htim, GPIO_TypeDef* left_buttonPort, uint16_t left_buttonPin,
                  TIM_HandleTypeDef* right_htim, GPIO_TypeDef* right_buttonPort, uint16_t right_buttonPin)
{
    if (handle == NULL) {
        return;
    }

    EC11_InitWithButton(&handle->leftEncoder, left_htim, left_buttonPort, left_buttonPin);
    EC11_InitWithButton(&handle->rightEncoder, right_htim, right_buttonPort, right_buttonPin);
}

HAL_StatusTypeDef EC11_DualStart(EC11_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status1 = EC11_Start(&handle->leftEncoder);
    HAL_StatusTypeDef status2 = EC11_Start(&handle->rightEncoder);

    if (status1 != HAL_OK || status2 != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef EC11_DualStop(EC11_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return HAL_ERROR;
    }

    EC11_Stop(&handle->leftEncoder);
    EC11_Stop(&handle->rightEncoder);

    return HAL_OK;
}

int16_t EC11_DualGetDelta(EC11_DualHandleTypeDef* handle, uint8_t encoderIndex)
{
    if (handle == NULL) {
        return 0;
    }

    if (encoderIndex == 0) {
        return EC11_GetDelta(&handle->leftEncoder);
    } else {
        return EC11_GetDelta(&handle->rightEncoder);
    }
}

int16_t EC11_DualGetLeftDelta(EC11_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return EC11_GetDelta(&handle->leftEncoder);
}

int16_t EC11_DualGetRightDelta(EC11_DualHandleTypeDef* handle)
{
    if (handle == NULL) {
        return 0;
    }

    return EC11_GetDelta(&handle->rightEncoder);
}
