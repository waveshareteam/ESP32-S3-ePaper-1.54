#ifndef BUTTON_BSP_H
#define BUTTON_BSP_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#ifdef __cplusplus
extern "C" {
#endif

extern EventGroupHandle_t boot_groups;
extern EventGroupHandle_t pwr_groups;
#define set_bit_button(x) ((uint32_t)(0x01)<<(x))
#define get_bit_button(x,y) (((uint32_t)(x)>>(y)) & 0x01)
#define set_bit_all 0x00ffffff                   //最高24bit


//set bit
#define set_bit_data(x,y) (x |= (0x01<<y))
#define clr_bit_data(x,y) (x &= ~(0x01<<y))
#define get_bit_data(x,y) ((x>>y) & 0x01)
#define rset_bit_data(x) ((uint32_t)0x01<<(x))

void user_button_init(void);
uint8_t user_button_get_repeat_count(void);
uint8_t user_boot_get_repeat_count(void);


#ifdef __cplusplus
}
#endif
#endif