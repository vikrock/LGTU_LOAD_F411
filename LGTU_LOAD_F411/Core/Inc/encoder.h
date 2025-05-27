/*
 * encoder.h
 *
 *  Created on: Apr 29, 2025
 *      Author: Сергей
 */

#ifndef INC_ENCODER_H_
#define INC_ENCODER_H_

#include "arm_math.h"

// Объявление переменных
extern uint16_t counter_encoder; // переменная для хранения значений счетчика энкодера от 0 до 300
extern uint8_t menu_item_disch; // переменная для хранения значений пунктов меню экрана от 1 до 4
extern uint8_t menu_item_load;
extern uint8_t type_item;    // переменная для хранениея значения типов нагрузки
extern uint8_t mode_item;      // переменная для хранениея значения типов режима
extern uint8_t button_flag; // переменная для хранения флага прерывания с кнопки энкодера
extern uint8_t short_button_flag; // переменная для хранения флага кратковременного нажатия кнопки энкодера
extern uint8_t long_button_flag; // переменная для хранения флага длительного нажатия кнопки энкодера
extern uint8_t short_press; // переменная для хранения значения кнопки энкодера
extern uint8_t long_press; 	// переменная для хранения значения кнопки энкодера
extern uint32_t time_key; 			// время нажатия кнопки
extern float32_t current_value; // переменная для хранения значения тока настроенного энкодером
extern float32_t voltage_value; // переменная для хранения значения напряжения настроенного энкодером
extern float32_t current_value; // переменная для хранения значения тока настроенного энкодером
extern float32_t voltage_value; // переменная для хранения значения напряжения настроенного энкодером

// Прототипы функций
void Button_click_process(void); // Реализация короткого и длинного нажатия с устранением дребезга
void Short_Press(void); // Функция для обработки действий при кратковременном нажатии
void Long_Press(void); // Функция для обработки действий длительного нажатия

#endif /* INC_ENCODER_H_ */
