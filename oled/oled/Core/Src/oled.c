#include "oled.h"
#include <math.h>
#include <stdlib.h>

#define OLED_DEFAULT_ADDRESS  0x78
#define OLED_PAGE             8
#define OLED_ROW              (8 * OLED_PAGE)
#define OLED_COLUMN           128

static void OLED_Send(OLED_HandleTypeDef* handle, uint8_t *data, uint8_t len)
{
    if (handle == NULL || handle->hi2c == NULL) return;
    HAL_I2C_Master_Transmit(handle->hi2c, handle->address, data, len, HAL_MAX_DELAY);
}

static void OLED_SendCmd(OLED_HandleTypeDef* handle, uint8_t cmd)
{
    static uint8_t sendBuffer[2] = {0};
    sendBuffer[1] = cmd;
    OLED_Send(handle, sendBuffer, 2);
}

HAL_StatusTypeDef OLED_Init(OLED_HandleTypeDef* handle, I2C_HandleTypeDef* hi2c)
{
    if (handle == NULL || hi2c == NULL) return HAL_ERROR;

    handle->hi2c = hi2c;
    handle->address = OLED_DEFAULT_ADDRESS;
    handle->isInitialized = 0;
    memset(handle->gram, 0, sizeof(handle->gram));

    OLED_SendCmd(handle, 0xAE);

    OLED_SendCmd(handle, 0x20);
    OLED_SendCmd(handle, 0x10);

    OLED_SendCmd(handle, 0xB0);

    OLED_SendCmd(handle, 0xC8);

    OLED_SendCmd(handle, 0x00);
    OLED_SendCmd(handle, 0x10);

    OLED_SendCmd(handle, 0x40);

    OLED_SendCmd(handle, 0x81);

    OLED_SendCmd(handle, 0xDF);
    OLED_SendCmd(handle, 0xA1);

    OLED_SendCmd(handle, 0xA6);
    OLED_SendCmd(handle, 0xA8);

    OLED_SendCmd(handle, 0x3F);

    OLED_SendCmd(handle, 0xA4);

    OLED_SendCmd(handle, 0xD3);
    OLED_SendCmd(handle, 0x00);

    OLED_SendCmd(handle, 0xD5);
    OLED_SendCmd(handle, 0xF0);

    OLED_SendCmd(handle, 0xD9);
    OLED_SendCmd(handle, 0x22);

    OLED_SendCmd(handle, 0xDA);
    OLED_SendCmd(handle, 0x12);

    OLED_SendCmd(handle, 0xDB);
    OLED_SendCmd(handle, 0x20);

    OLED_SendCmd(handle, 0x8D);
    OLED_SendCmd(handle, 0x14);

    OLED_NewFrame(handle);
    OLED_ShowFrame(handle);

    OLED_SendCmd(handle, 0xAF);

    handle->isInitialized = 1;
    return HAL_OK;
}

HAL_StatusTypeDef OLED_DeInit(OLED_HandleTypeDef* handle)
{
    if (handle == NULL) return HAL_ERROR;

    OLED_DisPlay_Off(handle);
    handle->hi2c = NULL;
    handle->address = 0;
    memset(handle->gram, 0, sizeof(handle->gram));
    handle->isInitialized = 0;

    return HAL_OK;
}

void OLED_DisPlay_On(OLED_HandleTypeDef* handle)
{
    if (handle == NULL) return;
    OLED_SendCmd(handle, 0x8D);
    OLED_SendCmd(handle, 0x14);
    OLED_SendCmd(handle, 0xAF);
}

void OLED_DisPlay_Off(OLED_HandleTypeDef* handle)
{
    if (handle == NULL) return;
    OLED_SendCmd(handle, 0x8D);
    OLED_SendCmd(handle, 0x10);
    OLED_SendCmd(handle, 0xAE);
}

void OLED_SetColorMode(OLED_HandleTypeDef* handle, OLED_ColorMode mode)
{
    if (handle == NULL) return;
    if (mode == OLED_COLOR_NORMAL)
        OLED_SendCmd(handle, 0xA6);
    else
        OLED_SendCmd(handle, 0xA7);
}

