#ifndef I2C_EQUIPMENT_H
#define I2C_EQUIPMENT_H

#include "SensorPCF85063.hpp"

typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t week;
}RtcDateTime_t;

class i2c_equipment
{
private:
	SensorPCF85063 rtc;
	RtcDateTime_t time;
public:
  	i2c_equipment();
  	~i2c_equipment();

  	void set_rtcTime(uint16_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second);
	RtcDateTime_t get_rtcTime();
};


typedef enum{
  NO_ERROR       = 0x00, // no error
  ACK_ERROR      = 0x01, // no acknowledgment error
  CHECKSUM_ERROR = 0x02 // checksum mismatch error
}etError;

typedef enum{
  READ_ID            = 0xEFC8, // command: read ID register
  SOFT_RESET         = 0x805D, // soft reset
  SLEEP              = 0xB098, // sleep
  WAKEUP             = 0x3517, // wakeup
  MEAS_T_RH_POLLING  = 0x7866, // meas. read T first, clock stretching disabled
  MEAS_T_RH_CLOCKSTR = 0x7CA2, // meas. read T first, clock stretching enabled
  MEAS_RH_T_POLLING  = 0x58E0, // meas. read RH first, clock stretching disabled
  MEAS_RH_T_CLOCKSTR = 0x5C24  // meas. read RH first, clock stretching enabled
}etCommands;

typedef struct 
{
  float Temp;
  float RH;
}shtc3_data_t;

class i2c_equipment_shtc3
{
private:
	uint16_t shtc3_id = 0x00;
	const uint16_t CRC_POLYNOMIAL = 0x131;
	const uint8_t  SHTC3_PETP_NUM = 4;

	etError SHTC3_GetId();
	etError SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,uint8_t checksum);
	etError SHTC3_GetTempAndHumiPolling(float *temp, float *humi);
	float SHTC3_CalcTemperature(uint16_t rawValue);
	float SHTC3_CalcHumidity(uint16_t rawValue);

public:
	i2c_equipment_shtc3();
	~i2c_equipment_shtc3();

	etError shtc3_Wakeup();
	etError shtc3_Sleep();
	etError shtc3_SoftReset();
	uint16_t get_Shtc3Id();
	shtc3_data_t readTempHumi();
};

#endif 
