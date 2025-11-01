#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "sdcard_bsp.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "sdcard";

#define PIN_NUM_MISO  (gpio_num_t)40
#define PIN_NUM_MOSI  (gpio_num_t)41
#define PIN_NUM_CLK   (gpio_num_t)39
#define SDlist "/sdcard"     //Directory, similar to a standard

sdmmc_card_t *card = NULL;   //handle

uint32_t sdcard_slot = 0;

void _sdcard_init(void)
{
  	esp_vfs_fat_sdmmc_mount_config_t mount_config = {};
    	mount_config.format_if_mount_failed = false;     //If the hook fails, create a partition table and format the SD car
    	mount_config.max_files = 5;                      //Maximum number of open files
    	mount_config.allocation_unit_size = 2 * 1024;         //Similar to sector size

  	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  		host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;//high speed

  	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  		slot_config.width = 1;           //1-wire
  		slot_config.clk = PIN_NUM_CLK;
  		slot_config.cmd = PIN_NUM_MOSI;
  		slot_config.d0 = PIN_NUM_MISO;
  	ESP_ERROR_CHECK_WITHOUT_ABORT(esp_vfs_fat_sdmmc_mount(SDlist, &host, &slot_config, &mount_config, &card));

  	if(card != NULL)
  	{
  	  	sdmmc_card_print_info(stdout, card); //Print out the card information
		sdcard_slot = 1;
  	  	//ESP_LOGI(TAG,"practical_size:%.2fG",(float)(card->csd.capacity)/2048/1024);//g
  	}
}
float sd_card_get_value(void)
{
  	if(card != NULL)
  	{
  	  	return (float)(card->csd.capacity)/2048/1024; //G
  	}
  	return 0;
}

/*write data
path:path
data:data
*/
esp_err_t s_example_write_file(const char *path, char *data)
{
  	esp_err_t err;
  	if(card == NULL)
  	{
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	err = sdmmc_get_status(card); //First check if there is an SD card
  	if(err != ESP_OK)
  	{
  	  	return err;
  	}
  	FILE *f = fopen(path, "w"); //Get path address
  	if(f == NULL)
  	{
  	  	ESP_LOGI(TAG,"path:Write Wrong path");
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	fprintf(f, data); //write in
  	fclose(f);
  	return ESP_OK;
}
/*
read data
path:path
*/
esp_err_t s_example_read_file(const char *path,char *pxbuf,uint32_t *outLen)
{
  	esp_err_t err;
  	if(card == NULL)
  	{
  	  	ESP_LOGI(TAG,"card == NULL");
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	err = sdmmc_get_status(card); //First check if there is an SD card
  	if(err != ESP_OK)
  	{
  	  	ESP_LOGI(TAG,"card == NO");
  	  	return err;
  	}
  	FILE *f = fopen(path, "rb");
  	if (f == NULL)
  	{
  	  	ESP_LOGI(TAG,"Read Wrong path");
  	  	return ESP_ERR_NOT_FOUND;
  	}
  	fseek(f, 0, SEEK_END);     //Move the pointer to the back
  	uint32_t unlen = ftell(f);
  	//fgets(pxbuf, unlen, f);  //Read text
  	fseek(f, 0, SEEK_SET);     //Move the pointer to the front
  	uint32_t poutLen = fread((void *)pxbuf,1,unlen,f);
  	if(outLen != NULL)
  	*outLen = poutLen;
  	fclose(f);
  	return ESP_OK;
}

/*
有偏移量读取SD卡数据
*/
uint32_t s_example_read_from_offset(const char *path, char *buffer, uint32_t len, uint32_t offset)
{
  	esp_err_t err;
  	if (card == NULL)
  	{
  	  	ESP_LOGE(TAG, "SD card not initialized (card == NULL)");
  	  	return 0;
  	}
  	err = sdmmc_get_status(card); // 检查 SD 卡是否存在
  	if (err != ESP_OK)
  	{
  	  	ESP_LOGE(TAG, "SD card status check failed (card not present or unresponsive)");
  	  	return 0;
  	}
  	FILE *f = fopen(path, "rb");
  	if (f == NULL)
  	{
  	  	ESP_LOGE(TAG, "Failed to open file: %s", path);
  	  	return 0;
  	}
  	// 移动文件指针到偏移位置
  	fseek(f, offset, SEEK_SET);
  	// 从文件中读取数据到 buffer
  	uint32_t bytesRead = fread((void *)buffer, 1, len, f);
  	fclose(f);
  	//ESP_LOGI(TAG, "Read %u bytes from file: %s (offset: %u)", bytesRead, path, offset);
  	return bytesRead;
}
/*
有偏移量写SD卡数据
*/
uint32_t s_example_wriet_from_offset(const char *path, char *buffer, uint32_t len, uint8_t mode)
{
  	esp_err_t err;
  	if (card == NULL)
  	{
  	  	ESP_LOGE(TAG, "SD card not initialized (card == NULL)");
  	  	return 0;
  	}
  	err = sdmmc_get_status(card); // 检查 SD 卡是否存在
  	if (err != ESP_OK)
  	{
  	  	ESP_LOGE(TAG, "SD card status check failed (card not present or unresponsive)");
  	  	return 0;
  	}
  	FILE *f = NULL;
  	if(mode == 0)    //清空数据
  	{
  	  	f = fopen(path, "w");
  	  	if (f == NULL)
  	  	{
  	    	ESP_LOGE(TAG, "Failed to open file: %s", path);
  	    	return 0;
  	  	}
  	  	fclose(f);
  	  	return 0;
  	}
  	else
  	{
  	  	f = fopen(path, "ab");
  	  	if (f == NULL)
  	  	{
  	  	  	ESP_LOGE(TAG, "Failed to open file: %s", path);
  	  	  	return 0;
  	  	}
  	  	// 从文件中读取数据到 buffer
  	  	uint32_t bytesRead = fwrite((void *)buffer, 1, len, f);
  	  	fclose(f);
  	  	return bytesRead;
  	}
}
