#include <stdio.h>
#include "adc_bsp.h"

adc_bsp::adc_bsp()
{
    adc_cali_curve_fitting_config_t cali_config = {};
        cali_config.unit_id = ADC_UNIT_1;
        cali_config.atten = ADC_ATTEN_DB_12;
        cali_config.bitwidth = ADC_BITWIDTH_12; //4096
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
    
    adc_oneshot_unit_init_cfg_t init_config1 = {};
        init_config1.unit_id = ADC_UNIT_1;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
    adc_oneshot_chan_cfg_t config = {};
        config.bitwidth = ADC_BITWIDTH_12;
        config.atten = ADC_ATTEN_DB_12;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
}

adc_bsp::~adc_bsp()
{

}

float adc_bsp::get_vbatVoltage()
{
    int OriginalData;
    int CalibratedData;
    float VbatVoltage;
    adc_oneshot_read(adc1_handle,ADC_CHANNEL_3,&OriginalData);
    adc_cali_raw_to_voltage(cali_handle,OriginalData,&CalibratedData);
    VbatVoltage = 0.001 * CalibratedData * 2;
    return VbatVoltage;
}

/*4.12V满电 3*/
uint8_t adc_bsp::get_Batterylevel() 
{
    float vol = get_vbatVoltage();
    if(vol < 3.0)
    {
        return 0;
    }
    if(vol > 4.12)
    {
        return 100;
    }
    float level = (vol / 4.12) * 100;
    return (uint8_t)level;
}
