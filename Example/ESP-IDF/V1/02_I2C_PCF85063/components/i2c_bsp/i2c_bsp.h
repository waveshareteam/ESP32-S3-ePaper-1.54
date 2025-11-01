#ifndef I2C_BSP_H
#define I2C_BSP_H

#include "driver/i2c_master.h"

extern i2c_master_dev_handle_t rtc_dev_handle;
extern i2c_master_dev_handle_t shtc3_handle;

#ifdef __cplusplus
extern "C" {
#endif

void i2c_master_Init(void);
int i2c_write_buff(i2c_master_dev_handle_t dev_handle,int reg,uint8_t *buf,uint8_t len);
int i2c_master_write_read_dev(i2c_master_dev_handle_t dev_handle,uint8_t *writeBuf,uint8_t writeLen,uint8_t *readBuf,uint8_t readLen);
int i2c_read_buff(i2c_master_dev_handle_t dev_handle,int reg,uint8_t *buf,uint8_t len);

#ifdef __cplusplus
}
#endif

#endif