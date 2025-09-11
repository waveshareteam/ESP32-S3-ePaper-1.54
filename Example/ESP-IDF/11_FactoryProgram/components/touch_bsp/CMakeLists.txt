idf_component_register(
    SRCS "touch_bsp.cpp"
    PRIV_REQUIRES i2c_bsp driver
    INCLUDE_DIRS "./")
