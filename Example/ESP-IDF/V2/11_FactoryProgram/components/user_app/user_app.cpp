#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "user_app.h"
#include "driver/gpio.h"
#include "user_config.h"
#include "board_power_bsp.h"
#include "gui_guider.h"
#include "button_bsp.h"
#include "esp_log.h"
#include "esp_err.h"

#include "driver/rtc_io.h"
#include "esp_sleep.h"

#include "sdcard_bsp.h"
#include "i2c_bsp.h"

#include "i2c_equipment.h"
#include "adc_bsp.h"
#include "ble_scan_bsp.h"
#include "esp_wifi_bsp.h"
#include "audio_bsp.h"
#include "touch_bsp.h"

epaper_driver_display *driver = NULL;
i2c_equipment *rtc_dev = NULL;
i2c_equipment_shtc3 *shtc3_dev = NULL;
board_power_bsp_t board_div(EPD_PWR_PIN,Audio_PWR_PIN,VBAT_PWR_PIN);
adc_bsp adc_dev;
//touch_bsp *touch_dev = NULL;

uint8_t power_flag = 0;
uint8_t audio_flag = 0;

lv_ui src_ui;


void user_app_init(void)
{
    board_div.VBAT_POWER_ON();
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

    rtc_dev = new i2c_equipment();
    rtc_dev->set_rtcTime(2025,8,27,8,0,0);

    shtc3_dev = new i2c_equipment_shtc3();
    
    //touch_dev = new touch_bsp(ESP32_I2C_DEV_NUM,0x38,EPD_WIDTH,EPD_HEIGHT);

    user_button_init();
    _sdcard_init();
    espwifi_init();
    audio_bsp_init();
}

