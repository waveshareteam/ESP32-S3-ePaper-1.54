idf_component_register(
  SRCS "epaper_driver_bsp.cpp"
  PRIV_REQUIRES
  driver
  REQUIRES
  driver
  fonts
  INCLUDE_DIRS "./")
