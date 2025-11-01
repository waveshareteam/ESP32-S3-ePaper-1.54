#include <stdio.h>
#include "user_app.h"

#include "sdcard_bsp.h"

#define sdcard_write_Test

void sdcard_loop_task(void *arg);

void user_app_init(void)
{
    sdcard_init();
    xTaskCreatePinnedToCore(sdcard_loop_task, "sdcard_loop_task", 5 * 1024, NULL , 2, NULL,0); //sd card test
}


void sdcard_loop_task(void *arg)
{
  	uint32_t value = 1;
  	char test[45] = {""};
  	char rtest[45] = {""};
  	for(;;)
  	{
#ifdef sdcard_write_Test
  		snprintf(test,45,"sdcard_writeTest : %ld\n",value);
  		s_example_write_file("/sdcard/writeTest.txt",test);
  		vTaskDelay(pdMS_TO_TICKS(500));
  		s_example_read_file("/sdcard/writeTest.txt",rtest,NULL);
  		printf("rtest:%s\n",rtest);
  		vTaskDelay(pdMS_TO_TICKS(500));
  		value++;
#else
  		vTaskDelay(pdMS_TO_TICKS(500));  
#endif
  	}
}




