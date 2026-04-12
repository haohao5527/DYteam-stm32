#ifndef __SPEED_H__
#define __SPEED_H__

#include "stm32f1xx_hal.h"

// 测速盘参数
#define HOLES_PER_ROTATION 20    // 每圈20个孔

// 速度数据结构
typedef struct {
    uint32_t pulse_count;      // 脉冲计数
    uint32_t last_tick;        // 上次计数的系统时钟
    float rpm;                // 当前转速 (转/分钟)
    uint8_t is_new_rotation;   // 新圈标志
} SpeedData_t;

// 函数声明
void Speed_Init(void);
void Speed_CountPulse(void);
void Speed_Update(void);
float Speed_GetRPM(void);
uint32_t Speed_GetTotalPulses(void);

#endif