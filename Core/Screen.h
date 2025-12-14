#pragma once
#include "u8g2.h"

struct ScreenData
{
	bool stateBlink = false;

	bool isBmeOk = false;
	int8_t bme280ErrCode = 0;
	bool isBMEDataOLD = false;

	int32_t temp_10Cels = 0;
	int32_t preassure_Pa = 0;
	int32_t humidity_10RH = 0;
};

class Screen
{
	u8g2_t display;
	char buffer[32];
public:
	Screen();
	void initialize_ssd1306(u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb);
	void update(const ScreenData& data);
private:
	void flush();
};
