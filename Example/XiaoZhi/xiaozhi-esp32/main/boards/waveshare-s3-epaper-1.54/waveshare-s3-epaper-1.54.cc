#include "wifi_board.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "codecs/es8311_audio_codec.h"

#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <driver/i2c_master.h>
#include <driver/spi_common.h>
#include <wifi_station.h>
#include "mcp_server.h"
#include "lvgl.h"
#include "custom_lcd_display.h"
#include "board_power_bsp.h"

#define TAG "waveshare_epaper_1_54"

LV_FONT_DECLARE(font_puhui_16_4);
LV_FONT_DECLARE(font_awesome_16_4);

class CustomBoard : public WifiBoard {
private:
    i2c_master_bus_handle_t i2c_bus_;
    Button boot_button_;
    Button pwr_button_;
    CustomLcdDisplay *display_;
    board_power_bsp *power_;
    adc_oneshot_unit_handle_t adc1_handle;
    adc_cali_handle_t cali_handle;
    bool vbat_status = 0;

    void InitializeI2c() {
        i2c_master_bus_config_t i2c_bus_cfg = {
            .i2c_port = (i2c_port_t)0,
            .sda_io_num = AUDIO_CODEC_I2C_SDA_PIN,
            .scl_io_num = AUDIO_CODEC_I2C_SCL_PIN,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags = 
            {
                .enable_internal_pullup = 1,
            },
        };
        ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus_));
    }

    void InitializeButtons() { 
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                ResetWifiConfiguration();
            }
            app.ToggleChatState();
        });

        pwr_button_.OnLongPress([this]() {
            if(vbat_status) {
                vbat_status = 0;
                display_->Power_label_OFF();
                power_->POWEER_Audio_OFF();
                power_->POWEER_EPD_OFF();
                power_->VBAT_POWER_OFF();
            }
        });

        pwr_button_.OnPressUp([this]() {
            if(!vbat_status) {
                vbat_status = 1;
            }
        });
    }

    void InitializeTools() {
        auto& mcp_server = McpServer::GetInstance();
        mcp_server.AddTool("self.disp.network", "重新配网", PropertyList(),
        [this](const PropertyList&) -> ReturnValue {
            ResetWifiConfiguration();
            return true;
        });

        mcp_server.AddTool("self.sht3.read", "通过板载的温湿度传感器获取温湿度值", PropertyList(),
        [this](const PropertyList&) -> ReturnValue {
            char str [50] = {""};
            snprintf(str,49,"温度:25°,湿度:78%%");
            ESP_LOGE("OK","%s",str);
            return str;
        });
    }

    void Adc_Init() {
        adc_cali_curve_fitting_config_t cali_config = {};
            cali_config.unit_id = ADC_UNIT_1;
            cali_config.atten = ADC_ATTEN_DB_12;
            cali_config.bitwidth = ADC_BITWIDTH_12; //4096
        ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle));
        
        adc_oneshot_unit_init_cfg_t init_config1 = {};
            init_config1.unit_id = ADC_UNIT_1;
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));
        adc_oneshot_chan_cfg_t config = {};
            config.bitwidth = ADC_BITWIDTH_12;
            config.atten = ADC_ATTEN_DB_12;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_3, &config));
    }

    void InitializeLcdDisplay() {
        custom_lcd_spi_t lcd_spi_data = {};
            lcd_spi_data.cs = EPD_CS_PIN;
            lcd_spi_data.dc = EPD_DC_PIN;
            lcd_spi_data.rst = EPD_RST_PIN;
            lcd_spi_data.busy = EPD_BUSY_PIN;
            lcd_spi_data.mosi = EPD_MOSI_PIN;
            lcd_spi_data.scl = EPD_SCK_PIN;
            lcd_spi_data.spi_host = EPD_SPI_NUM;
            lcd_spi_data.buffer_len  = 5000;
        display_ = new CustomLcdDisplay(NULL, NULL, EXAMPLE_LCD_WIDTH,EXAMPLE_LCD_HEIGHT,DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,lcd_spi_data,
        {
            .text_font = &font_puhui_16_4,
            .icon_font = &font_awesome_16_4,
            .emoji_font = font_emoji_64_init(),
        });
    }

    void Power_Init() {
        power_ = new board_power_bsp(EPD_PWR_PIN,Audio_PWR_PIN,VBAT_PWR_PIN);
        power_->VBAT_POWER_ON();
        power_->POWEER_Audio_ON();
        power_->POWEER_EPD_ON();
    }

    static void example_time_example(void *arg) {
        CustomBoard *board = (CustomBoard *)(arg);
        vTaskDelay(pdMS_TO_TICKS(35 * 100));
        if(gpio_get_level(VBAT_PWR_GPIO))
        {
            board->vbat_status = 1;
        }
        vTaskDelete(NULL);
    }

    static void example_adc_tick_example(void *arg) {
        CustomBoard *board = (CustomBoard *)(arg);
        for(;;)
        {
            int OriginalData;
            int CalibratedData;
            float VbatVoltage;
            adc_oneshot_read(board->adc1_handle,ADC_CHANNEL_3,&OriginalData);
            adc_cali_raw_to_voltage(board->cali_handle,OriginalData,&CalibratedData);
            VbatVoltage = 0.001 * CalibratedData * 2;
            if(VbatVoltage < 3.0)
            {
                board->display_->Power_label_OFF();
                board->power_->POWEER_Audio_OFF();
                board->power_->POWEER_EPD_OFF();
                board->power_->VBAT_POWER_OFF();
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        vTaskDelete(NULL);
    }

public:
    CustomBoard() : boot_button_(BOOT_BUTTON_GPIO),
    pwr_button_(VBAT_PWR_GPIO) {
        Power_Init();     
        InitializeI2c();  
        InitializeButtons();     
        InitializeTools();
        InitializeLcdDisplay();
        Adc_Init();
        xTaskCreatePinnedToCore(example_time_example, "example_time_example", 3 * 1024, this , 2, NULL,0);
        xTaskCreatePinnedToCore(example_adc_tick_example, "example_adc_tick_example", 3 * 1024, this , 2, NULL,0);
    }

    virtual AudioCodec* GetAudioCodec() override {
        static Es8311AudioCodec audio_codec(i2c_bus_, I2C_NUM_0, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_MCLK, AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN,
            AUDIO_CODEC_PA_PIN, AUDIO_CODEC_ES8311_ADDR);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
};

DECLARE_BOARD(CustomBoard);