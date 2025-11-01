#ifndef TOUCH_BSP_H
#define TOUCH_BSP_H

#include "driver/i2c_master.h"

class touch_bsp
{
private:
    i2c_master_bus_handle_t bus;
    i2c_master_dev_handle_t dev;
    const int port_num;
    const int dev_addr;
    const int H_RES;
    const int V_RES;
public:
    touch_bsp(int _port_num,int _dev_addr,int _H_RES,int _V_RES);
    ~touch_bsp();

    uint8_t getTouch(uint16_t *x,uint16_t *y);
};

#endif
