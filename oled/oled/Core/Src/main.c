/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "speed_sensor.h"
#include "tb6612.h"
#include "ec11.h"
#include "oled.h"
#include "vl53l0x.h"
#include "ir_tracking.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TB6612_DualHandleTypeDef hMotors;
EC11_HandleTypeDef hEncoder;
SpeedSensor_DualHandleTypeDef hSpeedSensors;
VL53L0X_HandleTypeDef hLaser;
OLED_HandleTypeDef hOLED;
IR_TrackSensor_ArrayHandleTypeDef hTrackSensor;

int16_t left_speed = 50;
int16_t right_speed = 50;

char speed_str[20] = "L:50% R:50%";
char speed_str_line2[20] = "Spd:0.0cm/s";
char dist_str[20] = "Dist:INIT";

#define ENCODER_SPEED_STEP    3
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Hardware_Init(void);
static void App_Loop(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  硬件初始化（电机、编码器、测速传感器、OLED等）
  */
static void Hardware_Init(void)
{
    HAL_Delay(100);
    OLED_Init(&hOLED, &hi2c1);

    TB6612_DualInit(&hMotors,
                    GPIOB, GPIO_PIN_0,   GPIOB, GPIO_PIN_1,   &htim2, TIM_CHANNEL_2,
                    GPIOB, GPIO_PIN_2,   GPIOB, GPIO_PIN_3,   &htim2, TIM_CHANNEL_3);

    TB6612_DualStart(&hMotors);
    TB6612_DualSetSpeed(&hMotors, 50, 50);

    EC11_Init(&hEncoder, &htim1);
    EC11_Start(&hEncoder);

    SpeedSensor_DualInit(&hSpeedSensors,
                         GPIOA, GPIO_PIN_0, EXTI0_IRQn,
                         GPIOB, GPIO_PIN_10, EXTI15_10_IRQn);

    HAL_TIM_Base_Start(&htim4);

    if (VL53L0X_Init(&hLaser, &hi2c1, GPIOB, GPIO_PIN_11) != HAL_OK) {
        sprintf(dist_str, "Dist:FAIL");
    }

    IR_TrackSensor_ArrayInit(&hTrackSensor,
                              GPIOB, GPIO_PIN_14,
                              GPIOB, GPIO_PIN_12,
                              GPIOB, GPIO_PIN_13);
}

/**
  * @brief  应用主循环：编码器调速 + 测速 + OLED显示
  */
static void App_Loop(void)
{
    int16_t enc_delta = EC11_GetDelta(&hEncoder);
    int16_t speed_change;
    float left_speed_cm_s;
    float right_speed_cm_s;

    if (enc_delta != 0) {
        speed_change = enc_delta * ENCODER_SPEED_STEP;
        left_speed += speed_change;
        right_speed += speed_change;

        if (left_speed < 0)  left_speed = 0;
        if (left_speed > 100) left_speed = 100;
        if (right_speed < 0)  right_speed = 0;
        if (right_speed > 100) right_speed = 100;

        TB6612_DualSetSpeed(&hMotors, (int8_t)left_speed, (int8_t)right_speed);
    }

    SpeedSensor_DualUpdate(&hSpeedSensors);

    static uint16_t last_dist = 0;
    uint16_t current_dist;

    if (VL53L0X_ReadDistance(&hLaser) == HAL_OK) {
        if (VL53L0X_IsValid(&hLaser)) {
            current_dist = VL53L0X_GetDistance(&hLaser);
            last_dist = current_dist;
            sprintf(dist_str, "Dist:%4d", current_dist);
        } else {
            if (last_dist != 0) {
                sprintf(dist_str, "Dist:%4d*", last_dist);
            }
        }
    }

    IR_TrackSensor_ArrayUpdate(&hTrackSensor);
    uint8_t track_pattern = IR_TrackSensor_ArrayGetPattern(&hTrackSensor);

    OLED_NewFrame(&hOLED);

    OLED_PrintASCIIString(&hOLED, 0, 0, dist_str, &afont8x6, OLED_COLOR_NORMAL);

    OLED_DrawLine(&hOLED, 63, 8, 63, 63, OLED_COLOR_NORMAL);

    sprintf(speed_str, "L:%3d%%", left_speed);
    OLED_PrintASCIIString(&hOLED, 0, 16, speed_str, &afont8x6, OLED_COLOR_NORMAL);

    sprintf(speed_str, "R:%3d%%", right_speed);
    OLED_PrintASCIIString(&hOLED, 70, 16, speed_str, &afont8x6, OLED_COLOR_NORMAL);

    left_speed_cm_s = SpeedSensor_DualGetFilteredLeftSpeedCmS(&hSpeedSensors);
    right_speed_cm_s = SpeedSensor_DualGetFilteredRightSpeedCmS(&hSpeedSensors);

    sprintf(speed_str, "%.1fcm/s", left_speed_cm_s);
    OLED_PrintASCIIString(&hOLED, 0, 26, speed_str, &afont8x6, OLED_COLOR_NORMAL);

    sprintf(speed_str, "%.1fcm/s", right_speed_cm_s);
    OLED_PrintASCIIString(&hOLED, 70, 26, speed_str, &afont8x6, OLED_COLOR_NORMAL);

    OLED_ShowFrame(&hOLED);

    HAL_Delay(50);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  Hardware_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    App_Loop();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
