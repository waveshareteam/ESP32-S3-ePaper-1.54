#include <stdio.h>
#include "user_app.h"

#include "adc_bsp.h"

void user_app_init(void);

void user_top_init(void)
{
  Serial.println("adc-example run");
  user_app_init();
}
void user_app_init(void)
{
  xTaskCreate(adc_example, "adc_example", 3000, NULL , 2, NULL);
}



