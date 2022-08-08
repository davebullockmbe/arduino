#include <Arduino.h>
#include <RotaryEncoder.h>

#define RotaryEncoder_Pin1 2
#define RotaryEncoder_Pin2 3
RotaryEncoder* knob;

#define EnableTX_Pin 2
#define TX HIGH
#define RX LOW

#define IncreasePosition_Pin 4
#define DecreasePosition_Pin 5
#define HalfSpeed_Pin 6
#define HalfSpeedPositionThreshold 5

#define Relay_ON LOW
#define Relay_OFF HIGH
int currentAntennaPosition = 0;
bool moving = true;

bool direction = false;
#define Direction_Increasing true
#define Direction_Decreasing false

int targetAntennaPosition = 0;


// commands

#define Message_Initiate 'I'
#define Command_ReadPosition 'P'
#define Response_Position 'P'
#define Message_End 'E'


void setup()
{
	knob = new RotaryEncoder(RotaryEncoder_Pin1, RotaryEncoder_Pin2, RotaryEncoder::LatchMode::FOUR3);

	Serial.begin(9600);
	Serial.setTimeout(100);

	pinMode(EnableTX_Pin, OUTPUT);
	pinMode(IncreasePosition_Pin, OUTPUT);
	pinMode(DecreasePosition_Pin, OUTPUT);
	pinMode(HalfSpeed_Pin, OUTPUT);

	digitalWrite(IncreasePosition_Pin, Relay_OFF);
	digitalWrite(DecreasePosition_Pin, Relay_OFF);
	setHalfSpeed(false);
}

void loop() 
{
	readPosition();
}

void handleKnob()
{
	knob->tick();
	int16_t knobPosition = knob->getPosition();
}

void handleButtonPress()
{

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
		digitalWrite(IncreasePosition_Pin, Relay_OFF);
		digitalWrite(DecreasePosition_Pin, Relay_OFF);
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
		digitalWrite(DecreasePosition_Pin, Relay_ON);
	else
		digitalWrite(IncreasePosition_Pin, Relay_ON);
}

void setHalfSpeed(bool mode)
{
	digitalWrite(HalfSpeed_Pin, mode ? Relay_ON : Relay_OFF);
}

void setTxRx(bool mode)
{
	digitalWrite(EnableTX_Pin, mode);
}


void readPosition()
{
	setTxRx(TX);

	Serial.print(Message_Initiate);
	Serial.print(Command_ReadPosition);
	Serial.print(Message_End);
	Serial.flush();

	setTxRx(RX);

	handleResponse();
}


void handleResponse()
{
	// TODO: error handling
	while(!Serial.available())
		delay(10);

	if(Serial.read() == Message_Initiate)
	{
		char message = Serial.read();

		if(message == Response_Position)
		{
			int position = Serial.read();
			if(Serial.read() != Message_End)
			{
				// TODO: handle error
				return;
			}

			updateAntennaPosition(position);
		}	
	}
}

void updateAntennaPosition(int position)
{
	if(position == currentAntennaPosition)
		return;

	// TODO: write position to LCD

}