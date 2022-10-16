#include <Arduino.h>

#ifndef Button_h
#define Button_h

#define ButtonEvent_None 0
#define ButtonEvent_Pressed 1
#define ButtonEvent_Released 2

/**
 * Handles button presses and releases with debouncing.
 */
class Button {

	int pin;
	int mode;
	unsigned long lastMillis = 0;
	bool lastPressed = false;
	bool isPressed = false;
	bool read = true;

	/**
	 * Updates the state of the button.
	 * If the state changes, this is recorded and `read = false`
	 */
	void updateState()
	{
		bool pressed = digitalRead(this->pin) == (mode == INPUT ? HIGH : LOW);

		if (pressed != this->lastPressed)
		{
			this->lastMillis = millis();
			this->lastPressed = pressed;
			return;
		}
		
		if (millis() > (this->lastMillis + 50))
		{
			if (pressed == this->isPressed)
				return;

			this->isPressed = pressed;
			this->read = false;
		}
	}

public:

	/**
	 * Creates a new button for the given pin and pinMode
	 */
	Button(int pin, int mode) {
		this->pin = pin;
		this->mode = mode;

		pinMode(pin, mode);
		this->updateState();
		// ensure released() works when a button is in the released state when this class is first instantiated
		this->read = false;
	}

	/**
	 * Determines if the button has been pressed.
	 * Calling this method will only return `true` once for each press/release.
	 * This will return pressed/released if the button is pressed/released when this class was instantiated
	 */
	uint8_t event() {
		this->updateState();

		if(this->read)
			return ButtonEvent_None;

		this->read = true;
		return isPressed ? ButtonEvent_Pressed : ButtonEvent_Released;
	}
};

#endif