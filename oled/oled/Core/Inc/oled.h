#ifndef __OLED_H__
#define __OLED_H__

#include "font.h"
#include "main.h"
#include "string.h"

typedef enum {
  OLED_COLOR_NORMAL = 0,
  OLED_COLOR_REVERSED
} OLED_ColorMode;

typedef struct {
    I2C_HandleTypeDef* hi2c;
    uint16_t address;
    uint8_t gram[8][128];
    uint8_t isInitialized;
} OLED_HandleTypeDef;

HAL_StatusTypeDef OLED_Init(OLED_HandleTypeDef* handle, I2C_HandleTypeDef* hi2c);
HAL_StatusTypeDef OLED_DeInit(OLED_HandleTypeDef* handle);

void OLED_DisPlay_On(OLED_HandleTypeDef* handle);
void OLED_DisPlay_Off(OLED_HandleTypeDef* handle);
void OLED_SetColorMode(OLED_HandleTypeDef* handle, OLED_ColorMode mode);

void OLED_NewFrame(OLED_HandleTypeDef* handle);
void OLED_ShowFrame(OLED_HandleTypeDef* handle);
void OLED_SetPixel(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, OLED_ColorMode color);

void OLED_DrawLine(OLED_HandleTypeDef* handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color);
void OLED_DrawRectangle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);
void OLED_DrawFilledRectangle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);
void OLED_DrawTriangle(OLED_HandleTypeDef* handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);
void OLED_DrawFilledTriangle(OLED_HandleTypeDef* handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color);
void OLED_DrawCircle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
void OLED_DrawFilledCircle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
void OLED_DrawEllipse(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color);
void OLED_DrawImage(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color);

void OLED_PrintASCIIChar(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color);
void OLED_PrintASCIIString(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color);
void OLED_PrintString(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color);

#endif
