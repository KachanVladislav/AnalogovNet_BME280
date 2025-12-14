#include "Screen.h"
#include <cstdio>

Screen::Screen()
{
}

void Screen::initialize_ssd1306(u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&display, U8G2_R0, byte_cb, gpio_and_delay_cb);
	u8g2_InitDisplay(&display); // эта функция отправляет последовательность инициализации на дисплей, после чего он переходит в спящий режим
	u8g2_SetPowerSave(&display, 0); // пробуждает дисплей
	u8g2_SetFont(&display, u8g2_font_8x13B_tf);//for degree glyph
//	u8g2_SetFont(&display, u8g2_font_8x13B_mr);
//	u8g2_SetFont(data->display, u8g2_font_10x20_t_cyrillic);
}

void Screen::update(const ScreenData& data)
{
	u8g2_ClearBuffer(&display);

	if(data.stateBlink)
		if(data.isBMEDataOLD)
			u8g2_DrawStr(&display, 10, 16, "AnaLogovOLD");
		else
			u8g2_DrawStr(&display, 10, 16, "AnaLogovNET");
	else
		u8g2_DrawStr(&display, 10, 16, "AnaLogov");

	if(data.isBmeOk)
	{
		if(data.isBMEDataOLD && data.stateBlink)
		{
			sprintf(buffer, "BME280 found");
			u8g2_DrawStr(&display, 10, 32, buffer);
			sprintf(buffer, "Waiting for");
			u8g2_DrawStr(&display, 10, 48, buffer);
			sprintf(buffer, "   data");
			u8g2_DrawStr(&display, 10, 64, buffer);
		}
		else
		{
			sprintf(buffer, "t %d.%d %cC", (int)data.temp_10Cels / 10, (int)data.temp_10Cels % 10, 176); //176 is degree glyph
			u8g2_DrawStr(&display, 10, 32, buffer);
			int preassureLow = (data.preassure_Pa % 1000) / 10;
			sprintf(buffer, "P %d.%d kPa", (int)data.preassure_Pa / 1000, preassureLow);
			u8g2_DrawStr(&display, 10, 48, buffer);
			sprintf(buffer, "H %d.%d %%RH", (int) data.humidity_10RH / 10,(int) data.humidity_10RH % 10);
			u8g2_DrawStr(&display, 10, 64, buffer);
		}
	}
	else
	{
		sprintf(buffer, "bmeFail(%d)", data.bme280ErrCode);
		u8g2_DrawStr(&display, 10, 32, buffer);
	}

	flush();
}

void Screen::flush()
{
	u8g2_SendBuffer(&display);
}