void OLED_NewFrame(OLED_HandleTypeDef* handle)
{
    if (handle == NULL) return;
    memset(handle->gram, 0, sizeof(handle->gram));
}

void OLED_ShowFrame(OLED_HandleTypeDef* handle)
{
    uint8_t i;
    static uint8_t sendBuffer[OLED_COLUMN + 1];

    if (handle == NULL) return;

    sendBuffer[0] = 0x40;
    for (i = 0; i < OLED_PAGE; i++)
    {
        OLED_SendCmd(handle, 0xB0 + i);
        OLED_SendCmd(handle, 0x00);
        OLED_SendCmd(handle, 0x10);
        memcpy(sendBuffer + 1, handle->gram[i], OLED_COLUMN);
        OLED_Send(handle, sendBuffer, OLED_COLUMN + 1);
    }
}

void OLED_SetPixel(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, OLED_ColorMode color)
{
    if (handle == NULL) return;
    if (x >= OLED_COLUMN || y >= OLED_ROW) return;
    if (!color)
        handle->gram[y / 8][x] |= 1 << (y % 8);
    else
        handle->gram[y / 8][x] &= ~(1 << (y % 8));
}

static void OLED_SetByte_Fine(OLED_HandleTypeDef* handle, uint8_t page, uint8_t column, uint8_t data, uint8_t start, uint8_t end, OLED_ColorMode color)
{
    uint8_t temp;
    if (handle == NULL) return;
    if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
    if (color) data = ~data;

    temp = data | (0xff << (end + 1)) | (0xff >> (8 - start));
    handle->gram[page][column] &= temp;
    temp = data & ~(0xff << (end + 1)) & ~(0xff >> (8 - start));
    handle->gram[page][column] |= temp;
}

static void OLED_SetByte(OLED_HandleTypeDef* handle, uint8_t page, uint8_t column, uint8_t data, OLED_ColorMode color)
{
    if (handle == NULL) return;
    if (page >= OLED_PAGE || column >= OLED_COLUMN) return;
    if (color) data = ~data;
    handle->gram[page][column] = data;
}

static void OLED_SetBits_Fine(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t data, uint8_t len, OLED_ColorMode color)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    if (handle == NULL) return;
    if (bit + len > 8)
    {
        OLED_SetByte_Fine(handle, page, x, data << bit, bit, 7, color);
        OLED_SetByte_Fine(handle, page + 1, x, data >> (8 - bit), 0, len + bit - 1 - 8, color);
    }
    else
    {
        OLED_SetByte_Fine(handle, page, x, data << bit, bit, bit + len - 1, color);
    }
}

static void OLED_SetBits(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t data, OLED_ColorMode color)
{
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    if (handle == NULL) return;
    OLED_SetByte_Fine(handle, page, x, data << bit, bit, 7, color);
    if (bit)
        OLED_SetByte_Fine(handle, page + 1, x, data >> (8 - bit), 0, bit - 1, color);
}

static void OLED_SetBlock(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, const uint8_t *data, uint8_t w, uint8_t h, OLED_ColorMode color)
{
    uint8_t i, j;
    uint8_t fullRow = h / 8;
    uint8_t partBit = h % 8;
    if (handle == NULL) return;
    for (i = 0; i < w; i++)
    {
        for (j = 0; j < fullRow; j++)
            OLED_SetBits(handle, x + i, y + j * 8, data[i + j * w], color);
    }
    if (partBit)
    {
        uint16_t fullNum = (uint16_t)w * fullRow;
        for (i = 0; i < w; i++)
            OLED_SetBits_Fine(handle, x + i, y + (fullRow * 8), data[fullNum + i], partBit, color);
    }
}

