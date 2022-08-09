#ifndef Shack_h
#define Shack_h

#include <RotaryEncoder.h>
#include <constants.h>
#include <display.h>
#include <AltSoftSerial.h>

#define TX HIGH
#define RX LOW

#define Relay_ON LOW
#define Relay_OFF HIGH

#define Direction_Increasing true
#define Direction_Decreasing false

#define HalfSpeedPositionThreshold 5

#define MinPositionDegrees 90
#define MaxPositionDegrees 270

class Shack
{
	RotaryEncoder* knob;
	Display* display;
	AltSoftSerial* uart;

	int currentAntennaPosition = 0;
	int targetAntennaPosition = 0;
	bool moving = true;
	bool direction = false;

	int16_t knobPosition = 0;

	uint8_t increasePositionPin;
	uint8_t decreasePositionPin;
	uint8_t halfSpeedPin;
	uint8_t enableTxPin;

	void handleKnob()
	{
		knob->tick();
		int16_t newPos = knob->getPosition();
		
		newPos = max(MinPositionDegrees, min(MaxPositionDegrees, newPos));

		if(knobPosition == newPos)
			return;

		knobPosition = newPos;
		display->setTargetPosition(knobPosition);
	}

	void handleButtonPress()
	{
		setAntennaPosition(knobPosition);
	}

	void setAntennaPosition(int newPosition)
	{
		if(newPosition == currentAntennaPosition)
			return;

		targetAntennaPosition = newPosition;
	}

	void loop_setAntennaPosition()
	{
		readPosition();

		if(!moving)
		{
			if(currentAntennaPosition != targetAntennaPosition)
			{
				// TODO: handle cyclical wrapping
				direction = currentAntennaPosition > targetAntennaPosition ? Direction_Decreasing : Direction_Increasing;
				moving = true;

				if(abs(currentAntennaPosition - targetAntennaPosition) <= HalfSpeedPositionThreshold)
					setHalfSpeed(true);

				setRotationDirection(direction);
			}
			return;
		}

		if(currentAntennaPosition == targetAntennaPosition)
		{
			digitalWrite(increasePositionPin, Relay_OFF);
			digitalWrite(decreasePositionPin, Relay_OFF);
			setHalfSpeed(false);
			moving = false;
			return;
		}

		// TODO: handle cyclical wrapping
		if(direction == Direction_Increasing)
		{
			// if we're nearly there...
			if(currentAntennaPosition >= targetAntennaPosition - HalfSpeedPositionThreshold)
			{
				setHalfSpeed(true);
				return;
			}
			// if we've overshot...
			else if(currentAntennaPosition > targetAntennaPosition)
			{
				direction = Direction_Decreasing;
				setHalfSpeed(true);
			}
			// else we're not there yet, so keep going
		}

		// if we're nearly there...
		if(currentAntennaPosition <= targetAntennaPosition + HalfSpeedPositionThreshold)
		{
			setHalfSpeed(true);
			return;
		}
		// if we've overshot...
		else if(currentAntennaPosition < targetAntennaPosition)
		{
			direction = Direction_Increasing;
			setHalfSpeed(true);
		}
		// else we're not there yet, so keep going
	}

	void setRotationDirection(bool direction)
	{
		if(direction == Direction_Decreasing)
			digitalWrite(decreasePositionPin, Relay_ON);
		else
			digitalWrite(increasePositionPin, Relay_ON);
	}

	void setHalfSpeed(bool mode)
	{
		digitalWrite(halfSpeedPin, mode ? Relay_ON : Relay_OFF);
		display->setSpeed(mode);
	}

	void setTxRx(int mode)
	{
		digitalWrite(enableTxPin, mode);
	}

	void updateAntennaPosition(uint16_t position)
	{
		uint16_t degrees = map(position, 0, 16384, 0, 3640);
		display->setCurrentPosition(degrees);
	}

	void handleResponse()
	{
		// TODO: error handling

		Serial.println("Waiting for response");
		while(!uart->available())
			delay(10);

		Serial.println("	got response");

		if(uart->read() == Message_Initiate)
		{
			char message = uart->read();

			if(message == Response_Position)
			{
				uint16_t position = uart->read() << 8;
				position |= uart->read();

				if(uart->read() != Message_End)
				{
					// TODO: handle error
					return;
				}

				updateAntennaPosition(position);
			}	
		}
	}

	void readPosition()
	{
		Serial.println("Sending Read Position message");
		setTxRx(TX);

		uart->print(Message_Initiate);
		uart->print(Command_ReadPosition);
		uart->print(Message_End);
		uart->flush();

		setTxRx(RX);

		handleResponse();
	}

public:
	Shack(uint8_t encoderPin1, uint8_t encoderPin2, uint8_t increasePositionPin, uint8_t decreasePositionPin, 
		uint8_t halfSpeedPin, uint8_t enableTxPin)
	{
		this->display = new Display();
		this->uart = new AltSoftSerial(3, 7);
		this->uart->begin(9600);

		this->knob = new RotaryEncoder(encoderPin1, encoderPin2, RotaryEncoder::LatchMode::FOUR3);

		this->increasePositionPin = increasePositionPin;
		this->decreasePositionPin = decreasePositionPin;
		this->halfSpeedPin = halfSpeedPin;
		this->enableTxPin = enableTxPin;

		pinMode(enableTxPin, OUTPUT);
		pinMode(increasePositionPin, OUTPUT);
		pinMode(decreasePositionPin, OUTPUT);
		pinMode(halfSpeedPin, OUTPUT);

		digitalWrite(increasePositionPin, Relay_OFF);
		digitalWrite(decreasePositionPin, Relay_OFF);
		setHalfSpeed(false);
	}

	void loop()
	{
		loop_setAntennaPosition();
		handleKnob();
	}
};

#endif