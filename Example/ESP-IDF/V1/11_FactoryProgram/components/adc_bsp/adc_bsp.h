#ifndef ADC_BSP_H
#define ADC_BSP_H

#include "esp_adc/adc_oneshot.h"

class adc_bsp
{
private:
    adc_oneshot_unit_handle_t adc1_handle;
    adc_cali_handle_t cali_handle;

public:
    adc_bsp();
    ~adc_bsp();

    float get_vbatVoltage();
    uint8_t get_Batterylevel();
};

#endif
