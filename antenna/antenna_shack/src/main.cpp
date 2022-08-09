#include <Arduino.h>
#include <Wire.h>
#include <shack.h>

#define RotaryEncoder_Pin1 2
#define RotaryEncoder_Pin2 3

#define EnableTX_Pin 2

#define IncreasePosition_Pin 4
#define DecreasePosition_Pin 5
#define HalfSpeed_Pin 6

Shack* shack;

void setup()
{
	Serial.begin(9600);
	//Serial.setTimeout(100);

	shack = new Shack(RotaryEncoder_Pin1, RotaryEncoder_Pin2, IncreasePosition_Pin, DecreasePosition_Pin, HalfSpeed_Pin, EnableTX_Pin);
}

void loop() 
{
	shack->loop();
}
