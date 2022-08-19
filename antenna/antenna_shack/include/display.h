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
	uint16_t calibrationPos;

	void printDegrees(uint16_t degrees)
	{
		uint8_t hundreds = degrees / 1000; // we're x10 for the 1dp
		if(hundreds == 0)
			lcd->print(' ');
		else
			lcd->print(hundreds);
		
		uint8_t tens = (degrees - hundreds * 1000) / 100; // we're x10 for the 1dp
		if(hundreds == 0 && tens == 0)
			lcd->print(' ');
		else
			lcd->print(tens);

		uint8_t ones = (degrees - hundreds * 1000 - tens * 100) / 10; // we're x10 for the 1dp
		lcd->print(ones);

		lcd->print('.');
		
		uint8_t fraction = degrees - hundreds * 1000 - tens * 100 - ones * 10; // we're x10 for the 1dp
		lcd->print(fraction);
		
		lcd->write((byte)0); //degree
	}

public:
	Display()
	{
		lcd = new hd44780_I2Cexp();
		uint8_t degree[8] = {0b01000,0b10100,0b01000,0x0,0x0,0x0,0x0,0x0};

		lcd->begin(20, 4);
		lcd->createChar(0, degree);
		lcd->home();

		currentPos = 0;
		targetPos = 0;
		calibrationPos = 0;
	}

	void setRunMode(uint16_t target, uint16_t current)
	{
		currentPos = current;
		targetPos = target;

		lcd->clear();

		lcd->setCursor(0, 0);
		lcd->print("Target pos: ");
		printDegrees(targetPos);

		lcd->setCursor(0, 1);
		lcd->print("Current pos: ");
		printDegrees(currentPos);

		lcd->setCursor(0, 2);
		lcd->print("Speed: 100%");

		lcd->setCursor(0, 3);
		lcd->print("Direction: N/A");
	}

	void setCalibratePosMode(uint16_t position)
	{
		calibrationPos = position;
		lcd->clear();

		lcd->setCursor(0, 0);
		lcd->print("Calibration Mode");

		lcd->setCursor(0, 1);
		lcd->print("Set Current Position");

		lcd->setCursor(0, 2);
		
		lcd->print("       ");
		printDegrees(calibrationPos);
	}

	void setCalibrationPosition(uint16_t position)
	{
		if(position == calibrationPos)
			return;

		lcd->setCursor(7, 2);
		printDegrees(position);
	}

	void setTargetPosition(uint16_t degrees)
	{
		if(degrees == targetPos)
			return;

		targetPos = degrees;

		lcd->setCursor(12, 0);
		printDegrees(degrees);
	}

	void setCurrentPosition(uint16_t degrees)
	{
		if(degrees == currentPos)
			return;
			
		lcd->setCursor(13, 1);
		printDegrees(degrees);
	}

	void setSpeed(uint8_t speed)
	{
		lcd->setCursor(7, 2);
		
		if(speed == Speed_Stop)
			lcd->print("0%  ");
		else if(speed == Speed_Half)
			lcd->print("50% ");
		else
			lcd->print("100%");
	}

	void setDirection(uint8_t direction)
	{
		lcd->setCursor(11, 3);
		
		if(direction == Direction_Decreasing)
			lcd->print("CCW");
		else if(direction == Direction_Increasing)
			lcd->print("CW ");
		else
			lcd->print("N/A");
	}
};

#endif