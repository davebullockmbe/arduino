//Pins of the 7 segment display
#define MAX7219_Data_IN 2
#define MAX7219_Chip_Select  3
#define MAX7219_Clock 4

#define btn_Pin 10

// Sensor_Front_Pin 8
// Sensor_Rear_Pin 9

#define buzzer_Pin 6

// Distance between sensors in metres
#define distance_between 0.59

unsigned long counter_1;
unsigned long current_time;
unsigned long time_interval;

byte last_CH1_state;
byte last_CH2_state;
float speed;
float last_speed;

bool but_state = false;


//Function to send bytes to the 7 seg display
void shift(byte send_to_address, byte send_this_data)
{
	digitalWrite(MAX7219_Chip_Select, LOW);
	shiftOut(MAX7219_Data_IN, MAX7219_Clock, MSBFIRST, send_to_address);
	shiftOut(MAX7219_Data_IN, MAX7219_Clock, MSBFIRST, send_this_data);
	digitalWrite(MAX7219_Chip_Select, HIGH);
}

void setup() 
{
	Serial.begin(9600);

	pinMode(8, INPUT);
	pinMode(9, INPUT);
	PCICR |= (1 << PCIE0);    //enable PCMSK0 scan  
	PCMSK0 |= (1 << PCINT0);  //Set pin D8 trigger an interrupt on state change. 
	PCMSK0 |= (1 << PCINT1);  //Set pin D9 trigger an interrupt on state change.      

	pinMode(btn_Pin, INPUT_PULLUP);

	pinMode(buzzer_Pin, OUTPUT);
	digitalWrite(buzzer_Pin,LOW);
	
	pinMode(MAX7219_Data_IN, OUTPUT);
	pinMode(MAX7219_Chip_Select, OUTPUT);
	pinMode(MAX7219_Clock, OUTPUT);
	digitalWrite(MAX7219_Clock, HIGH);
	delay(200);

	// Setup of MAX7219 chip
	shift(0x0f, 0x00); //display test register - test mode off
	shift(0x0c, 0x01); //shutdown register - normal operation
	shift(0x0b, 0x07); //scan limit register - display digits 0 thru 7
	shift(0x0a, 0x0f); //intensity register - max brightness
	shift(0x09, 0xff); //decode mode register - CodeB decode all digits
	
	reset_screen();
}

void loop() {
	speed = distance_between / (time_interval / 1E6);
	if(speed != last_speed)
	{
		last_speed = speed;

		display(speed);
		tone(buzzer_Pin, 1000, 300);
	}

	if(!digitalRead(btn_Pin) && but_state)
	{
		but_state = false;
		reset_screen();
		tone(buzzer_Pin, 1500, 1000);
	}
	else if(digitalRead(btn_Pin) && !but_state)
	{
		but_state = true;
	}
}


ISR(PCINT0_vect){
	current_time = micros();
	
	// Read Pin 8
	if(PINB & B00000001){
		if(last_CH1_state == 0){
			last_CH1_state = 1;
			counter_1 = current_time;
		}
	}
	else if(last_CH1_state == 1){
		last_CH1_state = 0;
	}

	// Read Pin 9
	if(PINB & B00000010 ){                                   
		if(last_CH2_state == 0){                                               
			last_CH2_state = 1;                                                   
			time_interval = current_time - counter_1;                                    
		}
	}
	else if(last_CH2_state == 1){                                           
		last_CH2_state = 0;                                                     
	}
}

void reset_screen() {
	shift(0x08, 0x00);
	shift(0x07, 0x00);
	shift(0x06, 0x00);
	shift(0x05, 0x00);
	shift(0x04, 0x00);
	shift(0x03, 0x00);
	shift(0x02, 0x00);
	shift(0x01, 0x00);
}

void display(double value)
{
	Serial.print("Displaying: ");
	Serial.println(value);

	byte digit_1 = value / 100000;
	value -= digit_1 * 100000;

	byte digit_2 = value / 10000;
	value -= digit_2 * 10000;

	byte digit_3 = value / 1000;
	value -= digit_3 * 1000;

	byte digit_4 = value / 100;
	value -= digit_4 * 100;

	byte digit_5 = value / 10;
	value -= digit_5 * 10;

	byte digit_6 = value;
	value -= digit_6;
	digit_6 |= 0b10000000; // turn DP on

	value *= 10;
	byte digit_7 = value;
	value -= digit_7;

	value *= 10;
	byte digit_8 = value;	

	shift(0x08, digit_1);
	shift(0x07, digit_2);
	shift(0x06, digit_3);
	shift(0x05, digit_4);
	shift(0x04, digit_5);
	shift(0x03, digit_6);
	shift(0x02, digit_7);
	shift(0x01, digit_8);	
}
