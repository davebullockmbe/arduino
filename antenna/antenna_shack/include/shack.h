#ifndef Shack_h
#define Shack_h

#include <RotaryEncoder.h>
#include <constants.h>
#include <display.h>
#include <AltSoftSerial.h>
#include <button.h>
#include <EEPROM.h>

class Shack
{
	RotaryEncoder* knob;
	Button* menuButton;
	Button* targetButton;
	Display* display;
	AltSoftSerial* uart;

	// Values in degrees * 10
	uint16_t currentAntennaPosition = 0;
	uint16_t antennaPositionCalibration = 0;
	uint16_t targetAntennaPosition = 0;

	bool setTargetToCurrentOnNextRead = true;

	bool moving = false;
	uint8_t direction = Direction_Decreasing;
	uint8_t speed = Speed_Stop;

	int16_t knobPosition = 0;

	uint8_t directionPin;
	uint8_t runPin;
	uint8_t halfSpeedPin;

	uint8_t displayMode = Mode_Run;

	void handleMenuButtonPress()
	{
		if(!menuButton->pressed())
			return;
		
		// if(targetAntennaPosition != currentAntennaPosition)
		// {
		// 	Serial.print("Start moving from: ");
		// 	Serial.print(currentAntennaPosition);
		// 	Serial.print(" to ");
		// 	Serial.println(targetAntennaPosition);

		// 	moving = true;
		// }
		
		// writes
		if(displayMode == Mode_Calibrate_Pos)
		{
			Serial.print("Setting position calibration to ");
			Serial.println(antennaPositionCalibration);
			writeCalibrationPos();
			setTargetToCurrentOnNextRead = true;
		}

		displayMode++;
		if(displayMode > Mode_Calibrate_Max)
			displayMode = Mode_Run;

		if(displayMode == Mode_Run)
		{
			targetAntennaPosition = currentAntennaPosition;
			display->setRunMode(targetAntennaPosition, currentAntennaPosition);
		}
		else if(displayMode == Mode_Calibrate_Pos)
		{
			display->setCalibratePosMode(currentAntennaPosition);
		}

		// reset knob activity
		knob->setPosition(0);
		knobPosition = 0;
	}


	void runMode_handleTargetButtonPress()
	{
		if(!targetButton->pressed())
			return;
		
		if(targetAntennaPosition == currentAntennaPosition)
			return;
		
		Serial.print("Start moving from: ");
		Serial.print(currentAntennaPosition);
		Serial.print(" to ");
		Serial.println(targetAntennaPosition);

		moving = true;
	}

	void runMode_handleKnob()
	{
		knob->tick();
		int16_t newPos = knob->getPosition();
		
		if(knobPosition == newPos)
			return;

		int diff = newPos - knobPosition;
		knobPosition = newPos;

		targetAntennaPosition = cycle360(targetAntennaPosition + (diff * 10)); //5)); // TODO put back to .5 degree

		display->setTargetPosition(targetAntennaPosition);
	}

	void runMode_setAntennaPosition()
	{
		readPosition();

		if(!moving)
			return;
			
		if(currentAntennaPosition == targetAntennaPosition)
		{
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
				speed = Speed_Half;
			}
			// if we've overshot...
			else if(currentAntennaPosition > targetAntennaPosition)
			{
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
				speed = Speed_Half;
			}
			// if we've overshot...
			else if(currentAntennaPosition < targetAntennaPosition)
			{
				direction = Direction_Increasing;
				speed = Speed_Half;
			}
			// else we're not there yet, so keep going
		}

		move(speed, direction);
	}

	void updateAntennaPosition(uint16_t position)
	{
		uint16_t degrees = map(position, 0, 16384, 0, 3600);
		// round to nearest 0.5 (value is * 10)
		degrees -= degrees % 5;
		
		currentAntennaPosition = cycle360(degrees + antennaPositionCalibration);

		if(displayMode == Mode_Run)
			display->setCurrentPosition(currentAntennaPosition);

		if(setTargetToCurrentOnNextRead)
		{
			targetAntennaPosition = currentAntennaPosition;
			display->setTargetPosition(targetAntennaPosition);
			setTargetToCurrentOnNextRead = false;
		}
	}

	void calibratePosMode_handleKnob()
	{
		knob->tick();
		int16_t newPos = knob->getPosition();
		
		if(knobPosition == newPos)
			return;

		int diff = newPos - knobPosition;
		knobPosition = newPos;

		uint16_t oldAntennaPositionCalibration = antennaPositionCalibration;

		antennaPositionCalibration = cycle360(antennaPositionCalibration + (diff * 5));
		
		int16_t value = cycle360(currentAntennaPosition - oldAntennaPositionCalibration + antennaPositionCalibration);
		
		display->setCalibrationPosition(value);
	}

	uint16_t cycle360(int16_t value)
	{
		if(value < 0)
			return 3600 + value;
		
		if(value >= 3600)
			return value - 3600;
		
		return value;
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

	void readCalibrationPos()
	{
		uint16_t pos = 0;
		EEPROM.get(0, pos);

		//handle empty EEPROM
		if(pos == 65535)
			pos = 0;
		
		antennaPositionCalibration = pos;
	}

	void writeCalibrationPos()
	{
		EEPROM.put(0, antennaPositionCalibration);
	}

public:
	Shack(AltSoftSerial* uart, uint8_t encoderPin1, uint8_t encoderPin2, uint8_t encoderButtonPin, uint8_t directionPin, uint8_t runPin, uint8_t halfSpeedPin, uint8_t targetButtonPin)
	{
		EEPROM.begin();

		readCalibrationPos();

		this->uart = uart;

		this->display = new Display();
		this->display->setRunMode(0, 0);

		this->displayMode = Mode_Run;

		this->knob = new RotaryEncoder(encoderPin1, encoderPin2, RotaryEncoder::LatchMode::FOUR3);
		this->menuButton = new Button(encoderButtonPin, INPUT_PULLUP);
		this->targetButton = new Button(targetButtonPin, INPUT_PULLUP);

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
		handleMenuButtonPress();

		if(displayMode == Mode_Run)
		{
			runMode_handleTargetButtonPress();
			runMode_setAntennaPosition();
			runMode_handleKnob();
			return;
		}

		if(displayMode == Mode_Calibrate_Pos)
		{
			readPosition();
			calibratePosMode_handleKnob();
		}
	}
};

#endif
