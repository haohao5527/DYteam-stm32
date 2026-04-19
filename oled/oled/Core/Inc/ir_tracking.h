#ifndef __IR_TRACKING_H__
#define __IR_TRACKING_H__

#include "main.h"
#include "stm32f1xx_hal.h"

#define IR_TRACKING_DEBOUNCE_MS    20
#define IR_FILTER_SAMPLES          5

typedef enum {
    IR_TRACK_LINE_NONE = 0,
    IR_TRACK_LINE_DETECTED = 1
} IR_TrackStatusTypeDef;

typedef struct {
    GPIO_TypeDef* sensorPort;
    uint16_t sensorPin;
    IR_TrackStatusTypeDef status;
    IR_TrackStatusTypeDef lastStatus;
    uint32_t lastChangeTime;
    uint8_t isInitialized;
    uint8_t filterBuffer[IR_FILTER_SAMPLES]; // 滑动滤波缓冲区
    uint8_t filterIndex;                     // 缓冲索引
} IR_TrackSensor_HandleTypeDef;

typedef struct {
    IR_TrackSensor_HandleTypeDef leftSensor;
    IR_TrackSensor_HandleTypeDef middleSensor;
    IR_TrackSensor_HandleTypeDef rightSensor;
    uint8_t linePattern;
} IR_TrackSensor_ArrayHandleTypeDef;

HAL_StatusTypeDef IR_TrackSensor_Init(IR_TrackSensor_HandleTypeDef* handle,
                                       GPIO_TypeDef* port, uint16_t pin);

void IR_TrackSensor_Update(IR_TrackSensor_HandleTypeDef* handle);
IR_TrackStatusTypeDef IR_TrackSensor_GetStatus(IR_TrackSensor_HandleTypeDef* handle);
uint8_t IR_TrackSensor_IsLineDetected(IR_TrackSensor_HandleTypeDef* handle);

void IR_TrackSensor_ArrayInit(IR_TrackSensor_ArrayHandleTypeDef* handle,
                              GPIO_TypeDef* leftPort, uint16_t leftPin,
                              GPIO_TypeDef* middlePort, uint16_t middlePin,
                              GPIO_TypeDef* rightPort, uint16_t rightPin);

void IR_TrackSensor_ArrayUpdate(IR_TrackSensor_ArrayHandleTypeDef* handle);
uint8_t IR_TrackSensor_ArrayGetPattern(IR_TrackSensor_ArrayHandleTypeDef* handle);

#define IR_TRACK_PATTERN_LEFT      0x01
#define IR_TRACK_PATTERN_MIDDLE    0x02
#define IR_TRACK_PATTERN_RIGHT     0x04
#define IR_TRACK_PATTERN_ALL       0x07
#define IR_TRACK_PATTERN_NONE      0x00

#endif
