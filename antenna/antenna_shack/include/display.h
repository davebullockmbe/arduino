#ifndef display_h
#define display_h

#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <constants.h>

class Display
{
	hd44780_I2Cexp* lcd;
	uint16_t targetPos;
	uint16_t currentPos;

public:
	Display()
	{
		lcd->setCursor(0, 0);
		lcd->print("Tgt 000°");
		targetPos = 0;

		lcd->setCursor(0, 1);
		lcd->print("Pos 000° Spd 100");
		currentPos = 0;
	}

	void setTargetPosition(uint16_t degrees)
	{
		if(degrees == targetPos)
			return;

		lcd->setCursor(5, 0);
		if(degrees < 100)
			lcd->print(' ');
		if(degrees < 10)
			lcd->print(' ');
		lcd->print(degrees);
	}

	void setCurrentPosition(uint16_t degrees)
	{
		if(degrees == currentPos)
			return;
			
		lcd->setCursor(4, 1);
		if(degrees < 100)
			lcd->print(' ');
		if(degrees < 10)
			lcd->print(' ');
		lcd->print(degrees);
	}

	void setSpeed(uint8_t speed)
	{
		lcd->setCursor(4, 1);
		if(speed == Speed_Zero)
			lcd->print("  0");
		else if(speed == Speed_Half)
			lcd->print(" 50");
		else
			lcd->print("100");
	}
};

#endif display_h