#pragma once
#include "BME280_SensorAPI/bme280.h"

class BME280_Processor
{
	enum class STATE
	{
		needInitialize,
		gettingData,
	} state = STATE::needInitialize;
	bme280_dev bme280;
	uint16_t I2C_ADDR = BME280_I2C_ADDR_PRIM << 1;
	bme280_data comp_data;
	bool gotDataFlag = false;
public:
	BME280_Processor(bme280_read_fptr_t read, bme280_write_fptr_t write, bme280_delay_us_fptr_t delay_us)
	{
		bme280_dev* dev = &bme280;
		dev->read = read;
		dev->write = write;
		dev->intf = BME280_I2C_INTF;
		dev->intf_ptr = (void*)&I2C_ADDR;
		dev->delay_us = delay_us;
	}
	int8_t update()
	{
		int8_t res = BME280_OK;
		gotDataFlag = false;
		if(state == STATE::needInitialize)
		{
			res = initialize(&bme280);
			if(res == BME280_OK)
				state = STATE::gettingData;
		}
		else
		{
			res = getData(&bme280, gotDataFlag, &comp_data);
			if(res != BME280_OK)
				state = STATE::needInitialize;
		}
		return res;
	}
	bool gotNewData() { return gotDataFlag; }
	const bme280_data& getData() { return comp_data; }
private:
	static int8_t initialize(bme280_dev* dev)
	{
		int8_t res = BME280_OK;

		res = bme280_init(dev);
		if(res != BME280_OK) return res;

		res = _initializeSettings(dev);
		if(res != BME280_OK) return res;

		res = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, dev);

		return res;
	}

	static int8_t _initializeSettings(bme280_dev* dev)
	{
		int8_t res = BME280_OK;
		bme280_settings settings;
		res = bme280_get_sensor_settings(&settings, dev);
		if(res != BME280_OK) return res;

		settings.filter = BME280_FILTER_COEFF_2;
		settings.osr_h = BME280_OVERSAMPLING_16X;
		settings.osr_p = BME280_OVERSAMPLING_16X;
		settings.osr_t = BME280_OVERSAMPLING_16X;
		settings.standby_time = BME280_STANDBY_TIME_20_MS;

		res = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, dev);

		return res;
	}

	static int8_t getData(bme280_dev* dev, bool& gotDataFlag, bme280_data* comp_data)
	{
		int8_t res = BME280_OK;
		gotDataFlag = false;

		uint8_t status_reg{};
		res = bme280_get_regs(BME280_REG_STATUS, &status_reg, 1, dev);
		if(res != BME280_OK) return res;

		if (status_reg & BME280_STATUS_MEAS_DONE)
		{
			res = bme280_get_sensor_data(BME280_ALL, comp_data, dev);
			if(res != BME280_OK) return res;
			gotDataFlag = true;
		}

		return res;
	}
};
