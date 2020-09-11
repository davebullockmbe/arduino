// Dual-power-meter-AD8318-V1.ino
// by Reinhardt Weber, DC5ZM + AI6PK

#include <Arduino.h>
#include <EEPROM.h>
#include "LiquidCrystal.h"
#include "Bounce2.h"

// ----------------------------------------
// Configuration Zone


// KEY PINS
#define SELECT_PIN 11
#define UP_PIN 5
#define DOWN_PIN 4
#define LEFT_PIN 3
#define RIGHT_PIN 2

// Button debounce time in ms.
const byte BUTTON_DEBOUNCE_MS = 5;

// allocate arduino pins to LCD pins(rs,enable,DB4,DB5,DB6,DB7)
LiquidCrystal lcd(12, 10, 8, 5, 7, 6); 


// END Configuration Zone
// ----------------------------------------



Bounce button_SELECT = Bounce();
Bounce button_UP = Bounce();
Bounce button_DOWN = Bounce();
Bounce button_LEFT = Bounce();
Bounce button_RIGHT = Bounce();

// custom chacters made using   https://omerk.github.io/lcdchargen

byte char_up_down[8] = {0b00100, 0b01010, 0b10001, 0b00000, 0b10001, 0b01010, 0b00100, 0b00000};
byte char_left_right[8] = {0b10000, 0b01000, 0b00100, 0b01001, 0b10010, 0b00100, 0b00010, 0b00001};

byte freq_curve_nr, freq_curve_nr_prev, iii, key_voltage, KEY, display_menue_nr = 1;
byte att_CH1, att_CH1_prev, att_CH2 = 0, att_CH2_prev;

char float_string[7];
int error_limit_LOW, error_limit_HIGH;

float f_ghz, mmm, ccc;
float voltage_CH_1, level_CH_1, power_W_1, Return_Loss, RL_linear, SWR;
float voltage_CH_2, level_CH_2, power_W_2;


void setup()
{
	initButtons();

	lcd.begin(16, 2);
	// +5V
	analogReference(DEFAULT); 

	lcd.createChar(0, char_up_down);
	lcd.createChar(1, char_left_right);

	lcd.clear();
	lcd.print("DUAL POWER-METER");
	lcd.setCursor(0, 1);
	lcd.print(" 1 MHz -> 8 GHz ");
	delay(1000);
	lcd.setCursor(0, 1);
	lcd.print("(c)2019 by DC5ZM");
	delay(1000);

	freq_curve_nr = EEPROM.read(0);
	att_CH1 = EEPROM.read(1);
	att_CH2 = EEPROM.read(2);
	lcd.clear();
}

void initButtons()
{
	// Set button pins as input.
	pinMode(SELECT_PIN, INPUT_PULLUP);
	pinMode(UP_PIN, INPUT_PULLUP);
	pinMode(DOWN_PIN, INPUT_PULLUP);
	pinMode(LEFT_PIN, INPUT_PULLUP);
	pinMode(RIGHT_PIN, INPUT_PULLUP);

	// Attach pins to Bounce
	button_SELECT.attach(SELECT_PIN);
	button_UP.attach(UP_PIN);
	button_DOWN.attach(DOWN_PIN);
	button_LEFT.attach(LEFT_PIN);
	button_RIGHT.attach(RIGHT_PIN);

	// Set bounce interval
	button_SELECT.interval(BUTTON_DEBOUNCE_MS);
	button_UP.interval(BUTTON_DEBOUNCE_MS);
	button_DOWN.interval(BUTTON_DEBOUNCE_MS);
	button_LEFT.interval(BUTTON_DEBOUNCE_MS);
	button_RIGHT.interval(BUTTON_DEBOUNCE_MS);
}

void loop()
{
	if (display_menue_nr == 1)
	{
		select_menue();				// on LCD-Keypad-Shield
		select_attenuator_CH_1();	// total attenuation
		select_calibration_curve(); // corresponding to AD8318 data sheet

		read_output_CH_1();		// of module AD8318
		calculate_power_CH_1(); // of measurements
		display_power_CH_1();	// on LCD-Keypad-Shield
	}

	if (display_menue_nr == 2)
	{
		select_menue();				// on LCD-Keypad-Shield
		select_attenuator_CH_2();	// on input SMA socket
		select_calibration_curve(); // based on AD8318 data sheet

		read_output_CH_2();		// of module AD8318
		calculate_power_CH_2(); // of measurements
		display_power_CH_2();	// on LCD-Keypad-Shield
	}

	if (display_menue_nr == 3)
	{
		read_output_CH_1();		// of module AD8318
		calculate_power_CH_1(); // of measurements
		read_output_CH_2();		// of module AD8318

		calculate_power_CH_2(); // of measurements
		calculate_ReturnLoss_and_SWR();
		display_ReturnLoss_and_SWR(); // on LCD-Keypad-Shield
		select_menue();				  // on LCD-Keypad-Shield
	}
}

