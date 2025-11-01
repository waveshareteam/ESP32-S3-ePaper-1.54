#include "button_bsp.h"
#include "multi_button.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "user_config.h"

EventGroupHandle_t boot_groups;
EventGroupHandle_t pwr_groups;

static Button button1;                  //申请按键
#define USER_KEY_1 BOOT_BUTTON_PIN      //实际的GPIO
#define button1_id 1                    //按键的ID
#define button1_active 0                //有效电平


static Button button2;                  //申请按键
#define USER_KEY_2 PWR_BUTTON_PIN       //实际的GPIO
#define button2_id 2                    //按键的ID
#define button2_active 0                //有效电平

/*******************回调事件声明***************/
static void on_boot_single_click(Button* btn_handle);
static void on_boot_longpress_press(Button* btn_handle);
static void on_boot_pressup_press(Button* btn_handle);

static void on_pwr_single_click(Button* btn_handle);
static void on_pwr_double_press(Button* btn_handle);
static void on_pwr_longpress_press(Button* btn_handle);
static void on_pwr_pressup_press(Button* btn_handle);
/*********************************************/

static void clock_task_callback(void *arg)
{
  	button_ticks();              //状态回调
}
static uint8_t read_button_GPIO(uint8_t button_id)   //返回GPIO电平
{
	switch (button_id)
  	{
  	  	case button1_id:
  	  	  	return gpio_get_level(USER_KEY_1);
  	  	case button2_id:
  	  	  	return gpio_get_level(USER_KEY_2);
  	  	default:
  	  	  	break;
  	}
  	return 1;
}

static void gpio_init(void)
{
  	gpio_config_t gpio_conf = {};
  	gpio_conf.intr_type = GPIO_INTR_DISABLE;
  	gpio_conf.mode = GPIO_MODE_INPUT;
  	gpio_conf.pin_bit_mask = (0x1ULL<<USER_KEY_1) | (0x1ULL<<USER_KEY_2);
  	gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  	gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  	ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));
}

void user_button_init(void)
{
  	boot_groups = xEventGroupCreate();
  	pwr_groups = xEventGroupCreate();
  	gpio_init();

  	button_init(&button1, read_button_GPIO, button1_active , button1_id);       	// 初始化 初始化对象 回调函数 触发电平 按键ID
  	button_attach(&button1,BTN_SINGLE_CLICK,on_boot_single_click);            	    //单击事件             
	button_attach(&button2,BTN_LONG_PRESS_START,on_boot_longpress_press);            	//长按事件
	button_attach(&button2,BTN_PRESS_UP,on_boot_pressup_press);            	            //弹起事件

  	button_init(&button2, read_button_GPIO, button2_active , button2_id);       	    // 初始化 初始化对象 回调函数 触发电平 按键ID
  	button_attach(&button2,BTN_SINGLE_CLICK,on_pwr_single_click);            		    //单击事件
	button_attach(&button2,BTN_DOUBLE_CLICK,on_pwr_double_press);            		    //双击事件
	button_attach(&button2,BTN_LONG_PRESS_START,on_pwr_longpress_press);            	//长按事件
	button_attach(&button2,BTN_PRESS_UP,on_pwr_pressup_press);            	            //弹起事件

  	esp_timer_create_args_t clock_tick_timer_args = {};
  	  	clock_tick_timer_args.callback = &clock_task_callback;
  	  	clock_tick_timer_args.name = "clock_task";
  	  	clock_tick_timer_args.arg = NULL;
  	esp_timer_handle_t clock_tick_timer = NULL;
  	ESP_ERROR_CHECK(esp_timer_create(&clock_tick_timer_args, &clock_tick_timer));
  	ESP_ERROR_CHECK(esp_timer_start_periodic(clock_tick_timer, 1000 * 5));  //5ms
  	button_start(&button2); //启动按键
  	button_start(&button1); //启动按键
}


/*事件函数*/
/*单击*/
static void on_boot_single_click(Button* btn_handle)
{
  	xEventGroupSetBits(boot_groups,set_bit_button(0));
}

static void on_pwr_single_click(Button* btn_handle)
{
  	xEventGroupSetBits(pwr_groups,set_bit_button(0));
}

static void on_pwr_double_press(Button* btn_handle)
{
	xEventGroupSetBits(pwr_groups,set_bit_button(1));
}

static void on_pwr_longpress_press(Button* btn_handle)
{
	xEventGroupSetBits(pwr_groups,set_bit_button(2));
}

static void on_pwr_pressup_press(Button* btn_handle)
{
	xEventGroupSetBits(pwr_groups,set_bit_button(3));
}

static void on_boot_longpress_press(Button* btn_handle)
{
	xEventGroupSetBits(boot_groups,set_bit_button(1));
}
static void on_boot_pressup_press(Button* btn_handle)
{
	xEventGroupSetBits(boot_groups,set_bit_button(2));
}



/*其他封装函数*/
uint8_t user_button_get_repeat_count(void)
{
  	return (button_get_repeat_count(&button2));
}

uint8_t user_boot_get_repeat_count(void)
{
  	return (button_get_repeat_count(&button1));
}

/*
事件:
SINGLE_CLICK :单击
DOUBLE_CLICK :双击
PRESS_DOWN :按下
PRESS_UP :弹起事件
PRESS_REPEAT :重复按下
LONG_PRESS_START :长按触发一次
LONG_PRESS_HOLD :长按一直触发
*/
