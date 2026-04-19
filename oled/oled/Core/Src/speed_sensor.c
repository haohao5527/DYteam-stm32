#include "speed_sensor.h"

void SpeedSensor_Init(SpeedSensor_HandleTypeDef* hsensor, GPIO_TypeDef* Port, uint16_t Pin, uint8_t pulse_divider, uint8_t use_filter)
{
    hsensor->Port = Port;
    hsensor->Pin = Pin;
    hsensor->pulse_count = 0;
    hsensor->pulse_divider = pulse_divider;
    hsensor->raw_counter = 0;
    hsensor->last_pulse_count = 0;
    hsensor->speed_cm_s = 0.0f;
    hsensor->speed_buf_idx = 0;
    hsensor->use_filter = use_filter;
    for (uint8_t i = 0; i < 4; i++) {
        hsensor->speed_buffer[i] = 0.0f;
    }
}

void SpeedSensor_IncrementPulse(SpeedSensor_HandleTypeDef* hsensor)
{
    hsensor->raw_counter++;
    if (hsensor->raw_counter >= hsensor->pulse_divider) {
        hsensor->raw_counter = 0;
        hsensor->pulse_count++;
    }
}

uint32_t SpeedSensor_GetPulseCount(SpeedSensor_HandleTypeDef* hsensor)
{
    return hsensor->pulse_count;
}

void SpeedSensor_ResetPulseCount(SpeedSensor_HandleTypeDef* hsensor)
{
    hsensor->pulse_count = 0;
    hsensor->raw_counter = 0;
    hsensor->last_pulse_count = 0;
    hsensor->speed_cm_s = 0.0f;
    hsensor->speed_buf_idx = 0;
    for (uint8_t i = 0; i < 4; i++) {
        hsensor->speed_buffer[i] = 0.0f;
    }
}

void SpeedSensor_UpdateSpeed(SpeedSensor_HandleTypeDef* hsensor, float cm_per_pulse)
{
    uint32_t delta = hsensor->pulse_count - hsensor->last_pulse_count;
    float raw_speed = (float)delta * cm_per_pulse * 10.0f;
    hsensor->last_pulse_count = hsensor->pulse_count;

    if (hsensor->use_filter) {
        hsensor->speed_buffer[hsensor->speed_buf_idx] = raw_speed;
        hsensor->speed_buf_idx = (hsensor->speed_buf_idx + 1) & 0x03;

        float sum = 0.0f;
        for (uint8_t i = 0; i < 4; i++) {
            sum += hsensor->speed_buffer[i];
        }
        hsensor->speed_cm_s = sum / 4.0f;
    } else {
        hsensor->speed_cm_s = raw_speed;
    }
}
