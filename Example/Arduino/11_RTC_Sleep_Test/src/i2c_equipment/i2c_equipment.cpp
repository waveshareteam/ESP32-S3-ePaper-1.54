#include <stdio.h>
#include "i2c_equipment.h"
#include "../i2c_dev/i2c_bsp.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "rtc";

static bool rtc_Callback(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len, bool writeReg, bool isWrite)
{
  	uint8_t ret;
  	int areg = reg;
  	if(isWrite) // 写寄存器
  	{
  	  	if(writeReg)
  	  	{
  	  	  	ret = i2c_write_buff(rtc_dev_handle,areg,buf,len);
  	  	}
  	  	else
  	  	{
  	  	  	ret = i2c_write_buff(rtc_dev_handle,-1,buf,len);
  	  	}
  	}
  	else
  	{
  	  	if(writeReg)
  	  	{
  	  	  	ret = i2c_read_buff(rtc_dev_handle,areg,buf,len);
  	  	}
  	  	else
  	  	{
  	  	  	ret = i2c_read_buff(rtc_dev_handle,-1,buf,len);
  	  	}
  	}
  	return (ret == ESP_OK) ? true : false;
}

void i2c_equipment::gpio_isr_handler(void *arg)
{
	i2c_equipment *dev = (i2c_equipment *)arg;
	assert(dev);
    // 设置事件位（ISR-safe）
    xEventGroupSetBitsFromISR(dev->gpio_event_group,(0x01UL << GPIO_NUM_5),NULL);
}

EventGroupHandle_t i2c_equipment::get_rtcGroupHandle() 
{
	return gpio_event_group;
}
/*
uint16_t year = 2023;
uint8_t month = 9;
uint8_t day = 7;
uint8_t hour = 11;
uint8_t minute = 24;
uint8_t second = 30;
*/
void i2c_equipment::set_rtcTime(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
  	setDateTime(year, month, day, hour, minute, second);
}

void i2c_equipment::set_rtcAlarmSec(int sec) 
{
	setDateTime(2025, 8, 8, 8, 0, 0); //2025,8,8 8:00:00
	if(sec <= 60) {
		setAlarmBySecond(sec);
	} else {
		ESP_LOGE(TAG,"set alarm Sec fill :%d",sec);
	}
}

void i2c_equipment::set_rtcAlarmMinute(int min) 
{
	setDateTime(2025, 8, 8, 8, 0, 0); //2024,8,8 8:00:00
	if(min <= 60) {
		setAlarmByMinutes(min);
	} else {
		ESP_LOGE(TAG,"set alarm Minutes fill :%d",min);
	}
}

void i2c_equipment::set_rtcAlarmHour(int hour) 
{
	setDateTime(2025, 8, 8, 8, 0, 0); //2024,8,8 8:00:00
	if(hour <= 24) {
		setAlarmByHours(hour);
	} else {
		ESP_LOGE(TAG,"set alarm Hour fill :%d",hour);
	}
}

void i2c_equipment::set_rtcAlarmDay(int day) 
{
	setDateTime(2025, 8, 8, 8, 0, 0); //2024,8,8 8:00:00
	if(day <= 31) {   //只是测试使用,具体每个月的天数是不一样的
		setAlarmByDays(day);
	} else {
		ESP_LOGE(TAG,"set alarm day fill :%d",day);
	}
}



RtcDateTime_t i2c_equipment::get_rtcTime()
{
  	RTC_DateTime datetime = getDateTime();
  	time.year = datetime.getYear();
  	time.month = datetime.getMonth();
  	time.day = datetime.getDay();
  	time.hour = datetime.getHour();
  	time.minute = datetime.getMinute();
  	time.second = datetime.getSecond();
  	time.week = datetime.getWeek();
	return time;
}

i2c_equipment::i2c_equipment() {
	int err = (begin(rtc_Callback) == 1) ? ESP_OK : ESP_FAIL;
	ESP_ERROR_CHECK(err);

	gpio_event_group = xEventGroupCreate();     // 创建事件组
	// 配置 GPIO5
    gpio_config_t io_conf = {};
        io_conf.intr_type = GPIO_INTR_NEGEDGE;   // 下降沿中断
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = 1ULL << GPIO_NUM_5;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
	// 安装 ISR 服务
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_5, gpio_isr_handler, this);
}

i2c_equipment::~i2c_equipment()
{

}






i2c_equipment_shtc3::i2c_equipment_shtc3()
{
	shtc3_Wakeup();
	shtc3_SoftReset();
	vTaskDelay(pdMS_TO_TICKS(20));  //20MS
	SHTC3_GetId();
	ESP_LOGI("shtc3","ID:%04x",shtc3_id);
}

i2c_equipment_shtc3::~i2c_equipment_shtc3()
{
}

etError i2c_equipment_shtc3::SHTC3_GetId()
{
	uint8_t senBuf[2] = {(READ_ID>>8),(READ_ID&0xff)};
  	uint8_t readBuf[3] = {0,0,0};
  	int err = i2c_master_write_read_dev(shtc3_handle,senBuf,2,readBuf,3);
  	etError error = (err==ESP_OK) ? NO_ERROR : ACK_ERROR;
  	if(error != NO_ERROR)
  	{ESP_LOGE("shtc3","GetId WRITE Failure");return error;}
  	error = SHTC3_CheckCrc(readBuf,2,readBuf[2]);
  	if(error != NO_ERROR)
  	{ESP_LOGE("shtc3","GetId CRC Failure");return error;}
  	shtc3_id = ((readBuf[0] << 8) | readBuf[1]);
  	return error;
}

uint16_t i2c_equipment_shtc3::get_Shtc3Id()
{
	return shtc3_id;
}

