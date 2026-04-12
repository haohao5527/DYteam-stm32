/**
 * 电机测速模块
 * 使用红外对射传感器检测测速盘孔数
 * 轮胎直径: 6.5 cm
 * 检测时间窗口: 0.6秒
 */

#include "speed.h"
#include "ssd1306.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;
extern uint8_t speed;

// 轮胎参数
#define WHEEL_DIAMETER_CM 6.5f
#define WHEEL_CIRCUMFERENCE_M (3.14159f * WHEEL_DIAMETER_CM / 100.0f)  // 米
#define TIME_WINDOW_MS 600      // 检测时间窗口: 0.6秒

// 速度数据
static SpeedData_t speed_data = {0, 0, 0, 0};
static uint8_t last_sensor_state = 0;
static uint32_t sensor_start_tick = 0;
static uint32_t pulse_count_window = 0;

// 初始化
void Speed_Init(void)
{
    speed_data.pulse_count = 0;
    speed_data.last_tick = 0;
    speed_data.rpm = 0;
    speed_data.is_new_rotation = 0;
    last_sensor_state = 1;
    sensor_start_tick = HAL_GetTick();
    pulse_count_window = 0;
}

// 脉冲检测 - 在主循环中调用
void Speed_CountPulse(void)
{
    uint8_t current_state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14);
    
    // 下降沿检测 (从1变0 = 对射传感器被遮挡)
    if (last_sensor_state == 1 && current_state == 0) {
        uint32_t current_tick = HAL_GetTick();
        
        // 0.6秒时间窗口检测
        if (current_tick - sensor_start_tick >= TIME_WINDOW_MS) {
            // 计算转速 (RPM)
            // 脉冲数 / 时间(秒) * (60秒/分钟) / 20脉冲每圈 = RPM
            float time_sec = (current_tick - sensor_start_tick) / 1000.0f;
            if (time_sec > 0) {
                speed_data.rpm = (pulse_count_window / time_sec) * 60.0f / HOLES_PER_ROTATION;
            }
            
            // 重置计时器
            sensor_start_tick = current_tick;
            pulse_count_window = 0;
        }
        
        pulse_count_window++;
        speed_data.pulse_count++;
    }
    
    last_sensor_state = current_state;
}

// 更新转速 (在主循环中调用)
void Speed_Update(void)
{
    uint32_t current_tick = HAL_GetTick();
    
    // 如果超过2秒没有新脉冲，重置速度
    if (current_tick - sensor_start_tick > 2000 && pulse_count_window == 0) {
        speed_data.rpm = 0;
    }
    
    // 计算转速 (RPM) - 在0.6秒窗口内
    if (pulse_count_window > 0) {
        uint32_t elapsed = current_tick - sensor_start_tick;
        if (elapsed >= TIME_WINDOW_MS) {
            float time_sec = elapsed / 1000.0f;
            if (time_sec > 0) {
                speed_data.rpm = (pulse_count_window / time_sec) * 60.0f / HOLES_PER_ROTATION;
            }
        } else {
            // 不足0.6秒，估算当前转速
            float time_sec = elapsed / 1000.0f;
            if (time_sec > 0.01f) {
                speed_data.rpm = (pulse_count_window / time_sec) * 60.0f / HOLES_PER_ROTATION;
            }
        }
    }
    
    // 计算线速度 (km/h)
    // v = RPM / 60 * 轮胎周长(米) * 3.6 km/h
    float speed_kmh = (speed_data.rpm / 60.0f) * WHEEL_CIRCUMFERENCE_M * 3.6f;
    
    // 显示到OLED
    char buf[32];
    
    SSD1306_SetCursor(&hi2c1, 0, 0);
    SSD1306_WriteString(&hi2c1, "RPM:");
    if (speed_data.rpm > 0) {
        sprintf(buf, "%d", (int)speed_data.rpm);
    } else {
        sprintf(buf, "0");
    }
    SSD1306_WriteString(&hi2c1, buf);
    
    SSD1306_SetCursor(&hi2c1, 0, 2);
    SSD1306_WriteString(&hi2c1, "km/h:");
    if (speed_kmh > 0) {
        // 显示一位小数
        sprintf(buf, "%.1f", (double)speed_kmh);
    } else {
        sprintf(buf, "0.0");
    }
    SSD1306_WriteString(&hi2c1, buf);
    
    SSD1306_SetCursor(&hi2c1, 0, 4);
    sprintf(buf, "PWM:%d    ", speed);
    SSD1306_WriteString(&hi2c1, buf);
    
    // 更新屏幕
    SSD1306_UpdateScreen(&hi2c1);
}

// 获取当前转速
float Speed_GetRPM(void)
{
    return speed_data.rpm;
}

// 获取总脉冲数
uint32_t Speed_GetTotalPulses(void)
{
    return speed_data.pulse_count;
}