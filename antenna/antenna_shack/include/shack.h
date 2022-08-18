#ifndef Shack_h
#define Shack_h

#include <RotaryEncoder.h>
#include <constants.h>
#include <display.h>
#include <AltSoftSerial.h>
#include <button.h>

class Shack
{
	RotaryEncoder* knob;
	Button* knobButton;
	Display* display;
	AltSoftSerial* uart;

	int currentAntennaPosition = 0;
	int targetAntennaPosition = 0;
	bool moving = false;
	uint8_t direction = Direction_Decreasing;
	uint8_t speed = Speed_Stop;

	int16_t knobPosition = 0;
	bool firstRead = true;

	uint8_t directionPin;
	uint8_t runPin;
	uint8_t halfSpeedPin;

	void handleKnob()
	{
		knob->tick();
		int16_t newPos = knob->getPosition();
		
		if(knobPosition == newPos)
			return;

		int diff = newPos - knobPosition;
		knobPosition = newPos;

		targetAntennaPosition = targetAntennaPosition + (diff * 5);
		
		display->setTargetPosition(targetAntennaPosition);
	}

	void handleButtonPress()
	{
		if(knobButton->pressed())
		{
			if(targetAntennaPosition != currentAntennaPosition)
			{
				Serial.print("Start moving from: ");
				Serial.print(currentAntennaPosition);
				Serial.print(" to ");
				Serial.println(targetAntennaPosition);

				moving = true;
			}
		}
	}

	void loop_setAntennaPosition()
	{
		readPosition();

		if(!moving)
			return;
			
		if(currentAntennaPosition == targetAntennaPosition)
		{
			Serial.println("stop");
			stop();
			moving = false;
			return;
		}

		// TODO: handle cyclical wrapping
		direction = currentAntennaPosition > targetAntennaPosition ? Direction_Decreasing : Direction_Increasing;
		speed = abs(currentAntennaPosition - targetAntennaPosition) <= HalfSpeedPositionThreshold ? Speed_Half : Speed_Full;
		
		if(direction == Direction_Increasing)
		{
			// if we're nearly there...
			if(currentAntennaPosition >= targetAntennaPosition - HalfSpeedPositionThreshold)
			{
				Serial.println("I Go slow");
				speed = Speed_Half;
			}
			// if we've overshot...
			else if(currentAntennaPosition > targetAntennaPosition)
			{
				Serial.println("I Overshot");
				direction = Direction_Decreasing;
				speed = Speed_Half;
			}
			// else we're not there yet, so keep going
		}
		else 
		{
			// if we're nearly there...
			if(currentAntennaPosition <= targetAntennaPosition + HalfSpeedPositionThreshold)
			{
				Serial.println("D Go slow");
				speed = Speed_Half;
			}
			// if we've overshot...
			else if(currentAntennaPosition < targetAntennaPosition)
			{
				Serial.println("D Overshot");
				direction = Direction_Increasing;
				speed = Speed_Half;
			}
			// else we're not there yet, so keep going
		}

		move(speed, direction);
	}

	void stop()
	{
		move(Speed_Stop, Direction_Increasing);
	}

	void move(uint8_t speed, bool direction)
	{
		//Serial.print("Move: ");
		//Serial.print(speed);
		//Serial.print(" ");
		//Serial.println(direction);

		digitalWrite(directionPin, direction);
		display->setDirection(direction);

		if(speed == Speed_Stop)
		{
			digitalWrite(runPin, Run_OFF);
			digitalWrite(halfSpeedPin, Slow_OFF);
			display->setSpeed(0);
			return;
		}
		
		if(speed == Speed_Half)
		{
			digitalWrite(runPin, Run_ON);
			digitalWrite(halfSpeedPin, Slow_ON);
			display->setSpeed(0);
			return;
		}

		if(speed == Speed_Full)
		{
			digitalWrite(runPin, Run_ON);
			digitalWrite(halfSpeedPin, Slow_OFF);
			display->setSpeed(0);
			return;
		}
	}

	void setTxRx(int mode)
	{
		//digitalWrite(enableTxPin, mode);
	}

	void updateAntennaPosition(uint16_t position)
	{
		uint16_t degrees = map(position, 0, 16384, 0, 3600);
		// round to nearest 0.5 (value is * 10)
		degrees -= degrees % 5;

		display->setCurrentPosition(degrees);

		currentAntennaPosition = degrees;

		if(firstRead)
		{
			targetAntennaPosition = degrees;
			display->setTargetPosition(targetAntennaPosition);
			firstRead = false;
		}
	}

	void readPosition()
	{
		// TODO: error handling

		if(uart->available() < 4)
			return;

		char c = uart->read();
		if(c == Message_Initiate)
		{
			uint16_t position = uart->read() << 8;
			position |= uart->read();

			//Serial.print("Position: ");
			//Serial.println(position);

			if(uart->read() != Message_End)
			{
				// TODO: handle error
				return;
			}

			updateAntennaPosition(position);
		}
	}

public:
	Shack(AltSoftSerial* uart, uint8_t encoderPin1, uint8_t encoderPin2, uint8_t encoderButtonPin, uint8_t directionPin, uint8_t runPin, uint8_t halfSpeedPin)
	{
		this->display = new Display();
		this->uart = uart;

		this->knob = new RotaryEncoder(encoderPin1, encoderPin2, RotaryEncoder::LatchMode::FOUR3);
		this->knobButton = new Button(encoderButtonPin, INPUT_PULLUP);

		this->directionPin = directionPin;
		this->runPin = runPin;
		this->halfSpeedPin = halfSpeedPin;

		pinMode(directionPin, OUTPUT);
		pinMode(runPin, OUTPUT);
		pinMode(halfSpeedPin, OUTPUT);

		stop();
	}

	void loop()
	{
		loop_setAntennaPosition();
		handleKnob();
		handleButtonPress();
	}
};

#endif
