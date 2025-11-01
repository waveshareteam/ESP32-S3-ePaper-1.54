idf_component_register(
  SRCS "sdcard_bsp.c"
  PRIV_REQUIRES fatfs sdmmc   
  INCLUDE_DIRS "./")
#REQUIRES fatfs
#PRIV_REQUIRES