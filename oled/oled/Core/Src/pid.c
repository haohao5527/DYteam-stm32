#include "pid.h"

void PID_Init(PID_HandleTypeDef* pid,
              float kp, float ki, float kd,
              float integral_limit, float output_limit)
{
    if (pid == NULL) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;

    pid->target = 0.0f;
    pid->feedback = 0.0f;
    pid->output = 0.0f;

    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;

    pid->enabled = 1;
}

float PID_Calc(PID_HandleTypeDef* pid, float target, float feedback)
{
    if (pid == NULL || !pid->enabled) {
        return 0.0f;
    }

    float error, p_out, i_out, d_out;

    error = target - feedback;
    pid->feedback = feedback;
    pid->target = target;

    p_out = pid->kp * error;

    pid->integral += error;
    if (pid->integral > pid->integral_limit) {
        pid->integral = pid->integral_limit;
    } else if (pid->integral < -pid->integral_limit) {
        pid->integral = -pid->integral_limit;
    }
    i_out = pid->ki * pid->integral;

    d_out = pid->kd * (error - pid->last_error);
    pid->last_error = error;

    pid->output = p_out + i_out + d_out;

    if (pid->output > pid->output_limit) {
        pid->output = pid->output_limit;
    } else if (pid->output < -pid->output_limit) {
        pid->output = -pid->output_limit;
    }

    return pid->output;
}

void PID_Reset(PID_HandleTypeDef* pid)
{
    if (pid == NULL) {
        return;
    }

    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output = 0.0f;
}

void PID_SetTarget(PID_HandleTypeDef* pid, float target)
{
    if (pid == NULL) {
        return;
    }

    pid->target = target;
}

void PID_SetKp(PID_HandleTypeDef* pid, float kp)
{
    if (pid == NULL) {
        return;
    }

    pid->kp = kp;
}

void PID_SetKi(PID_HandleTypeDef* pid, float ki)
{
    if (pid == NULL) {
        return;
    }

    pid->ki = ki;
}

void PID_SetKd(PID_HandleTypeDef* pid, float kd)
{
    if (pid == NULL) {
        return;
    }

    pid->kd = kd;
}

void PID_Enable(PID_HandleTypeDef* pid)
{
    if (pid == NULL) {
        return;
    }

    pid->enabled = 1;
}

void PID_Disable(PID_HandleTypeDef* pid)
{
    if (pid == NULL) {
        return;
    }

    pid->enabled = 0;
}

uint8_t PID_IsEnabled(PID_HandleTypeDef* pid)
{
    if (pid == NULL) {
        return 0;
    }

    return pid->enabled;
}

