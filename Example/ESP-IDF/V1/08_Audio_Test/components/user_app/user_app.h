#ifndef USER_APP_H
#define USER_APP_H

#include "epaper_driver_bsp.h"

extern epaper_driver_display *driver;

#ifdef __cplusplus
extern "C" {
#endif

void user_app_init(void);
void user_ui_init(void);

#ifdef __cplusplus
}
#endif

#endif