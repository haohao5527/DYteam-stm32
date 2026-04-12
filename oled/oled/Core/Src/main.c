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
TB6612_HandleTypeDef hMotor;
EC11_HandleTypeDef hEncoder;
SpeedSensor_HandleTypeDef hSpeedSensor;

int16_t motor_speed = 50;
char speed_str[20];
char speed_str_line2[20];

/* 编码器调速参数 */
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
  * @brief  硬件初始化（电机、编码器、测速传感器、OLED）
  */
static void Hardware_Init(void)
{
    /* 先初始化OLED，避免I2C总线冲突 */
    HAL_Delay(100);
    OLED_Init();

    /* 再初始化电机 */
    TB6612_Init(&hMotor,
                GPIOB, GPIO_PIN_0,
                GPIOB, GPIO_PIN_1,
                &htim2, TIM_CHANNEL_1);

    TB6612_SetDirection(&hMotor, TB6612_DIR_FORWARD);
    TB6612_Start(&hMotor);
    TB6612_SetSpeed(&hMotor, 50);

    /* 初始化编码器 */
    EC11_Init(&hEncoder, &htim1);
    EC11_Start(&hEncoder);

    /* 初始化红外测速传感器 */
    SpeedSensor_Init(&hSpeedSensor, GPIOB, GPIO_PIN_10);
}

/**
  * @brief  应用主循环：编码器调速 + 测速 + OLED显示
  */
static void App_Loop(void)
{
    int16_t enc_delta = EC11_GetDelta(&hEncoder);
    int16_t speed_change;
    float speed_cm_s;

    if (enc_delta != 0) {
        speed_change = enc_delta * ENCODER_SPEED_STEP;
        motor_speed += speed_change;

        if (motor_speed < 0)   motor_speed = 0;
        if (motor_speed > 100) motor_speed = 100;

        TB6612_SetSpeed(&hMotor, (uint8_t)motor_speed);
    }

    SpeedSensor_Update(&hSpeedSensor);

    OLED_NewFrame();

    OLED_DrawImage(0, 0, &pikaImg, OLED_COLOR_NORMAL);

    sprintf(speed_str, "PWM:%3d%%", motor_speed);
    OLED_PrintASCIIString(0, 40, speed_str, &afont8x6, OLED_COLOR_NORMAL);

    speed_cm_s = SpeedSensor_GetFilteredSpeedCmS(&hSpeedSensor);
    sprintf(speed_str_line2, "Spd:%.1fcm/s", speed_cm_s);
    OLED_PrintASCIIString(0, 48, speed_str_line2, &afont8x6, OLED_COLOR_NORMAL);

    OLED_ShowFrame();

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
    /* USER CODE END 3 */
  }
  /* USER CODE END WHILE */
}

/* USER CODE BEGIN 4 */

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

/* USER CODE END 4 */

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
