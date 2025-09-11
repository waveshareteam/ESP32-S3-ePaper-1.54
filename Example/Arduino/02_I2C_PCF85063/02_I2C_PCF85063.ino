#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_config.h"
#include "i2c_equipment.h"
#include "board_power_bsp.h"
#include "i2c_bsp.h"

board_power_bsp_t board_div(EPD_PWR_PIN,Audio_PWR_PIN,VBAT_PWR_PIN);
i2c_equipment *rtc_dev = NULL;

void i2c_rtc_loop_task(void *arg)
{
  for(;;)
  {
    RtcDateTime_t datetime = rtc_dev->get_rtcTime();
    printf("%d/%d/%d %d:%d:%d \n",datetime.year,datetime.month,datetime.day,datetime.hour,datetime.minute,datetime.second);  
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{
  board_div.POWEER_EPD_ON();
  board_div.POWEER_Audio_ON();
  Serial.begin(115200);
  i2c_master_Init();
  rtc_dev = new i2c_equipment();
  rtc_dev->set_rtcTime(2025,9,10,8,30,30);

  xTaskCreatePinnedToCore(i2c_rtc_loop_task, "i2c_rtc_loop_task", 3 * 1024, NULL , 2, NULL,0); 
}

void loop() 
{
  
}
