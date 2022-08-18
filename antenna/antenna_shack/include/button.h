#include <Arduino.h>

#ifndef Button_h
#define Button_h

/**
 * Handles button presses and releases with debouncing.
 */
class Button {

	int pin;
	int mode;
	unsigned long lastMillis = 0;
	bool lastPressed = 0;
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
	 * Calling this method will only return `true` once for each press.
	 * This will return true if the button is pressed when this class was instantiated
	 */
	bool pressed() {
		this->updateState();

		if(this->read)
			return false;

		this->read = true;
		return isPressed;
	}

	/**
	 * Determines if the button has been released after being pressed.
	 * Calling this method will only return `true` once for each release.
	 * This will return true if the button is released when this class was instantiated
	 */
	bool released() {
		this->updateState();
		
		if(this->read)
			return false;

		this->read = true;
		return !isPressed;
	}
};

#endif