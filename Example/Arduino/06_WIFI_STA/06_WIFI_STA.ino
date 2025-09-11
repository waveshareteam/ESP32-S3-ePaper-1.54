#include <WiFi.h>

const char* ssid = "K2P";          // Change to your WiFi name
const char* password = "1234567890";  // Change to your WiFi password


static void gpio_init(void)
{
  gpio_config_t gpio_conf = {};
  gpio_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_conf.mode = GPIO_MODE_OUTPUT;
  gpio_conf.pin_bit_mask = 0x1ULL << GPIO_NUM_8;
  gpio_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_conf.pull_up_en = GPIO_PULLUP_ENABLE;

  ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_config(&gpio_conf));

  gpio_set_level(GPIO_NUM_8,true);
}


void setup()
{
  gpio_init();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set to STA mode
  WiFi.begin(ssid, password);
  
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP Address:");
  Serial.println(WiFi.localIP().toString());
  //Serial.printf("IP Address: %s\n",WiFi.localIP().toString().c_str());
}

void loop()
{
  // Your code
}
