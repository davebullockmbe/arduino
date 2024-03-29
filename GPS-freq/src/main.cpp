
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>

#include <maidenhead.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header

#define TXPin 2
#define RXPin 3

hd44780_I2Cexp lcd;
SoftwareSerial ss(TXPin, RXPin);
Adafruit_GPS GPS(&ss);

uint32_t timer = millis();
uint8_t prevSats = 0;
bool prevFix = false;

void setup()
{
	Serial.begin(115200);

	lcd.begin(40, 2);

	lcd.clear();
	lcd.print("Initialising...");

	// 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
	GPS.begin(9600);
	// uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
	GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
	// uncomment this line to turn on only the "minimum recommended" data
	//GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
	// For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
	// the parser doesn't care about other sentences at this time
	// Set the update rate
	GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
	// For the parsing code to work nicely and have time to sort thru the data, and
	// print it out we don't suggest using anything higher than 1 Hz

	// Request updates on antenna status, comment out to keep quiet
	GPS.sendCommand(PGCMD_ANTENNA);

	delay(1000);

	// Ask for firmware version
	ss.println(PMTK_Q_RELEASE);

}

void loop() // run over and over again
{
	char c = GPS.read();
	if(c)
		Serial.print(c);

	// if a sentence is received, we can check the checksum, parse it...
	if (GPS.newNMEAreceived()) {
		// a tricky thing here is if we print the NMEA sentence, or data
		// we end up not listening and catching other sentences!
		// so be very wary if using OUTPUT_ALLDATA and trying to print out data
		//Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
		if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
			return; // we can fail to parse a sentence in which case we should just wait for another
	}

 	// approximately every 2 seconds or so, print out the current stats
	if (millis() - timer <= 1000)
		return;
  
	timer = millis(); // reset the timer
	
	// Line 0
	
	// time
	lcd.setCursor(0, 0);
	if(GPS.satellites > 0 || GPS.fix)
 	{	
		lcd.print("GMT ");

		if (GPS.hour < 10)
			lcd.print('0');
		lcd.print(GPS.hour, DEC);
		lcd.print(':');
		if (GPS.minute < 10)
			lcd.print('0');
		lcd.print(GPS.minute, DEC);
		lcd.print(':');
		if (GPS.seconds < 10)
			lcd.print('0');
		lcd.print(GPS.seconds, DEC);
		lcd.print("  ");
	}
	else
	{
		lcd.print("              ");
	}
	

	// satellites
	lcd.setCursor(32, 0);
	lcd.print("Sats ");
	lcd.print(GPS.satellites);
	// trailing whitespace
	if(GPS.satellites < 100)
		lcd.print(" ");
	if(GPS.satellites < 10)
		lcd.print(" ");


	// Line 1

	lcd.setCursor(0, 1);


	if (GPS.fix)
	{
		if(!prevFix)
		{
			prevFix = true;
		}

		// position
		lcd.setCursor(0, 1);
		lcd.print("Pos ");
		lcd.print(GPS.latitudeDegrees, 5);
		lcd.print(' ');
		lcd.print(GPS.longitudeDegrees, 5);


		// altitude
		lcd.setCursor(14, 0);
		lcd.print("Altitude ");
		lcd.print(GPS.altitude);
		lcd.print("m");

		// maidenhead
		char* maidenhead = get_mh(GPS.latitudeDegrees, GPS.longitudeDegrees, 6);
		lcd.setCursor(23, 1);
		lcd.print("Maidenhead ");
		lcd.print(maidenhead);
	}
	else 
	{
		if(prevFix)
			prevFix = false;
		
		lcd.clear();
		lcd.setCursor(1, 0);
		lcd.print("No satellite fix.   ");
	}
}