#include "MainApp.h"
#include "stm32f1xx_hal.h"
#include "Screen.h"
#include "BME280_Processor.h"

extern I2C_HandleTypeDef hi2c1;
void I2cReset(I2C_HandleTypeDef* i2ch) //tested by reconnecting bme280 life
{
	HAL_I2C_DeInit(i2ch);
	HAL_I2C_Init(i2ch);
}

static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	  switch(msg)
	  {
	  case U8X8_MSG_DELAY_MILLI:
		  HAL_Delay(arg_int);
		  break;
	  }
	  return 1;
}

static uint8_t u8x8_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  static uint8_t buffer[32];		/* u8g2/u8x8 will никогда не отправляет более 32 байт между START_TRANSFER и END_TRANSFER */
  static uint8_t buf_idx;
  uint8_t *data;

  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 )
      {
	buffer[buf_idx++] = *data;
	data++;
	arg_int--;
      }
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      buf_idx = 0;
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
    {
    	HAL_StatusTypeDef res = HAL_I2C_Master_Transmit(&hi2c1, 0x78, buffer, buf_idx, 1000);
    	if(res != HAL_OK)
			I2cReset(&hi2c1);
      break;
    }
    default:
      return 0;
  }
  return 1;
}

BME280_INTF_RET_TYPE bme280_i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
	uint16_t addr = *static_cast<uint16_t*>(intf_ptr);
	HAL_StatusTypeDef res = HAL_I2C_Mem_Read(&hi2c1, addr, reg_addr, 1, reg_data, len, 1000);
	if(res != HAL_OK)
		I2cReset(&hi2c1);
	return res == HAL_OK ? BME280_OK : BME280_E_DEV_NOT_FOUND;
}

BME280_INTF_RET_TYPE bme280_i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{

	uint16_t addr = *static_cast<uint16_t*>(intf_ptr);
	HAL_StatusTypeDef res = HAL_I2C_Mem_Write(&hi2c1, addr, reg_addr, 1, (uint8_t*)reg_data, len, 1000);
	if(res != HAL_OK)
		I2cReset(&hi2c1);
	return res == HAL_OK ? BME280_OK : BME280_E_DEV_NOT_FOUND;
}

void bme280_delay_us(uint32_t period, void *intf_ptr)
{
	HAL_Delay(period / 1000);
}

void MainApp()
{
	HAL_Delay(100);
	Screen screen;
	ScreenData screenData;

	BME280_Processor bme280Processor(bme280_i2c_read, bme280_i2c_write, bme280_delay_us);

	uint32_t lastTimeReceivedBMEData_ms = 0;
	const uint32_t TIMEOUT_RECEIVE_BME_DATA_MS = 1000;

	screen.initialize_ssd1306(u8x8_i2c, u8x8_gpio_and_delay);
	uint32_t lastTimeScreenUpdated = 0;
	const uint32_t SCREN_REFRESH_RATE_MS = 400;
	while(1)
	{
		uint32_t currentTime_ms = HAL_GetTick();

		screenData.bme280ErrCode = bme280Processor.update();
		screenData.isBmeOk = (screenData.bme280ErrCode == BME280_OK);
		if(bme280Processor.gotNewData())
		{
			const auto& comp_data = bme280Processor.getData();
			screenData.temp_10Cels = (int32_t)(10 *comp_data.temperature);
			screenData.preassure_Pa = (int32_t)(comp_data.pressure);
			screenData.humidity_10RH = (int32_t)(10 *comp_data.humidity);
			lastTimeReceivedBMEData_ms = currentTime_ms;
		}

		if(currentTime_ms - lastTimeScreenUpdated > SCREN_REFRESH_RATE_MS)
		{
			lastTimeScreenUpdated = currentTime_ms;
			screenData.stateBlink = !screenData.stateBlink;
			screenData.isBMEDataOLD = (lastTimeReceivedBMEData_ms == 0 || currentTime_ms - lastTimeReceivedBMEData_ms > TIMEOUT_RECEIVE_BME_DATA_MS);
			screen.update(screenData);
			HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		}
	}
}
