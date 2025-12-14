#pragma once
#include "stm32f1xx_hal.h"

class I2cScan
{
public:
	static uint8_t blocking(I2C_HandleTypeDef *hi2c, uint8_t* resultAddresses, uint8_t maxSize)
	{
		uint8_t resultSize = 0;
		for(uint16_t i = 1; i <0x80; i++)
		{
			if(HAL_I2C_IsDeviceReady(hi2c, i << 1, 4, 100) == HAL_OK)
			{
				if(resultSize < maxSize)
					resultAddresses[resultSize] = i << 1;
				resultSize++;
			}
		}
		return resultSize;
	}
};
