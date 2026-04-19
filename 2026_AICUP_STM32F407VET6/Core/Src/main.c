/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "MecanumWheelPlatform.h"
#include "oled.h"
#include <stdio.h>
#include <stdlib.h>
#include "Bluetooth.h"
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
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void OLED_Show_Data(void);
char* float_to_str(float value, char* buffer);
void Human_Control(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_CAN1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
    OLED_Init();
    HAL_Delay(100);
    OLED_DisPlay_On();
    OLED_NewFrame();
    OLED_DrawImage(0, 5, &ScuImg, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
    MecnaumuWheelPlatform_Init();
    Bluetooth_Init();
    HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      //OLED_Show_Data();
      Human_Control();
//
//      f_ROS.linear_x = 0;
//      f_ROS.linear_y = 0;
//      f_ROS.angular_z = 0;

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void OLED_Show_Data() {
    OLED_NewFrame();
    snprintf(oled_buffer, sizeof(oled_buffer), "%ld", IMU406_YAW);
    OLED_PrintASCIIString(0, 0, "YAW:", &afont8x6, OLED_COLOR_NORMAL);
    OLED_PrintASCIIString(24, 0, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    snprintf(oled_buffer, sizeof(oled_buffer), "%ld", IMU406_PITCH);
//    OLED_PrintASCIIString(64, 8, "PIT:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(88, 8, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    snprintf(oled_buffer, sizeof(oled_buffer), "%ld", IMU406_ROLL);
//    OLED_PrintASCIIString(0, 8, "ROL:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(24, 8, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    snprintf(oled_buffer, sizeof(oled_buffer), "%d", JOYSTICK_FB);
//    OLED_PrintASCIIString(0, 20, "Joy_X:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(36, 20, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    snprintf(oled_buffer, sizeof(oled_buffer), "%d", JOYSTICK_LR);
//    OLED_PrintASCIIString(66, 20, "Joy_y:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(102, 20, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    snprintf(oled_buffer, sizeof(oled_buffer), "%d", JOYSTICK_Z);
//    OLED_PrintASCIIString(0, 28, "Joy_Z:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(36, 28, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);

//    float_to_str(t_ROS.v_x, oled_buffer);
//    OLED_PrintASCIIString(0, 40, "X:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(12, 40, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    float_to_str(t_ROS.v_y, oled_buffer);
//    OLED_PrintASCIIString(66, 40, "Y:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(78, 40, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
////
//    float_to_str(motor[2].current_speed, oled_buffer);
//    OLED_PrintASCIIString(0, 48, "C:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(12, 48, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    float_to_str((float)-1.123, oled_buffer);
//    OLED_PrintASCIIString(66, 48, "D:", &afont8x6, OLED_COLOR_NORMAL);
//    OLED_PrintASCIIString(78, 48, oled_buffer, &afont8x6, OLED_COLOR_NORMAL);
//
//    if (HAL_GPIO_ReadPin(GPIOA, BluetoothState_Pin) == GPIO_PIN_RESET) {
//        JOYSTICK_Z = false;
//        OLED_PrintASCIIString(66, 28, "BT_OFF", &afont8x6, OLED_COLOR_NORMAL);
//    }
//    else {
//        OLED_PrintASCIIString(66, 28, "BT_ON", &afont8x6, OLED_COLOR_NORMAL);
//    }

    OLED_ShowFrame();
}

char* float_to_str(float value, char* buffer) {
    int bp = (int) value;
    float ap = abs((value - (float) bp) * 100);
    snprintf(buffer, sizeof(buffer), "%d%d", bp, (int)ap);

    return buffer;
}

void Human_Control(void) {
    if (HAL_GPIO_ReadPin(GPIOA, BluetoothState_Pin) == GPIO_PIN_SET) {
        if (JOYSTICK_FB == 0x46) {
            f_ROS.linear_x = 0.2;
        } else if (JOYSTICK_FB == 0x42){
            f_ROS.linear_x = -0.2;
        }
        else {
            f_ROS.linear_x = 0;
        }

        if (JOYSTICK_Z == 0x01) {
            if (JOYSTICK_LR == 0x4C) {
                f_ROS.linear_y = -0.2;
            } else if (JOYSTICK_LR == 0x52){
                f_ROS.linear_y = 0.2;
            }
            else {
                f_ROS.linear_y = 0;
            }
        }
        else if (JOYSTICK_Z == 0x00) {
            if (JOYSTICK_LR == 0x4C) {
                f_ROS.angular_z = 1;
            } else if (JOYSTICK_LR == 0x52){
                f_ROS.angular_z = -1;
            }
            else {
                f_ROS.angular_z = 0;
            }
        }

    }

}

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
