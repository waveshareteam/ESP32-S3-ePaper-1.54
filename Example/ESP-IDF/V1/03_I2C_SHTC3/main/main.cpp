#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bsp.h"
#include "user_config.h"
#include "i2c_equipment.h"
#include "board_power_bsp.h"


board_power_bsp_t board_div(EPD_PWR_PIN,Audio_PWR_PIN,VBAT_PWR_PIN);
i2c_equipment_shtc3 *shtc3_dev = NULL;

void i2c_SHTC3_loop_task(void *arg)
{
  	for(;;)
  	{
  	  	shtc3_data_t shtc3_data = shtc3_dev->readTempHumi();
  	  	printf("RH:%.2f%%,Temp:%.2f° \n",shtc3_data.RH,shtc3_data.Temp);  
  	  	vTaskDelay(pdMS_TO_TICKS(1000));
  	}
}


extern "C" void app_main(void)
{
	board_div.VBAT_POWER_ON();
    board_div.POWEER_EPD_ON();
    board_div.POWEER_Audio_ON();
  	i2c_master_Init();
	shtc3_dev = new i2c_equipment_shtc3();

	xTaskCreatePinnedToCore(i2c_SHTC3_loop_task, "i2c_SHTC3_loop_task", 3 * 1024, NULL , 2, NULL,0); 
}
