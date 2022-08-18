#include <Arduino.h>
#include <Wire.h>
#include <shack.h>
#include <AltSoftSerial.h>

#define RotaryEncoder_Pin1 2
#define RotaryEncoder_Pin2 3
#define RotaryEncoder_ButtonPin 7

#define Direction_Pin A0
#define Run_Pin A1
#define HalfSpeed_Pin A2



Shack* shack;
AltSoftSerial uart(8, 9);

void setup()
{
	Serial.begin(9600);
	uart.begin(9600);

	shack = new Shack(&uart, RotaryEncoder_Pin1, RotaryEncoder_Pin2, RotaryEncoder_ButtonPin, Direction_Pin, Run_Pin, HalfSpeed_Pin);
}

void loop() 
{
	shack->loop();
}