// =========================================================================================== MENUE 1+2
void select_menue()
{
	bool selectPressed = (button_SELECT.update() && button_SELECT.fell());
	
	if (!selectPressed)
		return;	
	
	if (display_menue_nr == 1)
		display_menue_nr = 2;
	else if (display_menue_nr == 2)
		display_menue_nr = 3;
	else if (display_menue_nr == 3)
		display_menue_nr = 1;
}


void select_calibration_curve()
{
	bool leftPressed = (button_LEFT.update() && button_LEFT.fell());
	bool rightPressed = (button_RIGHT.update() && button_RIGHT.fell());

	if (leftPressed)
		freq_curve_nr--; // freq_curve_nr = counter index, decrement
	else if (rightPressed)
		freq_curve_nr++; // increment counter index

	// handle over/underflow of counter index
	freq_curve_nr = freq_curve_nr > 6 ? 6 : freq_curve_nr < 1 ? 1 : freq_curve_nr;

	if (freq_curve_nr == 1)
	{
		f_ghz = 0.9;
		AD8318_use_curve_0ghz9();
	} 
	else if (freq_curve_nr == 2)
	{
		f_ghz = 1.9;
		AD8318_use_curve_1ghz9();
	}
	else if (freq_curve_nr == 3)
	{
		f_ghz = 2.2;
		AD8318_use_curve_2ghz2();
	}
	else if (freq_curve_nr == 4)
	{
		f_ghz = 3.6;
		AD8318_use_curve_3ghz6();
	}
	else if (freq_curve_nr == 5)
	{
		f_ghz = 5.8;
		AD8318_use_curve_5ghz8();
	}
	else if (freq_curve_nr == 6)
	{
		f_ghz = 8.0;
		AD8318_use_curve_8ghz0();
	}

	if (freq_curve_nr_prev != freq_curve_nr) 
	{
		EEPROM.write(0, freq_curve_nr); 
		freq_curve_nr_prev = freq_curve_nr;
	}
}

void select_attenuator_CH_1()
{
	bool upPressed = (button_UP.update() && button_UP.fell());
	bool downPressed = (button_DOWN.update() && button_DOWN.fell());

	if (downPressed || upPressed)
	{
		if (downPressed)
			att_CH1--;
		else if (upPressed)
			att_CH1++;		
		
		// byte att_CH1 will self-cycle around from 255->0 and 0->255

		dtostrf(att_CH1, 2, 0, float_string); // format number to 2 digits, no nr. behind the dec.point
		lcd.setCursor(11, 0);
		lcd.print(float_string);
	}

	if (att_CH1_prev != att_CH1)
	{
		EEPROM.write(1, att_CH1);
		att_CH1_prev = att_CH1;
	}
}

void read_output_CH_1()
{
	voltage_CH_1 = 0;
	for (iii = 0; iii < 10; iii++)
		voltage_CH_1 = voltage_CH_1 + analogRead(A1);

	// calculate average value of 10 readings
	// 2046 = 10*1023/5V
	voltage_CH_1 = voltage_CH_1 / 2046; 
} 

void calculate_power_CH_1() 
{
	// uses straight line equation: y = mmm*x + ccc -> x = (y - ccc)/mmm
	level_CH_1 = -(voltage_CH_1 - ccc) / mmm + att_CH1;
	
	// round and convert to integer
	level_CH_1 = floor(level_CH_1 + 0.5);

	// convert dBm to mW
	power_W_1 = pow(10, level_CH_1 / 10); 
}

void display_power_CH_1()
{
	lcd.setCursor(0, 0);					 // go to line #1
	dtostrf(level_CH_1, 3, 0, float_string); //convert float to string
	lcd.print("1L");
	lcd.print(float_string);
	lcd.print("dBm ");

	lcd.print("AT");
	dtostrf(att_CH1, 2, 0, float_string);
	lcd.print(float_string);
	lcd.print("dB");
	lcd.write(byte(0)); // custom made char_up_down
	
	// go to line #2
	lcd.setCursor(0, 1); 
	lcd.print("1P");
	select_subunit_of_power_CH_1();

	dtostrf(f_ghz, 3, 1, float_string);
	lcd.print(float_string);
	lcd.print("GHz");
	lcd.write(byte(1)); // custom made char_left_right

	if ((level_CH_1 - att_CH1) < error_limit_LOW || (level_CH_1 - att_CH1) > error_limit_HIGH)
	{
		delay(300);
		lcd.setCursor(0, 0);
		lcd.print(" ");
		lcd.setCursor(0, 1);
		lcd.print(" ");
		delay(300);
	}
}

