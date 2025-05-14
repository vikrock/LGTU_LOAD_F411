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
int main(void) {

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
	start_screen(); // рисуем стартовый экран
	upd_mode(1); // выводим стартовый режим
	upd_type(1); // выводим стартовый тип нагрузки
	//вывод тестовых значений на экран:
	upd_chisl(15.0, 4);
	upd_chisl(10.0, 5);
	//upd_chisl(15.0, 2);
	//upd_chisl(23.8, 3);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		change_screen (long_press); // опрашиваем смену экрана по длительному нажатию кнопки энкодера

		if (button_flag) {
			Button_click_process(); // Если полнялся флажок прерывания кнопки энкодера, выполняем функцию
		}

		draw_blinking_underline(menu_item); // Функция для реализации моргания и статичного подчеркивания изменяемых энкодером значений

		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

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
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
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

// Функция обработки прерывания от таймера энкодера
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM1) { // Проверяем, что прерывание пришло от таймера 1
		counter_encoder = TIM1->CNT; // считываем значение регистра счетчика таймера 1
		if (short_press == 0) {
			menu_item = (counter_encoder / 2) % 4 + 1; // Если короткое нажатие не активно, преобразуем 0-300 в 1-4;
		}
		if (short_press == 1 && menu_item == 1) {
			mode_item = (counter_encoder / 2) % 2 + 1; // Если короткое нажатие активно и выбран первый пункт меню, преобразуем 0-300 в 1-2;
		}
		if (short_press == 1 && menu_item == 2) {
			type_item = (counter_encoder / 2) % 4 + 1; // Если короткое нажатие активно и выбран второй пункт меню, преобразуем 0-300 в 1-4;
		}
		if (short_press == 1) {
			if (menu_item == 4) {
				current_value = (float32_t) (counter_encoder) / 20.0f; // Вычисляем значение тока с шагом 0.1
				if (current_value > 10.0f)
					current_value = 10.0f; // Ограничение максимального значения
				upd_chisl(current_value, 4); // обновляем значение тока на экране
			}   if (menu_item == 3) {
				voltage_value = (float32_t) (counter_encoder) / 20.0f; // Вычисляем значение напряжения с шагом 0.1
				//if (voltage_value > 15.0f) voltage_value = 15.0f; // Можно не ограничивать так как максимальное значение = 15
				upd_chisl(voltage_value, 5); // обновляем значение напряжения на экране
			}   if (menu_item == 1) {
				upd_mode(mode_item);
			}   if (menu_item == 2) {
				upd_type(type_item);
			}
		}
	}
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
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
