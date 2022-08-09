
#include <Arduino.h>
#include <antenna.h>
#include <AltSoftSerial.h>

#define EnableTX_Pin 2
#define CS_Pin 10

// This must be defined here, for some reason
AltSoftSerial uart;
Antenna antenna(&uart, EnableTX_Pin, CS_Pin);

void setup() {
  Serial.begin(9600);

  // this has to be in setup() for some reason
  uart.begin(9600);
  
  Serial.println("Antenna Booted");
}

void loop() 
{
	antenna.loop();
}