void AD8318_use_curve_0ghz9()
{
	mmm = 0.025; // values for straight line equation: mmm = slope
	ccc = 0.56;	 // ccc = intercept y-axis

	error_limit_LOW = -55; // dBm limits for acceptable error range
	error_limit_HIGH = -1;
}

void AD8318_use_curve_1ghz9()
{
	mmm = 0.025;
	ccc = 0.45;
	error_limit_LOW = -68;
	error_limit_HIGH = -5;
}

void AD8318_use_curve_2ghz2()
{
	mmm = 0.025;
	ccc = 0.45;
	error_limit_LOW = -60;
	error_limit_HIGH = -4;
}

void AD8318_use_curve_3ghz6()
{
	mmm = 0.025;
	ccc = 0.5;
	error_limit_LOW = -52;
	error_limit_HIGH = -3;
}

void AD8318_use_curve_5ghz8()
{
	mmm = 0.025;
	ccc = 0.63;
	error_limit_LOW = -56;
	error_limit_HIGH = 0;
}

void AD8318_use_curve_8ghz0()
{
	mmm = 0.025;
	ccc = 0.77;
	error_limit_LOW = -54;
	error_limit_HIGH = 0;
}

void select_subunit_of_power_CH_1() 
{
	// select unit: W, mW, µW, nW

	if (level_CH_1 < -30)
	{
		dtostrf(power_W_1 * 1000000, 4, 0, float_string); // convert float to string
		lcd.print(float_string);
		lcd.print("nW ");
	}
	else if (level_CH_1 >= -30 && level_CH_1 < 0)
	{
		dtostrf(power_W_1 * 1000, 4, 0, float_string);
		lcd.print(float_string);
		lcd.print("\xE4W "); // \xE4W = ascii code for µW
	}
	else if (level_CH_1 >= 0 && level_CH_1 < 30)
	{
		dtostrf(power_W_1, 4, 0, float_string);
		lcd.print(float_string);
		lcd.print("mW ");
	}
	else if (level_CH_1 >= 30 && level_CH_1 < 40)
	{
		dtostrf(power_W_1 / 1000, 4, 1, float_string); // 1.0W to 9.9W
		lcd.print(float_string);
		lcd.print(" W ");
	}
	else if (level_CH_1 >= 40 && level_CH_1 < 60)
	{
		dtostrf(power_W_1 / 1000, 4, 0, float_string); // 10W to 999W
		lcd.print(float_string);
		lcd.print(" W ");
	}
	else if (level_CH_1 >= 60)
	{
		lcd.clear();
		delay(100);
		lcd.print(" Power 1P > 999W  ");
		lcd.setCursor(0, 1);
		lcd.print(" is out of range  ");
	}
}


void select_attenuator_CH_2()
{
	bool upPressed = (button_UP.update() && button_UP.fell());
	bool downPressed = (button_DOWN.update() && button_DOWN.fell());

	if (upPressed || downPressed)
	{
		if (downPressed)
			att_CH2--;
		else if (upPressed)
			att_CH2++;

		dtostrf(att_CH2, 2, 0, float_string); // format number to 2 digits, no nr. behind the dec.point
		lcd.setCursor(11, 0);
		lcd.print(float_string);
	}

	if (att_CH2_prev != att_CH2)
	{
		EEPROM.write(2, att_CH2);
		att_CH2_prev = att_CH2;
	}
}

void read_output_CH_2()
{
	voltage_CH_2 = 0; // read value 1 time
	// calculate average value of 10 readings
	for (iii = 0; iii < 10; iii++)
		voltage_CH_2 = voltage_CH_2 + analogRead(A2);
	
	// 2046 = 10*1023/5V
	voltage_CH_2 = voltage_CH_2 / 2046; 
} 


void calculate_power_CH_2()
{
	// uses straight line equation: y = mmm*x + ccc -> x = (y - ccc)/mmm
	level_CH_2 = -(voltage_CH_2 - ccc) / mmm + att_CH2;
	// round and convert to integer
	level_CH_2 = floor(level_CH_2 + 0.5);
	// convert dBm to mW
	power_W_2 = pow(10, level_CH_2 / 10); 
}

