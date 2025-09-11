idf_component_register(
  SRCS "user_app.cpp"
  PRIV_REQUIRES
  board_power_bsp
  driver
  main
  ui_bsp
  button_bsp
  REQUIRES
  epaper_driver_bsp
  sdcard_bsp
  i2c_equipment
  i2c_bsp
  adc_bsp
  esp_wifi_bsp
  ble_scan_bsp
  audio_bsp
  touch_bsp
  INCLUDE_DIRS ".")