// wake up the sensor from sleep mode
etError i2c_equipment_shtc3::shtc3_Wakeup()
{
	uint8_t senBuf[2] = {(WAKEUP>>8),(WAKEUP&0xff)};
  	int err = i2c_write_buff(shtc3_handle,-1,senBuf,2);
  	etError error = (err==ESP_OK) ? NO_ERROR : ACK_ERROR;
  	//esp_rom_delay_us(100); //100us
  	vTaskDelay(pdMS_TO_TICKS(50));  //50MS
  	if(error != NO_ERROR)
  	ESP_LOGE("shtc3","Wakeup Failure");
  	return error;
}

etError i2c_equipment_shtc3::shtc3_SoftReset()
{
  	uint8_t senBuf[2] = {(SOFT_RESET>>8),(SOFT_RESET&0xff)};
  	int err = i2c_write_buff(shtc3_handle,-1,senBuf,2);
  	etError error = (err==ESP_OK) ? NO_ERROR : ACK_ERROR;
  	if(error != NO_ERROR)
  	ESP_LOGE("shtc3","SoftReset Failure");
  	return error;
}

etError i2c_equipment_shtc3::SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,uint8_t checksum)
{
  	uint8_t bit;        // bit mask
  	uint8_t crc = 0xFF; // calculated checksum
  	uint8_t byteCtr;    // byte counter

  	// calculates 8-Bit checksum with given polynomial
  	for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
  	{
  	  	crc ^= (data[byteCtr]);
  	  	for(bit = 8; bit > 0; --bit) {
  	  	  	if(crc & 0x80) {
  	  	  	  	crc = (crc << 1) ^ CRC_POLYNOMIAL;
  	  	  	} else {
  	  	  	  	crc = (crc << 1);
  	  	  	}
  	  	}
  	}

  	// verify checksum
  	if(crc != checksum)
  	{
  	  	return CHECKSUM_ERROR;
  	}
  	else
  	{
  	  	return NO_ERROR;
  	}
}

float i2c_equipment_shtc3::SHTC3_CalcTemperature(uint16_t rawValue)
{
  // calculate temperature [°C]
  // T = -45 + 175 * rawValue / 2^16
  return 175 * (float)rawValue / 65536.0f - 45.0f - SHTC3_PETP_NUM;
}

float i2c_equipment_shtc3::SHTC3_CalcHumidity(uint16_t rawValue)
{
  // calculate relative humidity [%RH]
  // RH = rawValue / 2^16 * 100
  return 100 * (float)rawValue / 65536.0f;
}

etError i2c_equipment_shtc3::SHTC3_GetTempAndHumiPolling(float *temp, float *humi)
{
  	int err = 0;
  	etError  error;           // error code
  	uint16_t rawValueTemp;    // temperature raw value from sensor
  	uint16_t rawValueHumi;    // humidity raw value from sensor
  	uint8_t bytes[6] = {0};;
  	uint8_t senBuf[2] = {(MEAS_T_RH_POLLING>>8),(MEAS_T_RH_POLLING&0xff)};
  	err = i2c_write_buff(shtc3_handle,-1,senBuf,2);
  	error = (err==ESP_OK) ? NO_ERROR : ACK_ERROR;
  	if(error != NO_ERROR)
  	{ESP_LOGE("shtc3","GetTempAndHumi WRITE Failure");return error;}
	
  	vTaskDelay(pdMS_TO_TICKS(20));

  	// if no error, read temperature and humidity raw values
  	err = i2c_read_buff(shtc3_handle,-1,bytes,6);
  	error = (err == ESP_OK) ? NO_ERROR : ACK_ERROR;
  	if(error != NO_ERROR)
  	{ESP_LOGE("shtc3","GetTempAndHumi READ Failure"); return error;}
  	error = SHTC3_CheckCrc(bytes, 2, bytes[2]);
  	if(error != NO_ERROR)
  	{ESP_LOGE("shtc3","GetTempAndHumi TempCRC Failure"); return error;}
  	error = SHTC3_CheckCrc(&bytes[3], 2, bytes[5]);
  	if(error != NO_ERROR)
  	{ESP_LOGE("shtc3","GetTempAndHumi humidityCRC Failure"); return error;}
  	// if no error, calculate temperature in °C and humidity in %RH
  	rawValueTemp = (bytes[0] << 8) | bytes[1];
  	rawValueHumi = (bytes[3] << 8) | bytes[4];
  	*temp = SHTC3_CalcTemperature(rawValueTemp);
  	*humi = SHTC3_CalcHumidity(rawValueHumi);
  	return error;
}

etError i2c_equipment_shtc3::shtc3_Sleep()
{
  	uint8_t senBuf[2] = {(SLEEP>>8),(SLEEP&0xff)};
  	int err = i2c_write_buff(shtc3_handle,-1,senBuf,2);
  	etError error = (err==ESP_OK) ? NO_ERROR : ACK_ERROR;
  	if(error != NO_ERROR)
  	ESP_LOGE("shtc3","Sleep Failure");
  	return error;
}

shtc3_data_t i2c_equipment_shtc3::readTempHumi()
{
	etError  error;
  	float temperature;
  	float humidity;
  	shtc3_data_t shtc3_data;
  	shtc3_Wakeup();
  	error = SHTC3_GetTempAndHumiPolling(&temperature, &humidity);
  	if(error != NO_ERROR)
  	{
  	  ESP_LOGE("shtc3","error:%d",error);
  	}
  	else
  	{
  	  shtc3_data.RH = humidity;
  	  shtc3_data.Temp = temperature;
  	}
  	shtc3_Sleep();
  	return shtc3_data;
}
