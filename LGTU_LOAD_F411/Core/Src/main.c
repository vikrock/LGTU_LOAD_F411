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
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include "ssd1306.h"
#include "encoder.h"
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
char string[4] = { 0, };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Encoder_Start_IT(&htim1, TIM_CHANNEL_ALL); // Запускаем энкодер в режиме прерывания и запускаем каналы таймера 1
	//запуск экрана:
	ssd1306_Init();
	start_screen(mode_item); // рисуем стартовый экран
	upd_mode(1); // выводим стартовый режим
	//вывод тестовых значений на экран:
	//upd_chisl(0.0, 2);
	//upd_chisl(0.0, 3);
	//upd_chisl(15.0, 2);
	//upd_chisl(23.8, 3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {



		if (button_flag) {
			Button_click_process(); // Если полнялся флажок прерывания кнопки энкодера, выполняем функцию
		}
		change_screen (long_press); // опрашиваем смену экрана по длительному нажатию кнопки энкодера
		if (mode_item == 2) {
		draw_blinking_underline_disch(menu_item_disch); // Функция для реализации моргания и статичного подчеркивания изменяемых энкодером значений
		}
	    if (mode_item == 1) {
		draw_blinking_underline_load(menu_item_load);
		}
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Функция обработки прерывания от кнопки энкодера
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_14) { // Проверяем что прерывание от GPIO_PIN_14
		button_flag = 1; // Устанавливаем флажок, что пришло прерывание от кнопки энкодера
	}
}

// Функция для обработки прерывания энкодера

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    static uint16_t prev_counter_encoder = 0; // Переменная для хранения предыдущего значения counter_encoder
    int16_t encoder_diff = 0; // Переменная которая хранит разницу текущего и предыдущего значения

    if (htim->Instance == TIM1) { // Проверяем, что прерывание пришло от таймера 1

    	if (long_press == 0) {
    	encoder_diff = TIM1->CNT - prev_counter_encoder; // Вычисляем разницу между текущим и предыдущим значением счетчика
        counter_encoder = TIM1->CNT; // текущее значение счетчика
        }

    	if (short_press == 0 && long_press == 0 && mode_item == 1) {
    	    menu_item_load = (counter_encoder >> 1) % 3U + 1U; // Вычисляем выбранный пункт меню (1-3)
    	}

        if (short_press == 0 && long_press == 0 && mode_item == 2) {
            menu_item_disch = (counter_encoder >> 1) % 4U + 1U; // Вычисляем выбранный пункт меню (1-4)
        }


        if (short_press == 1 && long_press == 0 && mode_item == 2) {
                switch (menu_item_disch) {
                    case 1:
                        mode_item = (counter_encoder >> 1) % 2U + 1U;
                        start_screen(mode_item);
                        upd_mode(mode_item);
                        break;
                    case 2:
                        type_item = (counter_encoder >> 1) % 4U + 1U;
                        upd_type(type_item);
                        break;
                    case 3:
                        voltage_value += 0.1f * (encoder_diff >> 1);
                        if (voltage_value > 15.0f) voltage_value = 15.0f;
                        if (voltage_value < 0.0f) voltage_value = 0.0f;
                        upd_chisl(voltage_value, 5);
                        break;
                    case 4:
                        current_value += 0.1f * (encoder_diff >> 1);
                        if (current_value > 10.0f) current_value = 10.0f;
                        if (current_value < 0.0f) current_value = 0.0f;
                        upd_chisl(current_value, 4);
                        break;
                }
            }
        if (short_press == 1 && long_press == 0 && mode_item == 1) {
                        switch (menu_item_load) {
                            case 1:
                                mode_item = (counter_encoder >> 1) % 2U + 1U;
                                start_screen(mode_item);
                                upd_mode(mode_item);
                                break;
                            case 2:
                            voltage_value += 0.1f * (encoder_diff >> 1);
                            if (voltage_value > 15.0f) voltage_value = 15.0f;
                            if (voltage_value < 0.0f) voltage_value = 0.0f;
                            upd_chisl(voltage_value, 3);
                            break;
                           case 3:
                             current_value += 0.1f * (encoder_diff >> 1);
                            if (current_value > 10.0f) current_value = 10.0f;
                            if (current_value < 0.0f) current_value = 0.0f;
                            upd_chisl(current_value, 2);
                            break;

                        }
        }
        prev_counter_encoder = counter_encoder; // Сохраняем текущее значение для следующего вызова
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
	while (1) {
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
