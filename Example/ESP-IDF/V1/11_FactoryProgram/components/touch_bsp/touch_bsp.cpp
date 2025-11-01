#include <stdio.h>
#include "touch_bsp.h"
#include "i2c_bsp.h"

touch_bsp::touch_bsp(int _port_num,int _dev_addr,int _H_RES,int _V_RES) : port_num(_port_num) , 
    dev_addr(_dev_addr),
    H_RES(_H_RES),
    V_RES(_V_RES)
{
    ESP_ERROR_CHECK(i2c_master_get_bus_handle((i2c_port_num_t)port_num, &bus));
    i2c_device_config_t dev_cfg = {};
    	dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    	dev_cfg.device_address = dev_addr;
    	dev_cfg.scl_speed_hz = 300000;
  	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &dev));
}

touch_bsp::~touch_bsp()
{

}

uint8_t touch_bsp::getTouch(uint16_t *x,uint16_t *y)
{
    uint8_t data = 0;
    uint8_t buf[4];
    i2c_read_buff(dev,0x02,&data,1);
    if(data)
    {
        i2c_read_buff(dev,0x03,buf,4);
        *x = (((uint16_t)buf[0] & 0x0f)<<8) | (uint16_t)buf[1];
        *y = (((uint16_t)buf[2] & 0x0f)<<8) | (uint16_t)buf[3];
        if(*x > H_RES)
        *x = H_RES;
        if(*y > V_RES)
        *y = V_RES;
        return 1;
    }
    return 0;
}