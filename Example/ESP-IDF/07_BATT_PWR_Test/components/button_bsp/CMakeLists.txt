idf_component_register(
  SRCS "multi_button.c" "button_bsp.c"
  PRIV_REQUIRES esp_timer driver main
  INCLUDE_DIRS "./")
