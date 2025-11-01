idf_component_register(
  SRCS "i2c_equipment.cpp"
  PRIV_REQUIRES SensorLib i2c_bsp
  INCLUDE_DIRS "./")
