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

	void handleCommand()
	{
		if(uart->available() < 3)
			return;

		char c = uart->read();

		if(c == Message_Initiate)
		{
			Serial.println("Received UART message:");
			char message = uart->read();

			if(message == Command_ReadPosition)
			{
				Serial.println("	ReadPosition");

				if(uart->read() != Message_End)
					// TODO: handle error
					return;

				handleReadPosition();
			}	
		}
	}

	void handleReadPosition()
	{
		uint16_t pos = encoder->getPosition();
		
		Serial.print("		Read position: ");
		Serial.println(pos);

		setTxRx(TX);

		uart->print(Message_Initiate);
		uart->print(Response_Position);
		
		uint8_t pos8 = pos >> 8;
		uart->print(pos8);
		pos8 = pos;
		uart->print(pos8);

		uart->print(Message_End);
		uart->flush();

		setTxRx(RX);	
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
	}

	void loop()
	{
	// 	char c;
	// 	if (Serial.available()) {
	// 		c = Serial.read();
	// 		uart->print(c);
	// 	}
	// 	if (uart->available()) {
	// 		c = uart->read();
	// 		Serial.print(c);
	// 	}
		handleCommand();
	}
};

#endif