void display_power_CH_2()
{
	lcd.setCursor(0, 0);					 // go to line #1
	dtostrf(level_CH_2, 3, 0, float_string); //convert float to string
	lcd.print("2L");
	lcd.print(float_string);
	lcd.print("dBm ");

	lcd.print("AT");
	dtostrf(att_CH2, 2, 0, float_string);
	lcd.print(float_string);
	lcd.print("dB");
	lcd.write(byte(0)); // custom made char_up_down

	lcd.setCursor(0, 1); // go to line #2
	lcd.print("2P");
	select_subunit_of_power_CH_2();

	dtostrf(f_ghz, 3, 1, float_string);
	lcd.print(float_string);
	lcd.print("GHz");
	lcd.write(byte(1)); // custom made char_left_right

	if ((level_CH_2 - att_CH2) < error_limit_LOW || (level_CH_2 - att_CH2) > error_limit_HIGH)
	{
		delay(300);
		lcd.setCursor(0, 0);
		lcd.print(" ");
		lcd.setCursor(0, 1);
		lcd.print(" ");
		delay(300);
	}
}

void select_subunit_of_power_CH_2() 
{
	// select unit: W, mW, µW, nW
	if (level_CH_2 < -30)
	{
		dtostrf(power_W_2 * 1000000, 4, 0, float_string); //convert float to string
		lcd.print(float_string);
		lcd.print("nW ");
	}
	else if (level_CH_2 >= -30 && level_CH_2 < 0)
	{
		dtostrf(power_W_2 * 1000, 4, 0, float_string);
		lcd.print(float_string);
		lcd.print("\xE4W "); // \xE4W = ascii code for µW
	}
	else if (level_CH_2 >= 0 && level_CH_2 < 30)
	{
		dtostrf(power_W_2, 4, 0, float_string);
		lcd.print(float_string);
		lcd.print("mW ");
	}
	else if (level_CH_2 >= 30 && level_CH_2 < 40) // 1.0W to 9.9W
	{
		dtostrf(power_W_2 / 1000, 4, 1, float_string);
		lcd.print(float_string);
		lcd.print(" W ");
	}
	else if (level_CH_2 >= 40 && level_CH_2 < 60) // 10W to 999W
	{
		dtostrf(power_W_2 / 1000, 4, 0, float_string);
		lcd.print(float_string);
		lcd.print(" W ");
	}
	else if (level_CH_2 >= 60)
	{
		lcd.clear();
		delay(100);
		lcd.print(" Power 2P > 999W  ");
		lcd.setCursor(0, 1);
		lcd.print(" is out of range  ");
	}
}

void calculate_ReturnLoss_and_SWR()
{
	Return_Loss = level_CH_1 - level_CH_2;

	if (level_CH_1 < level_CH_2)
	{
		lcd.clear();
		lcd.print("!Level  CH2 >CH1");
		lcd.setCursor(0, 1);
		lcd.print("Change  Channels");
		delay(500);
	}

	RL_linear = pow(10, Return_Loss / 20);
	SWR = (RL_linear + 1) / (RL_linear - 1);
}

void display_ReturnLoss_and_SWR()
{
	lcd.setCursor(0, 0); // go to line #1
	lcd.print("1F");
	dtostrf(level_CH_1, 3, 0, float_string); //convert float to string
	lcd.print(float_string);
	lcd.print("dBm ");

	lcd.print("RL");
	dtostrf(Return_Loss, 3, 0, float_string); //convert float to string
	lcd.print(float_string);
	lcd.print("dBm");

	lcd.setCursor(0, 1); // go to line #1
	lcd.print("2R");
	dtostrf(level_CH_2, 3, 0, float_string); //convert float to string
	lcd.print(float_string);
	lcd.print("dBm ");

	lcd.print("SWR ");
	dtostrf(SWR, 3, 1, float_string);
	lcd.print(float_string);

	if ((level_CH_1 - att_CH1) < error_limit_LOW || (level_CH_1 - att_CH1) > error_limit_HIGH)
	{
		delay(300);
		lcd.setCursor(0, 0);
		lcd.print(" ");
	}

	if ((level_CH_2 - att_CH2) < error_limit_LOW || (level_CH_2 - att_CH2) > error_limit_HIGH)
	{
		lcd.setCursor(0, 1);
		lcd.print(" ");
		delay(300);
	}
}
