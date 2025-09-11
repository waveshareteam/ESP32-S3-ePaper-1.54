#include <stdio.h>
#include "adc_bsp.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ADC_Calibrate

#ifdef ADC_Calibrate
  static adc_cali_handle_t cali_handle;
#endif
  static adc_oneshot_unit_handle_t adc1_handle;
void adc_bsp_init(void)
{
#ifdef ADC_Calibrate
  adc_cali_curve_fitting_config_t cali_config = 
  {
    .unit_id = ADC_UNIT_1,
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12, //4096
  };
  ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
#endif
  adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1, //ADC1
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
  adc_oneshot_chan_cfg_t config = {
    .atten = ADC_ATTEN_DB_12,//ADC_ATTEN_DB_12,         //    1.1          ADC_ATTEN_DB_12:3.3
    .bitwidth = ADC_BITWIDTH_12,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
}
void adc_get_value(float *value,int *data)
{
  int adcdata;
#ifdef ADC_Calibrate
  int vol = 0;
#endif  
  esp_err_t err;
  err = adc_oneshot_read(adc1_handle,ADC_CHANNEL_3,&adcdata);
  if(err == ESP_OK)
  {
#ifdef ADC_Calibrate
    adc_cali_raw_to_voltage(cali_handle,adcdata,&vol);
    *value = 0.001 * vol * 2;
#else
    *value = ((float)adcdata * 3.3/4096) * 2;
#endif
    if(data != NULL)
    {*data = adcdata;}
  }
  else
  {
    *value = 0;
    if(data != NULL)
    {*data = 0;}
  }
}
/*demo*/
void adc_example(void* parmeter)
{
  adc_bsp_init();
  int adcdata = 0;
  float _vol = 0;
  for(;;)
  {
    adc_get_value(&_vol,&adcdata);
    Serial.printf("adc value:%d,system voltage:%.2f \n",adcdata,_vol);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

