
#include <Arduino.h>
#include <antenna.h>

#define EnableTX_Pin 2
#define CS_Pin 10


Antenna antenna(EnableTX_Pin, CS_Pin);

void setup() 
{ 
	Serial.begin(9600);
	Serial.setTimeout(100);
	Serial.println("Booted");

} 
 
void loop() 
{   
	antenna.loop();
}