#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "user_app.h"
#include "driver/gpio.h"
#include "user_config.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/rtc_io.h"
#include "esp_sleep.h"
  
#include "src/ui_src/generated/gui_guider.h"
#include "src/power/board_power_bsp.h"

#include "src/i2c_equipment/i2c_equipment.h"
#include "src/i2c_dev/i2c_bsp.h"
#include "src/button/button_bsp.h"

i2c_equipment *rtc_dev = NULL;
epaper_driver_display *driver = NULL;
board_power_bsp_t board_div(EPD_PWR_PIN,Audio_PWR_PIN,VBAT_PWR_PIN);

lv_ui src_ui;
int is_power_sleep_flag = 0;

static void get_wakeup_gpio(void) {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (ESP_SLEEP_WAKEUP_EXT1 == wakeup_reason) {
    uint64_t wakeup_pins = esp_sleep_get_ext1_wakeup_status();
    if (wakeup_pins == 0)
      return;
    if (wakeup_pins & (1ULL << GPIO_NUM_18)) {
      is_power_sleep_flag = 1;
      lv_label_set_text(src_ui.screen_label_3, "POWER OFF");
      //vTaskDelay(pdMS_TO_TICKS(1000));
      ESP_ERROR_CHECK(gpio_hold_dis(GPIO_NUM_17));
      board_div.VBAT_POWER_OFF();
    }
  }
}

void user_app_init(void)
{
  i2c_master_Init();
  user_button_init();
  board_div.VBAT_POWER_ON();
  board_div.POWEER_EPD_ON();
  board_div.POWEER_Audio_ON();
  /*epaper init*/
  custom_lcd_spi_t driver_config = {};
    driver_config.cs = EPD_CS_PIN;
    driver_config.dc = EPD_DC_PIN;
    driver_config.rst = EPD_RST_PIN;
    driver_config.busy = EPD_BUSY_PIN;
    driver_config.mosi = EPD_MOSI_PIN;
    driver_config.scl = EPD_SCK_PIN;
    driver_config.spi_host = EPD_SPI_NUM;
    driver_config.buffer_len  = 5000;
  driver = new epaper_driver_display(EPD_WIDTH,EPD_HEIGHT,driver_config);
  driver->EPD_Init();
  driver->EPD_Clear();
  driver->EPD_DisplayPartBaseImage();
  driver->EPD_Init_Partial();            //局部刷新初始化

  rtc_dev = new i2c_equipment();
	/*set rtc isr*/
  int year = rtc_dev->getDateTime().getYear(); //获取年份,观察rtc是否已经上电
  if(year < 2025) {                            //表示rtc已经断电
    rtc_dev->set_rtcAlarmSec(30);            //绝对时间,不是相对时间
    rtc_dev->enableAlarm();
  }
}

void led_test_task(void *arg)
{
  gpio_config_t gpio_conf = {};
  gpio_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_conf.mode = GPIO_MODE_OUTPUT;
  gpio_conf.pin_bit_mask = 0x1ULL<<3;
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));
  for(;;)
  {
    gpio_set_level((gpio_num_t)3,0);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level((gpio_num_t)3,1);
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

void loop_lvgl_task(void *arg)
{
  for(;;)
  {
    if(!is_power_sleep_flag) {
      vTaskDelay(pdMS_TO_TICKS(3000));
      lv_label_set_text(src_ui.screen_label_3, "Sleep");
      vTaskDelay(pdMS_TO_TICKS(1000));
      board_div.EnableDeepLowPowerMode();
    } else {
      vTaskDelay(pdMS_TO_TICKS(20));
    }
  }
}

void gpio_event_task(void *arg)
{
  for(;;)
  {
	if (rtc_dev->isAlarmActive()) {
		ESP_LOGE("rtc", "定时到");
		rtc_dev->resetAlarm();
	}
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void boot_button_user_Task(void *arg)
{
  for(;;)
  {
    EventBits_t even = xEventGroupWaitBits(boot_groups, 0x02, pdTRUE,pdFALSE, pdMS_TO_TICKS(2000));
    if(even & 0x02) //长按
    {
      ESP_LOGE("SFS","12313");
      rtc_dev->set_rtcAlarmSec(30);            //绝对时间,不是相对时间
      RtcDateTime_t time = rtc_dev->get_rtcTime();
      char str[50] = {""};
      snprintf(str,50,"%d/%02d/%02d\n%02d:%02d:%02d",time.year,time.month,time.day\
      ,time.hour,time.minute,time.second);
      lv_label_set_text(src_ui.screen_label_2, str);
    }
  }
}

void user_ui_init(void)
{
  setup_ui(&src_ui);
  get_wakeup_gpio();
  RtcDateTime_t time = rtc_dev->get_rtcTime();
  char str[50] = {""};
  snprintf(str,50,"%d/%02d/%02d\n%02d:%02d:%02d",time.year,time.month,time.day\
  ,time.hour,time.minute,time.second);
  lv_label_set_text(src_ui.screen_label_2, str);
  if(!is_power_sleep_flag)
  {lv_label_set_text(src_ui.screen_label_3, "POWER ON");}
  xTaskCreatePinnedToCore(led_test_task, "led_test_task", 4 * 1024, NULL, 4, NULL,1);
  xTaskCreatePinnedToCore(loop_lvgl_task, "loop_lvgl_task", 4 * 1024, NULL, 4, NULL,1);
  xTaskCreatePinnedToCore(gpio_event_task, "gpio_event_task", 4 * 1024, NULL, 4, NULL,1);
  xTaskCreatePinnedToCore(boot_button_user_Task, "boot_button_user_Task", 4 * 1024,NULL, 6, NULL,1);
  //xTaskCreatePinnedToCore(pwr_button_user_Task, "boot_button_user_Task", 4 * 1024,NULL, 3, NULL,1);        
}
