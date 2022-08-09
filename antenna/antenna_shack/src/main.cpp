#include <Arduino.h>
#include <Wire.h>
#include <shack.h>
#include <AltSoftSerial.h>

#define RotaryEncoder_Pin1 2
#define RotaryEncoder_Pin2 3

#define EnableTX_Pin 2

#define IncreasePosition_Pin 4
#define DecreasePosition_Pin 5
#define HalfSpeed_Pin 6

Shack* shack;
AltSoftSerial uart(8, 9);

void setup()
{
	Serial.begin(9600);
	//Serial.setTimeout(100);
	uart.begin(9600);
	shack = new Shack(&uart, RotaryEncoder_Pin1, RotaryEncoder_Pin2, IncreasePosition_Pin, DecreasePosition_Pin, HalfSpeed_Pin, EnableTX_Pin);
}

void loop() 
{
	shack->loop();
}


// 

// 

// void setup() {
//   Serial.begin(9600);
//   Serial.println("Shack Test");
//   uart.begin(9600);
//   uart.println("Good morning to you");
// }

// void loop() {
//   char c;

//   if (Serial.available()) {
//     c = Serial.read();
//     uart.print(c);
//   }
//   if (uart.available()) {
//     c = uart.read();
//     Serial.print(c);
//   }
// }