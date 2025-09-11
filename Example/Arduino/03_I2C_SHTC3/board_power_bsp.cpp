#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "board_power_bsp.h"

board_power_bsp_t::board_power_bsp_t(uint8_t _epd_power_pin,uint8_t _audio_power_pin,uint8_t _vbat_power_pin) :
    epd_power_pin(_epd_power_pin),
    audio_power_pin(_audio_power_pin),
    vbat_power_pin(_vbat_power_pin) {
    gpio_config_t gpio_conf = {};                                                            
        gpio_conf.intr_type = GPIO_INTR_DISABLE;                                             
        gpio_conf.mode = GPIO_MODE_OUTPUT;                                                   
        gpio_conf.pin_bit_mask = (0x1ULL << epd_power_pin) | (0x1ULL << audio_power_pin) | (0x1ULL << vbat_power_pin);
        gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;                                      
        gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;                                           
    ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));  
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