void button_power_task(void *srg)
{
    lv_ui *src = (lv_ui *)srg;
    uint8_t flag = 1;
    for(;;)
    {
        EventBits_t even = xEventGroupWaitBits(pwr_groups,set_bit_all,pdTRUE,pdFALSE,pdMS_TO_TICKS(2 * 1000));
  	  	if(get_bit_button(even,0))         //单击
  	  	{
            if(flag)
            {
                lv_obj_add_flag(src->screen_cont_1, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(src->screen_cont_2,LV_OBJ_FLAG_HIDDEN);
                flag = 0;
                audio_flag = 1;
            }
            else
            {
                lv_obj_add_flag(src->screen_cont_2, LV_OBJ_FLAG_HIDDEN);
                lv_obj_remove_flag(src->screen_cont_1,LV_OBJ_FLAG_HIDDEN);
                flag = 1;
                audio_flag = 0;
            }
        }
        else if(get_bit_button(even,1))
        {
            
        }
        else if(get_bit_button(even,2))
        {
            if(power_flag)
            {
                lv_label_set_text(src->screen_label_9, "OFF");
                lv_obj_invalidate(src->screen_label_9);
                vTaskDelay(pdMS_TO_TICKS(500));
                board_div.VBAT_POWER_OFF();
                board_div.POWEER_EPD_OFF();
                board_div.POWEER_Audio_OFF();
            }
        }
        else if(get_bit_button(even,3))
        {
            if(!power_flag)
            {
                power_flag = 1;
            }
        }
    }
}

void power_Testing_task(void *srg)
{
    vTaskDelay(pdMS_TO_TICKS(8000));
    if(!power_flag)
    power_flag = 1;
    vTaskDelete(NULL);
}

void user_lvgl_task(void *srg)
{
    lv_ui *src = (lv_ui *)srg;
    uint32_t times = 0;
    uint32_t rtc_time = 0;
    uint32_t shtc3_time = 0;
    uint32_t adc_time = 0;
    char lvgl_buffer[30] = {""};
    for(;;)
    {
        if(times - rtc_time == 5)
        {
            rtc_time = times;
            RtcDateTime_t rtc = rtc_dev->get_rtcTime();
            snprintf(lvgl_buffer,28,"%02d",rtc.hour);
            lv_label_set_text(src->screen_label_1, lvgl_buffer);
            snprintf(lvgl_buffer,28,"%02d",rtc.minute);
            lv_label_set_text(src->screen_label_2, lvgl_buffer);
        }
        if(times - shtc3_time == 25)
        {
            shtc3_time = times;
            shtc3_data_t shtc3_data = shtc3_dev->readTempHumi();
            snprintf(lvgl_buffer,28,"%d%%",(int)shtc3_data.RH);
            lv_label_set_text(src->screen_label_3, lvgl_buffer);
            snprintf(lvgl_buffer,28,"%d°",(int)shtc3_data.Temp);
            lv_label_set_text(src->screen_label_4, lvgl_buffer);
        }
        if(times - adc_time == 25)
        {
            adc_time = times;
            uint8_t level = adc_dev.get_Batterylevel();
            snprintf(lvgl_buffer,28,"%d%%",level);
            lv_label_set_text(src->screen_label_8, lvgl_buffer);
            if(level < 4)
            {
                lv_label_set_text(src->screen_label_9, "OFF");
                lv_obj_invalidate(src->screen_label_9);
                vTaskDelay(pdMS_TO_TICKS(500));
                board_div.VBAT_POWER_OFF();
                board_div.POWEER_EPD_OFF();
                board_div.POWEER_Audio_OFF();
            }
        }
  	  	vTaskDelay(pdMS_TO_TICKS(200));
        times++;
    }
}

void sdcard_lvgl_task(void *srg)
{
    lv_ui *src = (lv_ui *)srg;
    char str_write[20] = {"waveshare.com"};
    char str_read[20] = {""};
    if(!sdcard_slot)
    {
        lv_label_set_text(src->screen_label_5, "No sd card");
    }
    else
    {
        s_example_write_file("/sdcard/sdcard.txt",str_write);
        s_example_read_file("/sdcard/sdcard.txt",str_read,NULL);
        if(!strcmp(str_write,str_read))
        {
            lv_label_set_text(src->screen_label_5, "passed");
        }
        else
        {
            lv_label_set_text(src->screen_label_5, "failed");
        }
    }
    vTaskDelete(NULL);
}

void example_scan_wifi_ble_task(void *srg)
{
    lv_ui *Send_ui = (lv_ui *)srg;
    char send_lvgl[10] = {""};
    uint8_t ble_scan_count = 0;
    uint8_t ble_mac[6];
    EventBits_t even = xEventGroupWaitBits(wifi_even_,0x02,pdTRUE,pdTRUE,pdMS_TO_TICKS(30000)); 
    espwifi_deinit(); //释放WIFI
    ble_scan_prepare();
    ble_stack_init();
    ble_scan_start();
    for(;xQueueReceive(ble_queue,ble_mac,3500) == pdTRUE;)
    {
        ble_scan_count++;
        if(ble_scan_count >= 20)
        break;
        vTaskDelay(pdMS_TO_TICKS(30));
    }
    if(get_bit_data(even,1))
    {
        snprintf(send_lvgl,9,"%d",user_esp_bsp.apNum);
        lv_label_set_text(Send_ui->screen_label_13, send_lvgl);
    }
    else
    {
        lv_label_set_text(Send_ui->screen_label_13, "P");
    }
    snprintf(send_lvgl,9,"%d",ble_scan_count);
    lv_label_set_text(Send_ui->screen_label_11, send_lvgl);
    ble_stack_deinit();//释放BLE
    vTaskDelete(NULL);
}

void button_boot_task(void *arg)
{
    uint8_t *data_ptr = (uint8_t *)heap_caps_malloc(1024 * 100 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);
    audio_play_init();
    for(;;)
    {
        EventBits_t even = xEventGroupWaitBits(boot_groups,set_bit_all,pdTRUE,pdFALSE,pdMS_TO_TICKS(2 * 1000));
        if(get_bit_button(even,2))
        {
            
        }
        else if(get_bit_button(even,1))
        {
            
        }
        else if(get_bit_button(even,0))
        {
            if(audio_flag)
            {
                vTaskDelay(pdMS_TO_TICKS(300));
                audio_playback_read(data_ptr,1024 * 100);
                audio_playback_write(data_ptr,1024 * 100);
            }
        }
    }
}

//void touch_test_task(void *pvParameters)
//{
//    for(;;)
//    {
//        uint16_t _x = 0,_y = 0;
//        uint8_t win = 0;
//        win = touch_dev->getTouch(&_x,&_y);
//        if(win)
//        {
//            ESP_LOGI("touch","(%d,%d)",_x,_y);
//        }
//        vTaskDelay(pdMS_TO_TICKS(100));
//    }
//}

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

void user_ui_init(void)
{
    setup_ui(&src_ui);
    xTaskCreatePinnedToCore(button_power_task, "button_power_task", 4 * 1024, (void *)&src_ui, 2, NULL,1);
    xTaskCreatePinnedToCore(user_lvgl_task, "user_lvgl_task", 4 * 1024, (void *)&src_ui, 2, NULL,1);
    xTaskCreatePinnedToCore(power_Testing_task, "power_Testing_task", 3 * 1024, NULL, 2, NULL,1);      //检测
    xTaskCreatePinnedToCore(sdcard_lvgl_task, "sdcard_lvgl_task", 4 * 1024, (void *)&src_ui, 2, NULL,1);
    xTaskCreatePinnedToCore(example_scan_wifi_ble_task, "example_scan_wifi_ble_task", 4 * 1024, (void *)&src_ui, 2, NULL,1);
    xTaskCreatePinnedToCore(button_boot_task, "button_boot_task", 4 * 1024, NULL, 4, NULL,1);
    xTaskCreatePinnedToCore(led_test_task, "led_test_task", 4 * 1024, NULL, 4, NULL,1);
    //xTaskCreate(touch_test_task, "touch_test_task", 3 * 1024,NULL, 5, NULL);
}