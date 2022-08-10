#ifndef Antenna_h
#define Antenna_h

#include <Arduino.h>
#include <constants.h>
#include <AMT22.h>
#include <AltSoftSerial.h>

class Antenna
{
	AMT22* encoder;
	AltSoftSerial* uart;
	uint8_t enableTxPin;

	void sendPosition()
	{
		uint16_t pos = encoder->getPosition();
		
		Serial.print("Position: ");
		Serial.println(pos);

		uart->write(Message_Initiate);
		
		uint8_t pos8 = pos >> 8;
		uart->write(pos8);
		pos8 = pos;
		uart->write(pos8);

		uart->print(Message_End);
		uart->flush();
	}

	void setTxRx(bool mode)
	{
		digitalWrite(this->enableTxPin, mode);
	}

public:
	Antenna(AltSoftSerial* uart, uint8_t enableTxPin, uint8_t encoderCSPin)
	{
		this->enableTxPin = enableTxPin;
		pinMode(enableTxPin, OUTPUT);
		digitalWrite(enableTxPin, LOW); 

		this->encoder = new AMT22(encoderCSPin);
		this->uart = uart;
		
		setTxRx(TX);
	}

	void loop()
	{
		sendPosition();
		delay(100);
	}
};

#endif
