#include "display.h"
#include "button.h"
#include "motor.h"

#define rotateIntervalMs 50
#define breakPauseMs 50
#define retrationMs 50

Button btnPush(2, INPUT_PULLUP);
Button btnPull(3, INPUT_PULLUP);
Button btnMode(4, INPUT_PULLUP);

Motor motor(6, 5);

Display display;

unsigned int rotate_time = rotateIntervalMs;
unsigned int max_rotate_time = rotateIntervalMs * 12;
bool pushing = false;
unsigned long current_time = 0;
unsigned long push_time = 0;

void setup()
{
	Serial.begin(115200);
	Serial.println("Booting...");


	motor.brake();
	display.init();
	display.pushTime(rotate_time);

	Serial.println("Ready");
}

void handleButtons()
{
	// Mode

	ButtonState mode = btnMode.readStateChange();
	if(mode == ButtonState::Pressed)
	{
		Serial.println("Mode pressed");

		rotate_time += rotateIntervalMs;
		if(rotate_time > max_rotate_time)
			rotate_time = rotateIntervalMs;
		
		display.pushTime(rotate_time);
	}


	// Pull
	ButtonState pull = btnPull.readStateChange();
	if(pull == ButtonState::Pressed){
		Serial.println("Pull pressed");
		
		motor.counterClockwise();
		display.pulling();

	}
	else if(pull == ButtonState::Released)
	{
		motor.brake();
		display.pushTime(rotate_time);

	}


	// Push

	ButtonState push = btnPush.readStateChange();
	if(push == ButtonState::Pressed)
	{
		Serial.println("Push pressed");
		
		// stop double push during long push durations
		if(pushing)
			return;

		pushing = true;
		push_time = millis();
		motor.clockwise();

		//display.pushing();
	}
}

void loop() {
	current_time = millis();

	handleButtons();
	
	if(pushing)
	{
		// finished pushing
		if(current_time >= push_time + rotate_time)
		{
			pushing = false;
			motor.brake();
			delay(breakPauseMs);
			motor.counterClockwise();
			delay(retrationMs);
			motor.brake();		
			display.pushTime(rotate_time);

		}
  	}
}
