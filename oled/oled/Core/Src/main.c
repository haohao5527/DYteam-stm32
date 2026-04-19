/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - Smart Car Line Tracking & Following
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software component, it is provided AS-IS.
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
#include "tb6612.h"
#include "ec11.h"
#include "oled.h"
#include "vl53l0x.h"
#include "ir_tracking.h"
#include "pid.h"
#include "speed_sensor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
    MODE_STOP = 0,
    MODE_LINE_TRACK_ONLY,
    MODE_DISTANCE_FOLLOW_ONLY,
    MODE_LINE_TRACK_DISTANCE_FOLLOW
} CarMode_TypeDef;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ENCODER_SPEED_STEP        3
#define TARGET_SPEED_MIN           0
#define TARGET_SPEED_MAX           100

#define LINE_TRACK_KP             8.0f
#define LINE_TRACK_KI             0.0f
#define LINE_TRACK_KD             20.0f
#define LINE_TRACK_INTEGRAL_LIMIT 50.0f
#define LINE_TRACK_OUTPUT_LIMIT   40.0f

#define DISTANCE_TARGET_CM        20.0f
#define DISTANCE_KP               4.0f   // 增大KP，响应更快
#define DISTANCE_KI               0.05f  // 稍微增大KI
#define DISTANCE_KD               1.5f   // 增大KD，减少震荡
#define DISTANCE_INTEGRAL_LIMIT   30.0f
#define DISTANCE_OUTPUT_LIMIT     50.0f  // 增大输出限幅

#define DISPLAY_UPDATE_INTERVAL_MS 100

#define WHEEL_DIAMETER_CM           6.5f
#define WHEEL_CIRCUMFERENCE_CM      (3.14159f * WHEEL_DIAMETER_CM)
#define HOLES_PER_DISK              20
#define CM_PER_PULSE                (WHEEL_CIRCUMFERENCE_CM / (float)HOLES_PER_DISK)

#define SPEED_PID_DEADBAND_PERCENT  3.0f
#define SPEED_PID_INTEGRAL_LIMIT    30.0f
#define SPEED_PID_OUTPUT_LIMIT       40.0f
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TB6612_DualHandleTypeDef hMotors;
EC11_HandleTypeDef hEncoder;
SpeedSensor_HandleTypeDef hSpeedSensorLeft;
SpeedSensor_HandleTypeDef hSpeedSensorRight;
VL53L0X_HandleTypeDef hLaser;
OLED_HandleTypeDef hOLED;
IR_TrackSensor_ArrayHandleTypeDef hTrackSensor;

PID_HandleTypeDef pidLineTrack;
PID_HandleTypeDef pidDistance;

volatile int16_t target_speed = 50;
volatile int8_t left_motor_speed = 0;
volatile int8_t right_motor_speed = 0;

volatile uint8_t laser_valid = 0;
volatile uint16_t laser_distance_mm = 0;

volatile uint8_t track_pattern = 0;
volatile float line_track_correction = 0.0f;
volatile float distance_correction = 0.0f;
volatile float speed_diff_correction = 0.0f;

#define SPEED_DIFF_LIMIT   5.0f
#define SPEED_DIFF_KP      2.0f

#define RIGHT_MOTOR_COMPENSATION  10  // 静�?�补偿基准�??
#define SPEED_COMPENSATION_SCALE  0.3f  // 速度补偿系数

char dist_str[20] = "Dist:INIT";
char speed_str[24] = "L:  0% R:  0%";
char left_spd_str[16] = "Lft:0.0 cm/s  ";
char right_spd_str[16] = "Rgt:0.0 cm/s  ";
char track_str[12] = "Trk:000";
char mode_str[10] = "MODE:NORM";
char target_str[12] = "T:  0";

volatile CarMode_TypeDef current_mode = MODE_STOP;
volatile CarMode_TypeDef target_mode = MODE_STOP;
volatile uint32_t mode_transition_start = 0;
volatile int8_t transition_target_speed = 0;
volatile int8_t transition_left_speed = 0;
volatile int8_t transition_right_speed = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
void NVIC_Init_Priorities(void);

static void Hardware_Init(void);
static void App_Loop(void);

