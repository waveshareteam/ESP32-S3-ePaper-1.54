#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_config.h"
#include "lvgl.h"

#include "user_app.h"


/*lvgl port   以下均是lvgl的初始化代码,可以忽略不看 The following are all the initialization codes for lvgl. You can ignore them.*/
static const char *TAG = "main_1_54";
static SemaphoreHandle_t lvgl_mux = NULL;

/*lvgl tset unlock*/
static bool example_lvgl_lock(int timeout_ms);
static void example_lvgl_unlock(void);
static void example_lvgl_port_task(void *arg);

static void example_lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
  uint16_t *buffer = (uint16_t *)color_map;
  driver->EPD_Clear();
  for(int y = area->y1; y <= area->y2; y++) 
  {
   	for(int x = area->x1; x <= area->x2; x++)
   	{
   	  uint8_t color = (*buffer < 0x7fff) ? DRIVER_COLOR_BLACK : DRIVER_COLOR_WHITE;
   	  driver->EPD_DrawColorPixel(x,y,color);
   	  buffer++;
   	}
  }
  driver->EPD_DisplayPart();
	lv_disp_flush_ready(drv);
}

static void example_increase_lvgl_tick(void *arg)
{
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

static void lvgl_port(void)
{
  static lv_disp_draw_buf_t disp_buf; 		// contains internal graphic buffer(s) called draw buffer(s)
  static lv_disp_drv_t disp_drv;      		// contains callback functions

  lv_init();

  lv_color_t *buffer_1 = (lv_color_t *)heap_caps_malloc(LVGL_SPIRAM_BUFF_LEN , MALLOC_CAP_SPIRAM);
  lv_color_t *buffer_2 = (lv_color_t *)heap_caps_malloc(LVGL_SPIRAM_BUFF_LEN , MALLOC_CAP_SPIRAM);
  assert(buffer_1);
  assert(buffer_2);
  lv_disp_draw_buf_init(&disp_buf, buffer_1, buffer_2, EPD_WIDTH * EPD_HEIGHT);

	ESP_LOGI(TAG, "Register display driver to LVGL");
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = EPD_WIDTH;
  disp_drv.ver_res = EPD_HEIGHT;
  disp_drv.flush_cb = example_lvgl_flush_cb;
  disp_drv.draw_buf = &disp_buf;
  disp_drv.full_refresh = 1;          //full_refresh must be 1
  lv_disp_drv_register(&disp_drv);

  ESP_LOGI(TAG, "Install LVGL tick timer");
  esp_timer_create_args_t lvgl_tick_timer_args = {};
  lvgl_tick_timer_args.callback = &example_increase_lvgl_tick;
  lvgl_tick_timer_args.name = "lvgl_tick";
  esp_timer_handle_t lvgl_tick_timer = NULL;
  ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer,EXAMPLE_LVGL_TICK_PERIOD_MS * 1000));

  lvgl_mux = xSemaphoreCreateMutex();
  assert(lvgl_mux);
  xTaskCreatePinnedToCore(example_lvgl_port_task, "LVGL", 8 * 1024, NULL, 4, NULL,1);
  if(example_lvgl_lock(-1))
  {
    user_ui_init();
    example_lvgl_unlock();
  }
}


static bool example_lvgl_lock(int timeout_ms)
{
  const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xSemaphoreTake(lvgl_mux, timeout_ticks) == pdTRUE;       
}

static void example_lvgl_unlock(void)
{
  assert(lvgl_mux && "bsp_display_start must be called first");
  xSemaphoreGive(lvgl_mux);
}
static void example_lvgl_port_task(void *arg)
{
  uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
  for(;;)
  {
    if (example_lvgl_lock(-1)) 
    {
      task_delay_ms = lv_timer_handler();
      //Release the mutex
      example_lvgl_unlock();
    }
    if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS)
    {
      task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS)
    {
      task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
    }
    vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
  }
}




void setup()
{
  Serial.begin(115200);
  user_app_init();
  lvgl_port();
}

void loop() 
{
  
}
