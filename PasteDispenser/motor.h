#include <Arduino.h>

#ifndef Motor_h
#define Motor_h

class Motor {

	int pinCW;
	int pinCCW;

public:

	/**
	 * Creates a new button for the given pin and pinMode
	 */
	Motor(int pinCW, int pinCCW) {
		this->pinCW = pinCW;
		this->pinCCW = pinCCW;

		pinMode(pinCW, OUTPUT);
		pinMode(pinCCW, OUTPUT);
	}

	void clockwise()
	{
		digitalWrite(this->pinCCW, LOW);
		digitalWrite(this->pinCW, HIGH);
	}

	void counterClockwise()
	{
		digitalWrite(this->pinCCW, HIGH);
		digitalWrite(this->pinCW, LOW);
	}

	void stop()
	{
		digitalWrite(this->pinCCW, LOW);
		digitalWrite(this->pinCW, LOW);
	}
	
	void brake()
	{
		digitalWrite(this->pinCCW, HIGH);
		digitalWrite(this->pinCW, HIGH);
	}
};

#endif