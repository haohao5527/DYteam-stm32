/**
 * SSD1306 OLED 驱动 - 简化版
 * I2C1: PB6=SCL, PB7=SDA
 */

#include "ssd1306.h"
#include <string.h>
#include <stdio.h>

// OLED 屏幕缓冲区 (128x64, 1 bit per pixel = 1024 bytes)
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// 发送命令
static void ssd1306_WriteCommand(I2C_HandleTypeDef *hi2c, uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(hi2c, SSD1306_I2C_ADDR, data, 2, 100);
}

// 发送数据
static void ssd1306_WriteData(I2C_HandleTypeDef *hi2c, uint8_t *data, uint16_t size)
{
    uint8_t buf[129];
    buf[0] = 0x40;  // Co=0, D/C#=1
    memcpy(&buf[1], data, size);
    HAL_I2C_Master_Transmit(hi2c, SSD1306_I2C_ADDR, buf, size + 1, 100);
}

// 初始化
void SSD1306_Init(I2C_HandleTypeDef *hi2c)
{
    // 关闭显示
    ssd1306_WriteCommand(hi2c, SSD1306_DISPLAY_OFF);
    
    // 设置时钟分频
    ssd1306_WriteCommand(hi2c, SSD1306_SET_CLOCK_DIV);
    ssd1306_WriteCommand(hi2c, 0x80);
    
    // 设置多路复用
    ssd1306_WriteCommand(hi2c, SSD1306_SET_MULTIPLEX);
    ssd1306_WriteCommand(hi2c, 0x3F);
    
    // 设置偏移
    ssd1306_WriteCommand(hi2c, SSD1306_SET_OFFSET);
    ssd1306_WriteCommand(hi2c, 0x00);
    
    // 设置起始行
    ssd1306_WriteCommand(hi2c, SSD1306_SET_START_LINE);
    
    // 设置段重映射
    ssd1306_WriteCommand(hi2c, SSD1306_SET_SEG_REMAP | 0x01);
    
    // 设置COM输出扫描方向
    ssd1306_WriteCommand(hi2c, SSD1306_SET_COM_SCAN);
    
    // 设置COM引脚配置
    ssd1306_WriteCommand(hi2c, SSD1306_SET_COM_CONFIG);
    ssd1306_WriteCommand(hi2c, 0x12);
    
    // 设置对比度
    ssd1306_WriteCommand(hi2c, SSD1306_SET_CONTRAST);
    ssd1306_WriteCommand(hi2c, 0xCF);
    
    // 设置预充电
    ssd1306_WriteCommand(hi2c, SSD1306_SET_PRECHARGE);
    ssd1306_WriteCommand(hi2c, 0xF1);
    
    // 设置VCOM检测
    ssd1306_WriteCommand(hi2c, SSD1306_SET_VCOM_DETECT);
    ssd1306_WriteCommand(hi2c, 0x40);
    
    // 设置显示开启
    ssd1306_WriteCommand(hi2c, SSD1306_DISPLAY_ON);
    
    // 清屏
    SSD1306_Clear(hi2c);
}

// 清屏
void SSD1306_Clear(I2C_HandleTypeDef *hi2c)
{
    memset(SSD1306_Buffer, 0, sizeof(SSD1306_Buffer));
    SSD1306_UpdateScreen(hi2c);
}

// 更新屏幕
void SSD1306_UpdateScreen(I2C_HandleTypeDef *hi2c)
{
    // 设置列地址
    ssd1306_WriteCommand(hi2c, 0x21);
    ssd1306_WriteCommand(hi2c, 0x00);
    ssd1306_WriteCommand(hi2c, 0x7F);
    
    // 设置页地址
    ssd1306_WriteCommand(hi2c, 0x22);
    ssd1306_WriteCommand(hi2c, 0x00);
    ssd1306_WriteCommand(hi2c, 0x07);
    
    // 发送数据
    ssd1306_WriteData(hi2c, SSD1306_Buffer, sizeof(SSD1306_Buffer));
}

// 画点
void SSD1306_DrawPixel(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    
    if (color)
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    else
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
}

// 5x7 字体数据 (简化ASCII)
const uint8_t Font5x7[] = {
    // 空格
    0x00, 0x00, 0x00, 0x00, 0x00,
    // !
    0x00, 0x00, 0x5F, 0x00, 0x00,
    // "
    0x00, 0x07, 0x00, 0x07, 0x00,
    // #
    0x14, 0x7E, 0x14, 0x7E, 0x14,
    // $
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    // %
    0x23, 0x13, 0x08, 0x64, 0x62,
    // &
    0x36, 0x49, 0x55, 0x22, 0x50,
    // '
    0x00, 0x05, 0x03, 0x00, 0x00,
    // (
    0x00, 0x1C, 0x22, 0x41, 0x00,
    // )
    0x00, 0x41, 0x22, 0x1C, 0x00,
    // *
    0x14, 0x08, 0x3E, 0x08, 0x14,
    // +
    0x08, 0x08, 0x3E, 0x08, 0x08,
    // ,
    0x00, 0xE0, 0x60, 0x00, 0x00,
    // -
    0x08, 0x08, 0x08, 0x08, 0x08,
    // .
    0x00, 0x60, 0x60, 0x00, 0x00,
    // /
    0x20, 0x10, 0x08, 0x04, 0x02,
    // 0
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    // 1
    0x00, 0x42, 0x7F, 0x40, 0x00,
    // 2
    0x42, 0x61, 0x51, 0x49, 0x46,
    // 3
    0x21, 0x41, 0x45, 0x4B, 0x31,
    // 4
    0x18, 0x14, 0x12, 0x7F, 0x10,
    // 5
    0x27, 0x45, 0x45, 0x45, 0x39,
    // 6
    0x3C, 0x4A, 0x49, 0x49, 0x30,
    // 7
    0x01, 0x71, 0x09, 0x05, 0x03,
    // 8
    0x36, 0x49, 0x49, 0x49, 0x36,
    // 9
    0x06, 0x49, 0x49, 0x29, 0x1E,
};

// 设置光标位置
void SSD1306_SetCursor(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y)
{
    ssd1306_WriteCommand(hi2c, 0x21);
    ssd1306_WriteCommand(hi2c, x);
    ssd1306_WriteCommand(hi2c, 0x7F);
    
    ssd1306_WriteCommand(hi2c, 0x22);
    ssd1306_WriteCommand(hi2c, y);
    ssd1306_WriteCommand(hi2c, 0x07);
}

// 写字符
void SSD1306_WriteChar(I2C_HandleTypeDef *hi2c, char c)
{
    if (c < ' ' || c > '9') c = ' ';
    
    uint8_t charIndex = c - ' ';
    uint8_t buf[6] = {0x40};
    
    // 复制字模数据
    for (int i = 0; i < 5; i++) {
        buf[i + 1] = Font5x7[charIndex * 5 + i];
    }
    buf[5] = 0x00;  // 列间距
    
    HAL_I2C_Master_Transmit(hi2c, SSD1306_I2C_ADDR, buf, 6, 100);
}

// 写字符串
void SSD1306_WriteString(I2C_HandleTypeDef *hi2c, char *str)
{
    while (*str) {
        SSD1306_WriteChar(hi2c, *str);
        str++;
    }
}