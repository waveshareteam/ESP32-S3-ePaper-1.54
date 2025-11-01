#include <stdio.h>
#include "user_app.h"

#include "driver/gpio.h"
#include "esp_io_expander_tca9554.h"
#include "driver/i2c_master.h"

static void gpio_init(void)
{
    esp_io_expander_handle_t io_expander = NULL;
    i2c_master_bus_handle_t tca9554_i2c_bus_ = NULL;
    i2c_master_bus_config_t i2c_bus_config = {};
        i2c_bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
        i2c_bus_config.i2c_port = I2C_NUM_0;
        i2c_bus_config.scl_io_num = GPIO_NUM_48;
        i2c_bus_config.sda_io_num = GPIO_NUM_47;
        i2c_bus_config.glitch_ignore_cnt = 7;
        i2c_bus_config.flags.enable_internal_pullup = true;
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &tca9554_i2c_bus_));
  	esp_io_expander_new_i2c_tca9554(tca9554_i2c_bus_, ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000, &io_expander);
	esp_io_expander_set_dir(io_expander, IO_EXPANDER_PIN_NUM_1, IO_EXPANDER_OUTPUT);
  	esp_io_expander_set_level(io_expander, IO_EXPANDER_PIN_NUM_1, 0);
}

void app_main(void)
{
    gpio_init();
    user_top_init();
}
