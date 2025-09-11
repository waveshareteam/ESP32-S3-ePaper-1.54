#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "user_app.h"
#include "driver/gpio.h"
#include "user_config.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/rtc_io.h"
#include "esp_sleep.h"

#include "src/button_bsp/button_bsp.h"     
#include "src/ui_src/generated/gui_guider.h"
#include "src/i2c_bsp/i2c_bsp.h"
#include "src/power/board_power_bsp.h"


#include "audio_bsp.h"

epaper_driver_display *driver = NULL;
board_power_bsp_t board_div(EPD_PWR_PIN,Audio_PWR_PIN,VBAT_PWR_PIN);

lv_ui src_ui;
static EventGroupHandle_t lvgl_button_groups;
static uint8_t *audio_ptr = NULL;


void user_app_init(void)
{
  lvgl_button_groups = xEventGroupCreate();
	audio_ptr = (uint8_t *)heap_caps_malloc(288 * 1000 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);

  //board_div.VBAT_POWER_ON();
  board_div.POWEER_EPD_ON();
  board_div.POWEER_Audio_ON();
  i2c_master_Init();
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

  user_button_init();
  audio_bsp_init();
  audio_play_init();
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

void button_boot_task(void *arg)
{
  for(;;)
  {
    EventBits_t even = xEventGroupWaitBits(boot_groups,set_bit_all,pdTRUE,pdFALSE,pdMS_TO_TICKS(2 * 1000));
    if(get_bit_button(even,0))
    {
      xEventGroupSetBits(lvgl_button_groups,0x02);
    }
    else if(get_bit_button(even,3))
    {
      xEventGroupSetBits(lvgl_button_groups,0x01);
    }
  }
}

void i2s_audio_Test(void *arg)
{
	lv_ui *ui = (lv_ui *)arg;
	for(;;)
	{
		EventBits_t even = xEventGroupWaitBits(lvgl_button_groups,0x00ffffff,pdTRUE,pdFALSE,pdMS_TO_TICKS(8 * 1000));
		if(even & 0x01)
		{
			lv_label_set_text(ui->screen_label_1, "正在录音");
			lv_label_set_text(ui->screen_label_2, "Recording...");

			audio_playback_read(audio_ptr,192 * 1000);
			lv_label_set_text(ui->screen_label_1, "录音完成");
			lv_label_set_text(ui->screen_label_2, "Rec Done");
		}
		else if(even & 0x02)
		{
			lv_label_set_text(ui->screen_label_1, "正在播放");
			lv_label_set_text(ui->screen_label_2, "Playing...");
			audio_playback_write(audio_ptr,192 * 1000);
			lv_label_set_text(ui->screen_label_1, "播放完成");
			lv_label_set_text(ui->screen_label_2, "Play Done");
		}
		else
		{
			lv_label_set_text(ui->screen_label_1, "等待操作");
			lv_label_set_text(ui->screen_label_2, "Idle");
		}
	}
}

void user_ui_init(void)
{
  setup_ui(&src_ui);
  lv_label_set_text(src_ui.screen_label_1, "等待操作");
	lv_label_set_text(src_ui.screen_label_2, "Idle");
  xTaskCreatePinnedToCore(led_test_task, "led_test_task", 4 * 1024, NULL, 4, NULL,1);
  xTaskCreatePinnedToCore(button_boot_task, "button_boot_task", 4 * 1024, &src_ui, 4, NULL,1);
  xTaskCreatePinnedToCore(i2s_audio_Test, "i2s_audio_Test", 4 * 1024, &src_ui, 3, NULL,1);      //audio        
}

