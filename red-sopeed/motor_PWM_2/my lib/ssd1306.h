#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stm32f1xx_hal.h"

// I2C 地址 (SSD1306 默认地址 0x78)
#define SSD1306_I2C_ADDR 0x78

// SSD1306 命令
#define SSD1306_SET_CONTRAST 0x81
#define SSD1306_SET_LATCH_MODE 0x20
#define SSD1306_SET_PAGE_ADDR 0x22
#define SSD1306_SET_COLUMN_ADDR 0x21
#define SSD1306_SET_DISPLAY_ON 0xAF
#define SSD1306_SET_DISPLAY_OFF 0xAE
#define SSD1306_SET_NORMAL 0xA6
#define SSD1306_SET_INVERSE 0xA7
#define SSD1306_DISPLAY_OFF 0xAE
#define SSD1306_DISPLAY_ON 0xAF
#define SSD1306_SET_CLOCK_DIV 0xD5
#define SSD1306_SET_MULTIPLEX 0xA8
#define SSD1306_SET_OFFSET 0xD3
#define SSD1306_SET_START_LINE 0x40
#define SSD1306_SET_SEG_REMAP 0xA1
#define SSD1306_SET_COM_SCAN 0xC8
#define SSD1306_SET_COM_CONFIG 0xDA
#define SSD1306_SET_PRECHARGE 0xD9
#define SSD1306_SET_VCOM_DETECT 0xDB
#define SSD1306_SET_DCDC 0x8AD
#define SSD1306_CMD_NOP 0xE3
#define SSD1306_DEACTIVATE_SCROLL 0x2E

// 屏幕尺寸
#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64
#define SSD1306_PAGES 8

// 函数声明
void SSD1306_Init(I2C_HandleTypeDef *hi2c);
void SSD1306_Clear(I2C_HandleTypeDef *hi2c);
void SSD1306_UpdateScreen(I2C_HandleTypeDef *hi2c);
void SSD1306_SetCursor(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y);
void SSD1306_WriteChar(I2C_HandleTypeDef *hi2c, char c);
void SSD1306_WriteString(I2C_HandleTypeDef *hi2c, char *str);
void SSD1306_DrawPixel(I2C_HandleTypeDef *hi2c, uint8_t x, uint8_t y, uint8_t color);

// 字体宽度数据 (ASCII ' ' to '~')
extern const uint8_t Font5x7[];

#endif