void OLED_DrawLine(OLED_HandleTypeDef* handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color)
{
    int16_t dx, dy, ux, uy, x, y, eps;
    uint8_t temp;

    if (handle == NULL) return;
    if (x1 == x2)
    {
        if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
        for (; y1 <= y2; y1++) OLED_SetPixel(handle, x1, y1, color);
    }
    else if (y1 == y2)
    {
        if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
        for (; x1 <= x2; x1++) OLED_SetPixel(handle, x1, y1, color);
    }
    else
    {
        dx = x2 - x1; dy = y2 - y1;
        ux = ((dx > 0) << 1) - 1;
        uy = ((dy > 0) << 1) - 1;
        x = x1; y = y1; eps = 0;
        dx = abs(dx); dy = abs(dy);
        if (dx > dy)
        {
            for (x = x1; x != x2; x += ux)
            {
                OLED_SetPixel(handle, x, y, color);
                eps += dy;
                if ((eps << 1) >= dx) { y += uy; eps -= dx; }
            }
        }
        else
        {
            for (y = y1; y != y2; y += uy)
            {
                OLED_SetPixel(handle, x, y, color);
                eps += dx;
                if ((eps << 1) >= dy) { x += ux; eps -= dy; }
            }
        }
    }
}

void OLED_DrawRectangle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color)
{
    if (handle == NULL) return;
    OLED_DrawLine(handle, x, y, x + w, y, color);
    OLED_DrawLine(handle, x, y + h, x + w, y + h, color);
    OLED_DrawLine(handle, x, y, x, y + h, color);
    OLED_DrawLine(handle, x + w, y, x + w, y + h, color);
}

void OLED_DrawFilledRectangle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color)
{
    uint8_t i;
    if (handle == NULL) return;
    for (i = 0; i < h; i++)
        OLED_DrawLine(handle, x, y + i, x + w, y + i, color);
}

void OLED_DrawTriangle(OLED_HandleTypeDef* handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color)
{
    if (handle == NULL) return;
    OLED_DrawLine(handle, x1, y1, x2, y2, color);
    OLED_DrawLine(handle, x2, y2, x3, y3, color);
    OLED_DrawLine(handle, x3, y3, x1, y1, color);
}

void OLED_DrawFilledTriangle(OLED_HandleTypeDef* handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3, OLED_ColorMode color)
{
    uint8_t a = 0, b = 0, y, last = 0;

    if (handle == NULL) return;
    if (y1 > y2) { a = y2; b = y1; } else { a = y1; b = y2; }

    for (y = a; y <= b; y++)
    {
        if (y <= y3)
            OLED_DrawLine(handle, x1 + (int16_t)(y - y1) * (int16_t)(x2 - x1) / (int16_t)(y2 - y1), y,
                         x1 + (int16_t)(y - y1) * (int16_t)(x3 - x1) / (int16_t)(y3 - y1), y, color);
        else
        { last = y - 1; break; }
    }
    for (; y <= b; y++)
        OLED_DrawLine(handle, x2 + (int16_t)(y - y2) * (int16_t)(x3 - x2) / (int16_t)(y3 - y2), y,
                     x1 + (int16_t)(y - last) * (int16_t)(x3 - x1) / (int16_t)(y3 - last), y, color);
}

void OLED_DrawCircle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color)
{
    int16_t a = 0, b = r, di = 3 - (r << 1);

    if (handle == NULL) return;
    while (a <= b)
    {
        OLED_SetPixel(handle, x - b, y - a, color);
        OLED_SetPixel(handle, x + b, y - a, color);
        OLED_SetPixel(handle, x - a, y + b, color);
        OLED_SetPixel(handle, x - b, y - a, color);
        OLED_SetPixel(handle, x - a, y - b, color);
        OLED_SetPixel(handle, x + b, y + a, color);
        OLED_SetPixel(handle, x + a, y - b, color);
        OLED_SetPixel(handle, x + a, y + b, color);
        OLED_SetPixel(handle, x - b, y + a, color);
        a++;
        if (di < 0) di += 4 * a + 6;
        else { di += 10 + 4 * (a - b); b--; }
        OLED_SetPixel(handle, x + a, y + b, color);
    }
}

void OLED_DrawFilledCircle(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color)
{
    int16_t a = 0, b = r, di = 3 - (r << 1), i;

    if (handle == NULL) return;
    while (a <= b)
    {
        for (i = x - b; i <= x + b; i++) { OLED_SetPixel(handle, i, y + a, color); OLED_SetPixel(handle, i, y - a, color); }
        for (i = x - a; i <= x + a; i++) { OLED_SetPixel(handle, i, y + b, color); OLED_SetPixel(handle, i, y - b, color); }
        a++;
        if (di < 0) di += 4 * a + 6;
        else { di += 10 + 4 * (a - b); b--; }
    }
}

