#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "user_config.h"
#include "lvgl.h"

#include "user_app.h"


/*lvgl port   以下均是lvgl的初始化代码,可以忽略不看 The following are all the initialization codes for lvgl. You can ignore them.*/
static const char *TAG = "main_1_54";
static SemaphoreHandle_t lvgl_mux = NULL;

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
#define BUFF_SIZE (EPD_WIDTH * EPD_HEIGHT * BYTES_PER_PIXEL)

/*lvgl tset unlock*/
static bool example_lvgl_lock(int timeout_ms);
static void example_lvgl_unlock(void);
static void example_lvgl_port_task(void *arg);


static void example_lvgl_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
  uint16_t *buffer = (uint16_t *)color_p;
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
	lv_disp_flush_ready(disp);
}

static void example_increase_lvgl_tick(void *arg)
{
  lv_tick_inc(EXAMPLE_LVGL_TICK_PERIOD_MS);
}

void lvgl_port(void)
{
  lv_init();
  lv_display_t * disp = lv_display_create(EPD_WIDTH, EPD_HEIGHT); /* 以水平和垂直分辨率（像素）进行基本初始化 */
  lv_display_set_flush_cb(disp, example_lvgl_flush_cb);
  uint8_t *buffer_1 = NULL;
  buffer_1 = (uint8_t *)heap_caps_malloc(BUFF_SIZE, MALLOC_CAP_SPIRAM);
  assert(buffer_1);
  lv_display_set_buffers(disp, buffer_1, NULL, BUFF_SIZE, LV_DISPLAY_RENDER_MODE_FULL);
  
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
