uint8_t red_reflection = HAL_GPIO_ReadPin(RED_GPIO_GPIO_Port, RED_GPIO_Pin);
HAL_ADC_Start(&hadc1);
uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