void OLED_DrawEllipse(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, uint8_t ra, uint8_t rb, OLED_ColorMode color)
{
    int xpos = 0, ypos = rb;
    int a2 = ra * ra, b2 = rb * rb;
    int d = b2 + a2 * (0.25 - rb);

    if (handle == NULL) return;
    while ((uint32_t)a2 * ypos > (uint32_t)b2 * xpos)
    {
        OLED_SetPixel(handle, x + xpos, y + ypos, color);
        OLED_SetPixel(handle, x - xpos, y + ypos, color);
        OLED_SetPixel(handle, x + xpos, y - ypos, color);
        OLED_SetPixel(handle, x - xpos, y - ypos, color);
        if (d < 0) { d += b2 * ((xpos << 1) + 3); xpos++; }
        else { d += b2 * ((xpos << 1) + 3) + a2 * (-(ypos << 1) + 2); xpos++; ypos--; }
    }

    d = b2 * (xpos + 0.5) * (xpos + 0.5) + a2 * (ypos - 1) * (ypos - 1) - a2 * b2;
    while (ypos > 0)
    {
        OLED_SetPixel(handle, x + xpos, y + ypos, color);
        OLED_SetPixel(handle, x - xpos, y + ypos, color);
        OLED_SetPixel(handle, x + xpos, y - ypos, color);
        OLED_SetPixel(handle, x - xpos, y - ypos, color);
        if (d < 0) { d += b2 * ((xpos << 1) + 2) + a2 * (-(ypos << 1) + 3); xpos++; ypos--; }
        else { d += a2 * (-(ypos << 1) + 3); ypos--; }
    }
}

void OLED_DrawImage(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color)
{
    if (handle == NULL || img == NULL) return;
    OLED_SetBlock(handle, x, y, img->data, img->w, img->h, color);
}

void OLED_PrintASCIIChar(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color)
{
    if (handle == NULL || font == NULL) return;
    OLED_SetBlock(handle, x, y, font->chars + (ch - ' ') * (((font->h + 7) / 8) * font->w), font->w, font->h, color);
}

void OLED_PrintASCIIString(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color)
{
    uint8_t x0 = x;
    if (handle == NULL || str == NULL || font == NULL) return;
    while (*str)
    {
        OLED_PrintASCIIChar(handle, x0, y, *str, font, color);
        x0 += font->w;
        str++;
    }
}

static uint8_t _OLED_GetUTF8Len(char *string)
{
    if ((string[0] & 0x80) == 0x00) return 1;
    else if ((string[0] & 0xE0) == 0xC0) return 2;
    else if ((string[0] & 0xF0) == 0xE0) return 3;
    else if ((string[0] & 0xF8) == 0xF0) return 4;
    return 0;
}

void OLED_PrintString(OLED_HandleTypeDef* handle, uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color)
{
    uint16_t i = 0;
    uint8_t oneLen, found, utf8Len;
    uint8_t *head;

    if (handle == NULL || str == NULL || font == NULL) return;
    oneLen = (((font->h + 7) / 8) * font->w) + 4;

    while (str[i])
    {
        found = 0;
        utf8Len = _OLED_GetUTF8Len(str + i);
        if (utf8Len == 0) break;

        for (uint8_t j = 0; j < font->len; j++)
        {
            head = (uint8_t *)(font->chars) + (j * oneLen);
            if (memcmp(str + i, head, utf8Len) == 0)
            {
                OLED_SetBlock(handle, x, y, head + 4, font->w, font->h, color);
                x += font->w;
                i += utf8Len;
                found = 1;
                break;
            }
        }

        if (found == 0)
        {
            if (utf8Len == 1)
            { OLED_PrintASCIIChar(handle, x, y, str[i], font->ascii, color); x += font->ascii->w; i += utf8Len; }
            else
            { OLED_PrintASCIIChar(handle, x, y, ' ', font->ascii, color); x += font->ascii->w; i += utf8Len; }
        }
    }
}
