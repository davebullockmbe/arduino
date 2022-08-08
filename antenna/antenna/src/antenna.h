#ifndef Antenna_h
#define Antenna_h

#include <Arduino.h>
#include <constants.h>
#include <AMT22.h>


class Antenna
{
	AMT22* encoder;
	uint8_t encoderCSPin;
	uint8_t enableTxPin;

	void handleCommand()
	{
		if(!Serial.available())
			return;

		if(Serial.read() == Message_Initiate)
		{
			char message = Serial.read();

			if(message == Command_ReadPosition)
			{
				if(Serial.read() != Message_End)
					// TODO: handle error
					return;

				handleReadPosition();
			}	
		}
	}

	void handleReadPosition()
	{
		uint16_t pos = encoder.getPosition();
		
		setTxRx(TX);

		Serial.print(Message_Initiate);
		Serial.print(Response_Position);
		
		uint8_t pos8 = pos >> 8;
		Serial.print(pos);
		pos8 = pos;
		Serial.print(pos);

		Serial.print(Message_End);
		Serial.flush();

		setTxRx(RX);	
	}

	void setTxRx(bool mode)
	{
		digitalWrite(this->encoderCSPin, mode);
	}

public:
	Antenna(uint8_t enableTxPin, uint8_t encoderCSPin)
	{
		this->enableTxPin = enableTxPin;
		pinMode(enableTxPin, OUTPUT);
		digitalWrite(enableTxPin, LOW); 

		this->encoderCSPin = encoderCSPin;
	}

	void loop()
	{
		handleCommand();
	}
};

#endif
