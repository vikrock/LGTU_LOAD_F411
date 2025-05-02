/*
 * encoder.h
 *
 *  Created on: Apr 29, 2025
 *      Author: Сергей
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

// Includes
#include "stm32f4xx_hal.h" // Подключил библиотеку HAL
#include "gpio.h" // Подключил файл GPIO
#include "ssd1306.h" // Подключил библиотеку экрана
#include "arm_math.h" // Подключил библиотеку математики

// Объявление переменных
extern uint16_t counter_encoder; // переменная для хранения значений счетчика энкодера от 0 до 300
extern uint8_t menu_item; // переменная для хранения значений пунктов меню экрана от 1 до 2
extern uint8_t button_flag; // переменная для хранения флага прерывания от кнопки энкодера
extern uint8_t short_button_flag; // переменная для хранения флага кратковременного нажатия кнопки энкодера
extern uint8_t long_button_flag; // переменная для хранения флага длительного нажатия кнопки энкодера
extern uint8_t short_press; // переменная для хранения значения кнопки энкодера
extern uint8_t long_press; // переменная для хранения значения кнопки энкодера
extern uint32_t time_key; // время нажатия кнопки

// Переменные для редактирования энкодером
extern float32_t current_value; // переменная для хранения значения тока настроенного энкодером
extern float32_t voltage_value; // переменная для хранения значения напряжения настроенного энкодером

// Объявление функций
void Encoder_Init(void); // Обработка функции HAL на включение всех каналов первого таймера энкодера
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim); // Обработчик прерывания таймера энкодера
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin); // Обработчик прерывания кнопки энкодера
void Button_click_process(void); // Реализация короткого и длинного нажатия с устранением дребезга
void Short_Press(void); // Функция для обработки действий при кратковременном нажатии
void Long_Press(void); // Функция для обработки действий длительного нажатия

#endif /* INC_ENCODER_H_ */
