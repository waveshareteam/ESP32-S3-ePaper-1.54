#ifndef SDCARD_BSP_H
#define SDCARD_BSP_H

void sdcard_init(void);
float sdcard_GetValue(void);
esp_err_t s_example_write_file(const char *path, char *data);
esp_err_t s_example_read_file(const char *path,char *pxbuf,uint32_t *outLen);
#endif