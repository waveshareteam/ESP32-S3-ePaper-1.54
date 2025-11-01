idf_component_register(
  SRCS "esp_wifi_bsp.c"

  PRIV_REQUIRES
  esp_wifi
  esp_event
  nvs_flash
  
  INCLUDE_DIRS "./")
