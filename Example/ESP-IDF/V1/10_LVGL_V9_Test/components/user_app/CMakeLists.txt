idf_component_register(
  SRCS "user_app.cpp"
  PRIV_REQUIRES
  board_power_bsp
  driver
  main
  ui_bsp
  REQUIRES
  epaper_driver_bsp
  INCLUDE_DIRS ".")
