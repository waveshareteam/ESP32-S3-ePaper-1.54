#include "sdcard_bsp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define sdcard_write_Test

void setup()
{
  Serial.begin(115200);
  delay(3000);
  sdcard_init();
}

uint32_t value = 1;
char test[45] = {""};
char rtest[45] = {""};
void loop()
{
#ifdef sdcard_write_Test
  snprintf(test,45,"sdcard_writeTest : %ld\n",value);
  s_example_write_file("/sdcard/writeTest.txt",test);
  delay(500);
  s_example_read_file("/sdcard/writeTest.txt",rtest,NULL);
  Serial.printf("rtest:%s\n",rtest);
  delay(500);
  value++;
#endif
}
