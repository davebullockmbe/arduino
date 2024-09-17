#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef Display_h
#define Display_h

/**
 * Handles button presses and releases with debouncing.
 */
class Display {
	Adafruit_SSD1306 display;

public:

	Display()
	{
		
	}

	void init()
	{
		this->display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32 or 64 from eBay)
		this->display.clearDisplay();
		this->display.display();

		this->display.setTextSize(2);    
		this->display.setTextColor(WHITE);
	}

	void pulling()
	{
		this->display.setCursor(0, 19);
		this->display.print("Pulling"); 
		this->display.display();
	}

	void pushing()
	{
		this->display.setCursor(0, 19);
		this->display.print("Pushing"); 
		this->display.display();
	}

	void pushTime(unsigned int time)
	{
		this->display.clearDisplay();
		this->display.setCursor(0,0);
		this->display.print("Time "); 
		this->display.print(time); 
		this->display.println("ms");
		this->display.display();	
	}
};

#endif