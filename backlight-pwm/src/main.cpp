#include <Arduino.h>

// 1203 * 1v / 3.3v
#define V_IN_MIN 310
#define V_IN_MAX 1023

#define PWM_OUT_MIN 0
#define PWM_OUT_MAX 255

// https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
#define OUTPUT_PIN 1

void setup()
{
	pinMode(OUTPUT_PIN, OUTPUT);
}

void loop()
{
	int value = analogRead(A0);

	int out = map(value, V_IN_MIN, V_IN_MAX, PWM_OUT_MIN, PWM_OUT_MAX);

	analogWrite(OUTPUT_PIN, out);
}