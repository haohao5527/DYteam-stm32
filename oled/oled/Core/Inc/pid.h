#ifndef __PID_H__
#define __PID_H__

#include "main.h"
#include "stm32f1xx_hal.h"

typedef struct {
    float kp;
    float ki;
    float kd;

    float target;
    float feedback;
    float output;

    float integral;
    float last_error;
    float integral_limit;
    float output_limit;

    uint8_t enabled;
} PID_HandleTypeDef;

void PID_Init(PID_HandleTypeDef* pid,
              float kp, float ki, float kd,
              float integral_limit, float output_limit);

float PID_Calc(PID_HandleTypeDef* pid, float target, float feedback);
void PID_Reset(PID_HandleTypeDef* pid);

void PID_SetTarget(PID_HandleTypeDef* pid, float target);
void PID_SetKp(PID_HandleTypeDef* pid, float kp);
void PID_SetKi(PID_HandleTypeDef* pid, float ki);
void PID_SetKd(PID_HandleTypeDef* pid, float kd);

void PID_Enable(PID_HandleTypeDef* pid);
void PID_Disable(PID_HandleTypeDef* pid);
uint8_t PID_IsEnabled(PID_HandleTypeDef* pid);

#endif

