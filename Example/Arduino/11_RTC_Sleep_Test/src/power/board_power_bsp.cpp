#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "board_power_bsp.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"

board_power_bsp_t::board_power_bsp_t(uint8_t _epd_power_pin,uint8_t _audio_power_pin,uint8_t _vbat_power_pin) :
    epd_power_pin(_epd_power_pin),
    audio_power_pin(_audio_power_pin),
    vbat_power_pin(_vbat_power_pin) {
    ESP_ERROR_CHECK(gpio_reset_pin((gpio_num_t)epd_power_pin));
    ESP_ERROR_CHECK(gpio_reset_pin((gpio_num_t)audio_power_pin));
    ESP_ERROR_CHECK(gpio_reset_pin((gpio_num_t)vbat_power_pin));

    gpio_config_t gpio_conf = {};                                                            
        gpio_conf.intr_type = GPIO_INTR_DISABLE;                                             
        gpio_conf.mode = GPIO_MODE_OUTPUT;                                                   
        gpio_conf.pin_bit_mask = (0x1ULL << epd_power_pin) | (0x1ULL << audio_power_pin) | (0x1ULL << vbat_power_pin);
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;                                      
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;                                           
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));

    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_init((gpio_num_t)vbat_power_pin)); //初始化为RTC  io
    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction((gpio_num_t)vbat_power_pin,RTC_GPIO_MODE_OUTPUT_ONLY));
    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pullup_en((gpio_num_t)vbat_power_pin));
    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_pulldown_dis((gpio_num_t)vbat_power_pin));   
    //
    ///*rtc sleep init*/
    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_direction_in_sleep((gpio_num_t)vbat_power_pin,RTC_GPIO_MODE_OUTPUT_ONLY));
    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_set_drive_capability((gpio_num_t)vbat_power_pin, GPIO_DRIVE_CAP_DEFAULT));
    //ESP_ERROR_CHECK_WITHOUT_ABORT(rtc_gpio_hold_en((gpio_num_t)vbat_power_pin));
}

board_power_bsp_t::~board_power_bsp_t() {

}

void board_power_bsp_t::POWEER_EPD_ON() {
    gpio_set_level((gpio_num_t)epd_power_pin,0);
}

void board_power_bsp_t::POWEER_EPD_OFF() {
    gpio_set_level((gpio_num_t)epd_power_pin,1);
}

void board_power_bsp_t::POWEER_Audio_ON() {
    gpio_set_level((gpio_num_t)audio_power_pin,0);
}

void board_power_bsp_t::POWEER_Audio_OFF() {
    gpio_set_level((gpio_num_t)audio_power_pin,1);
}

void board_power_bsp_t::VBAT_POWER_ON() {
    gpio_set_level((gpio_num_t)vbat_power_pin,1);
}

void board_power_bsp_t::VBAT_POWER_OFF() {
    gpio_set_level((gpio_num_t)vbat_power_pin,0);
}

void board_power_bsp_t::EnableDeepLowPowerMode() {
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_AUTO); // 禁用电源域,更加低功耗,
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL); // 禁用所有的唤醒源
    const uint64_t ext_wakeup_boot_mask = 1ULL << GPIO_NUM_0; //BOOT唤醒
    const uint64_t ext_wakeup_rtc_mask = 1ULL << GPIO_NUM_5;  //RTC唤醒
    const uint64_t ext_wakeup_pwr_mask = 1ULL << GPIO_NUM_18;  //PWR唤醒
    ESP_ERROR_CHECK(esp_sleep_enable_ext1_wakeup_io(ext_wakeup_pwr_mask | ext_wakeup_rtc_mask | ext_wakeup_boot_mask, ESP_EXT1_WAKEUP_ANY_LOW)); //使能BOOT按键唤醒
    ESP_ERROR_CHECK(rtc_gpio_pulldown_dis(GPIO_NUM_5));
    ESP_ERROR_CHECK(rtc_gpio_pullup_en(GPIO_NUM_5));
    ESP_ERROR_CHECK(rtc_gpio_hold_en((gpio_num_t)vbat_power_pin));
    esp_deep_sleep_start();  //esp32进入低功耗
}
