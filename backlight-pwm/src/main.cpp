#include <Arduino.h>

#define V_IN_MIN 143 // 0.7V
#define V_IN_MAX 962 // 4.7V

#define PWM_OUT_MIN 0
#define PWM_OUT_MAX 255

// https://europe1.discourse-cdn.com/arduino/original/4X/7/9/2/792251f6b79970904de17bc51383f200771bc306.png
#define OUTPUT_PIN PB1
#define INPUT_PIN PB2

void setup()
{
	pinMode(OUTPUT_PIN, OUTPUT);
}

void loop()
{
	int value = analogRead(INPUT_PIN);

	int out = map(value, V_IN_MIN, V_IN_MAX, PWM_OUT_MIN, PWM_OUT_MAX);

	analogWrite(OUTPUT_PIN, out);
}