#ifndef SDCARD_BSP_H
#define SDCARD_BSP_H
#include "driver/sdmmc_host.h"

extern sdmmc_card_t *card;
extern uint32_t sdcard_slot;
#ifdef __cplusplus
extern "C" {
#endif

void _sdcard_init(void);
esp_err_t s_example_read_file(const char *path,char *pxbuf,uint32_t *outLen);
esp_err_t s_example_write_file(const char *path, char *data);

uint32_t s_example_read_from_offset(const char *path, char *buffer, uint32_t len, uint32_t offset);
uint32_t s_example_wriet_from_offset(const char *path, char *buffer, uint32_t len, uint8_t mode);


#ifdef __cplusplus
}
#endif

#endif