#ifndef ADC_BSP_H
#define ADC_BSP_H

#include <Arduino.h>

void adc_get_value(float *value,int *data);
void adc_bsp_init(void);
void adc_example(void* parmeter);
#endif