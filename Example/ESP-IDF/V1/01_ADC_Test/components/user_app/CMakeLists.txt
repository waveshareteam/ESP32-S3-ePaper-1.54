idf_component_register(
  SRCS "user_app.c"
  PRIV_REQUIRES adc_bsp esp_app_format app_update
  INCLUDE_DIRS "./")