void Control_Task_10ms(void);
void Motor_Apply_Speed(int8_t left, int8_t right);
void Encoder_Update_TargetSpeed(void);
void Laser_Update_Distance(void);
void LineTracking_Correct(int8_t* left_speed, int8_t* right_speed);
void Distance_PID_Compute(void);
float Speed_Diff_Correct(float left_spd, float right_spd);
void Mode_Switch(void);
void Mode_Transition_Process(void);
void App_Process_Button(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief Hardware initialization
  */
static void Hardware_Init(void)
{
    HAL_Delay(100);

    OLED_Init(&hOLED, &hi2c1);

    TB6612_DualInit(&hMotors,
                    GPIOB, GPIO_PIN_0,   GPIOB, GPIO_PIN_1,   &htim2, TIM_CHANNEL_2,
                    GPIOB, GPIO_PIN_2,   GPIOB, GPIO_PIN_3,   &htim2, TIM_CHANNEL_1);
    TB6612_DualStart(&hMotors);

    EC11_Init(&hEncoder, &htim1);
    EC11_Start(&hEncoder);

    SpeedSensor_Init(&hSpeedSensorLeft, GPIOA, GPIO_PIN_6, 1, 1);
    SpeedSensor_Init(&hSpeedSensorRight, GPIOA, GPIO_PIN_5, 1, 1);

    IR_TrackSensor_ArrayInit(&hTrackSensor,
                              GPIOB, GPIO_PIN_14,
                              GPIOB, GPIO_PIN_12,
                              GPIOB, GPIO_PIN_13);

    PID_Init(&pidLineTrack,
             LINE_TRACK_KP, LINE_TRACK_KI, LINE_TRACK_KD,
             LINE_TRACK_INTEGRAL_LIMIT, LINE_TRACK_OUTPUT_LIMIT);

    PID_Init(&pidDistance,
             DISTANCE_KP, DISTANCE_KI, DISTANCE_KD,
             DISTANCE_INTEGRAL_LIMIT, DISTANCE_OUTPUT_LIMIT);

    if (VL53L0X_Init(&hLaser, &hi2c1, GPIOB, GPIO_PIN_11) != HAL_OK) {
        sprintf(dist_str, "Dist:FAIL");
    }

    NVIC_Init_Priorities();
    HAL_TIM_Base_Start_IT(&htim4);

    target_speed = 50;
    left_motor_speed = 50;
    right_motor_speed = 50;
    TB6612_DualSetSpeed(&hMotors, 50, 50);

    HAL_Delay(1);
}

/**
  * @brief Main application loop - Display update only (runs at ~20Hz)
  * All real-time control runs in TIM4 10ms interrupt
  */
static void App_Loop(void)
{
    static uint32_t last_display_time = 0;
    static uint32_t last_laser_read_time = 0;
    uint32_t now = HAL_GetTick();

    Encoder_Update_TargetSpeed();
    App_Process_Button();

    IR_TrackSensor_ArrayUpdate(&hTrackSensor);
    track_pattern = IR_TrackSensor_ArrayGetPattern(&hTrackSensor);

    // 只在需要跟车的时候读，而且每100ms读一次就够
    if ((current_mode == MODE_DISTANCE_FOLLOW_ONLY || current_mode == MODE_LINE_TRACK_DISTANCE_FOLLOW) && 
        (now - last_laser_read_time >= 100))
    {
        Laser_Update_Distance();
        last_laser_read_time = now;
    }

    if (now - last_display_time >= DISPLAY_UPDATE_INTERVAL_MS) {
        last_display_time = now;

        OLED_NewFrame(&hOLED);

        sprintf(dist_str, "Dist:%5dmm", laser_distance_mm);
        OLED_PrintASCIIString(&hOLED, 0, 0, dist_str, &afont8x6, OLED_COLOR_NORMAL);

        sprintf(speed_str, "L:%3d%% R:%3d%%", left_motor_speed, right_motor_speed);
        OLED_PrintASCIIString(&hOLED, 0, 10, speed_str, &afont8x6, OLED_COLOR_NORMAL);

        sprintf(speed_str, "L:%4.1f R:%4.1f", hSpeedSensorLeft.speed_cm_s, hSpeedSensorRight.speed_cm_s);
        OLED_PrintASCIIString(&hOLED, 0, 22, speed_str, &afont8x6, OLED_COLOR_NORMAL);

        if (current_mode == MODE_STOP) {
            sprintf(mode_str, "STOP  ");
        } else if (current_mode == MODE_LINE_TRACK_ONLY) {
            sprintf(mode_str, "TRACK ");
        } else if (current_mode == MODE_DISTANCE_FOLLOW_ONLY) {
            sprintf(mode_str, "FOLLOW");
        } else {
            sprintf(mode_str, "TRK+F ");
        }
        OLED_PrintASCIIString(&hOLED, 0, 32, mode_str, &afont8x6, OLED_COLOR_NORMAL);

        uint8_t left_sensor = (track_pattern & IR_TRACK_PATTERN_LEFT) ? 1 : 0;
        uint8_t middle_sensor = (track_pattern & IR_TRACK_PATTERN_MIDDLE) ? 1 : 0;
        uint8_t right_sensor = (track_pattern & IR_TRACK_PATTERN_RIGHT) ? 1 : 0;

        if (left_sensor) {
            OLED_DrawFilledCircle(&hOLED, 50, 46, 4, OLED_COLOR_NORMAL);
        } else {
            OLED_DrawCircle(&hOLED, 50, 46, 4, OLED_COLOR_NORMAL);
        }

        if (middle_sensor) {
            OLED_DrawFilledCircle(&hOLED, 75, 46, 4, OLED_COLOR_NORMAL);
        } else {
            OLED_DrawCircle(&hOLED, 75, 46, 4, OLED_COLOR_NORMAL);
        }

        if (right_sensor) {
            OLED_DrawFilledCircle(&hOLED, 100, 46, 4, OLED_COLOR_NORMAL);
        } else {
            OLED_DrawCircle(&hOLED, 100, 46, 4, OLED_COLOR_NORMAL);
        }

        uint32_t left_pulses = SpeedSensor_GetPulseCount(&hSpeedSensorLeft);
        uint32_t right_pulses = SpeedSensor_GetPulseCount(&hSpeedSensorRight);
        char pulse_str[24];
        sprintf(pulse_str, "L:%5lu R:%5lu", left_pulses, right_pulses);
        OLED_PrintASCIIString(&hOLED, 0, 56, pulse_str, &afont8x6, OLED_COLOR_NORMAL);

        OLED_ShowFrame(&hOLED);
    }

    HAL_Delay(10);
}

/**
  * @brief Read EC11 encoder delta and update target speed
  * Called from main loop (not time-critical)
  */
void Encoder_Update_TargetSpeed(void)
{
    int16_t enc_delta = EC11_GetDelta(&hEncoder);

    if (enc_delta != 0) {
        target_speed += enc_delta * ENCODER_SPEED_STEP;

        if (target_speed < TARGET_SPEED_MIN) target_speed = TARGET_SPEED_MIN;
        if (target_speed > TARGET_SPEED_MAX) target_speed = TARGET_SPEED_MAX;
    }
}

/**
  * @brief Read VL53L0X laser distance sensor
  * Called from main loop (I2C is slow, don't run in ISR)
  */
void Laser_Update_Distance(void)
{
    if (VL53L0X_ReadDistance(&hLaser) == HAL_OK) {
        if (VL53L0X_IsValid(&hLaser)) {
            laser_distance_mm = VL53L0X_GetDistance(&hLaser);
            laser_valid = 1;
            sprintf(dist_str, "Dist:%4dmm", laser_distance_mm);
        } else {
            laser_valid = 0;
        }
    }
}

/**
  * @brief 10ms control task - called from TIM4 ISR
  * This is the core control loop running at 100Hz
  * Mode-based control:
  *   MODE_STOP: Stop car
  *   MODE_LINE_TRACK_ONLY: Line tracking only, ignore laser
  *   MODE_DISTANCE_FOLLOW_ONLY: Distance following only, ignore IR
  *   MODE_LINE_TRACK_DISTANCE_FOLLOW: Both line tracking and distance following
  */
void Control_Task_10ms(void)
{
    Mode_Transition_Process();

    // 更新速度
    SpeedSensor_UpdateSpeed(&hSpeedSensorLeft, CM_PER_PULSE);
    SpeedSensor_UpdateSpeed(&hSpeedSensorRight, CM_PER_PULSE);

    if (current_mode == MODE_STOP) {
        Motor_Apply_Speed(0, 0);
        return;
    }

    int8_t base_speed = (int8_t)target_speed;

    int8_t left_out = base_speed;
    int8_t right_out = base_speed;

    if (current_mode == MODE_DISTANCE_FOLLOW_ONLY || current_mode == MODE_LINE_TRACK_DISTANCE_FOLLOW) {
        Distance_PID_Compute();
        left_out += (int8_t)distance_correction;
        right_out += (int8_t)distance_correction;
    } else {
        PID_Reset(&pidDistance);
        distance_correction = 0.0f;
    }

    if (current_mode == MODE_LINE_TRACK_ONLY || current_mode == MODE_LINE_TRACK_DISTANCE_FOLLOW) {
        LineTracking_Correct(&left_out, &right_out);
    }

    float left_spd = hSpeedSensorLeft.speed_cm_s;
    float right_spd = hSpeedSensorRight.speed_cm_s;
    float speed_diff = left_spd - right_spd;
    if (speed_diff > SPEED_DIFF_LIMIT) speed_diff = SPEED_DIFF_LIMIT;
    if (speed_diff < -SPEED_DIFF_LIMIT) speed_diff = -SPEED_DIFF_LIMIT;
    speed_diff_correction = speed_diff * SPEED_DIFF_KP;
    
    left_out += (int8_t)speed_diff_correction;
    right_out -= (int8_t)speed_diff_correction;

    if (left_out > 100) left_out = 100;
    if (left_out < -100) left_out = -100;
    if (right_out > 100) right_out = 100;
    if (right_out < -100) right_out = -100;

    Motor_Apply_Speed(left_out, right_out);
}

/**
  * @brief Pure PID line-tracking correction
  * Error coding:
  *   left=1, middle=0, right=0 → error=-2
  *   left=1, middle=1, right=0 → error=-1
  *   left=0, middle=1, right=0 → error=0
  *   left=0, middle=1, right=1 → error=1
  *   left=0, middle=0, right=1 → error=2
  */
void LineTracking_Correct(int8_t* left_speed, int8_t* right_speed)
{
    uint8_t left_detected = (track_pattern & IR_TRACK_PATTERN_LEFT) ? 1 : 0;
    uint8_t middle_detected = (track_pattern & IR_TRACK_PATTERN_MIDDLE) ? 1 : 0;
    uint8_t right_detected = (track_pattern & IR_TRACK_PATTERN_RIGHT) ? 1 : 0;

    uint8_t is_going_straight = 0;
    float error = 0.0f;
    
    if (left_detected == 1 && middle_detected == 0 && right_detected == 0) {
        error = -1.0f;
    } else if (left_detected == 1 && middle_detected == 1 && right_detected == 0) {
        error = -2.0f;
    } else if (left_detected == 0 && middle_detected == 1 && right_detected == 0) {
        error = 0.0f;
        is_going_straight = 1;
    } else if (left_detected == 0 && middle_detected == 1 && right_detected == 1) {
        error = 2.0f;
    } else if (left_detected == 0 && middle_detected == 0 && right_detected == 1) {
        error = 1.0f;
    } else if (left_detected == 1 && middle_detected == 0 && right_detected == 1) {
        error = 0.0f;
        is_going_straight = 1;
    } else if (left_detected == middle_detected && middle_detected == right_detected) {
        error = 0.0f;
        is_going_straight = 1;
    }
    
    if (is_going_straight) {
        PID_Reset(&pidLineTrack);
    } else {
        float correction = PID_Calc(&pidLineTrack, 0.0f, error);
        *left_speed -= (int8_t)correction;
        *right_speed += (int8_t)correction;
    }
}

/**
 * @brief Laser distance following PID computation
 * Logic: Near (distance < target) -> slow down, Far (distance > target) -> speed up
 * When laser returns valid distance, adjust speed to maintain target distance
 * When invalid, car runs at encoder-set target speed (normal mode)
 */
void Distance_PID_Compute(void)
{
    if (!laser_valid || laser_distance_mm == 0) {
        PID_Reset(&pidDistance);
        distance_correction = 0.0f;
        return;
    }

    float current_dist_cm = (float)laser_distance_mm / 10.0f;

    distance_correction = -PID_Calc(&pidDistance, DISTANCE_TARGET_CM, current_dist_cm);
}

void Motor_Apply_Speed(int8_t left, int8_t right)
{
    int8_t left_out = left;
    int8_t right_out = right;

    if (left_out > 100) left_out = 100;
    if (left_out < -100) left_out = -100;
    if (right_out > 100) right_out = 100;
    if (right_out < -100) right_out = -100;

    left_motor_speed = left_out;
    right_motor_speed = right_out;

    TB6612_DualSetSpeed(&hMotors, left_out, right_out);
}

/**
  * @brief Speed difference correction based on photointerrupter readings
  * This is the ONLY authoritative source for straight-line determination
  * If left_speed == right_speed -> go straight, no correction
  * If left_speed > right_speed -> steer right slightly
  * If left_speed < right_speed -> steer left slightly
  */
float Speed_Diff_Correct(float left_spd, float right_spd)
{
    static float integral = 0.0f;
    float error, output;
    float diff_kp = SPEED_DIFF_KP;
    float diff_ki = 0.05f;
    float diff_limit = SPEED_DIFF_LIMIT;

    error = right_spd - left_spd;

    integral += error * diff_ki;
    if (integral > diff_limit) integral = diff_limit;
    if (integral < -diff_limit) integral = -diff_limit;

    output = diff_kp * error + integral;

    if (output > diff_limit) output = diff_limit;
    if (output < -diff_limit) output = -diff_limit;

    return output;
}

/**
  * @brief Process button on PA3 to switch between modes
  * Button press: cycle through modes (with edge detection)
  */
#define MODE_BUTTON_PIN     GPIOA
#define MODE_BUTTON_PIN_NUM GPIO_PIN_3

void App_Process_Button(void)
{
    static uint32_t button_press_start = 0;
    static uint8_t button_was_pressed = 0;

    uint8_t button_state = HAL_GPIO_ReadPin(MODE_BUTTON_PIN, MODE_BUTTON_PIN_NUM);

    if (button_state == GPIO_PIN_RESET) {
        if (!button_was_pressed) {
            button_press_start = HAL_GetTick();
            button_was_pressed = 1;
        }

        if (HAL_GetTick() - button_press_start > 1000) {
            target_mode = MODE_STOP;
        }
    } else {
        if (button_was_pressed) {
            uint32_t press_duration = HAL_GetTick() - button_press_start;

            if (press_duration < 1000) {
                Mode_Switch();
            }
        }
        button_was_pressed = 0;
    }
}

/**
  * @brief Cycle through modes when button is pressed
  * Order: STOP -> LINE_TRACK_ONLY -> DISTANCE_FOLLOW_ONLY -> LINE_TRACK_DISTANCE_FOLLOW -> STOP
  */
void Mode_Switch(void)
{
    switch (target_mode) {
        case MODE_STOP:
            target_mode = MODE_LINE_TRACK_ONLY;
            break;
        case MODE_LINE_TRACK_ONLY:
            target_mode = MODE_DISTANCE_FOLLOW_ONLY;
            break;
        case MODE_DISTANCE_FOLLOW_ONLY:
            target_mode = MODE_LINE_TRACK_DISTANCE_FOLLOW;
            break;
        case MODE_LINE_TRACK_DISTANCE_FOLLOW:
        default:
            target_mode = MODE_STOP;
            break;
    }
}

/**
  * @brief Smooth transition between modes
  * Transition duration: 500ms, gradually adjust speed during transition
  */
#define MODE_TRANSITION_MS  500

void Mode_Transition_Process(void)
{
    if (current_mode != target_mode) {
        if (current_mode == MODE_STOP) {
            transition_target_speed = (int8_t)target_speed;
        } else {
            transition_target_speed = (left_motor_speed + right_motor_speed) / 2;
        }

        mode_transition_start = HAL_GetTick();
        current_mode = target_mode;

        PID_Reset(&pidLineTrack);
        PID_Reset(&pidDistance);

        if (current_mode == MODE_STOP) {
            Motor_Apply_Speed(0, 0);
        }
    }
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
