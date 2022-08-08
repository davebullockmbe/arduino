#ifndef AMT22_h
#define AMT22_h

/*

 * Arduino Pin Connections
 * SPI Chip Select Enc 0:   Pin  2
 * SPI Chip Select Enc 1:   Pin  3
 * SPI MOSI                 Pin 11
 * SPI MISO                 Pin 12
 * SPI SCLK:                Pin 13
 * 
 * 
 * AMT22 Pin Connections
 * Vdd (5V):                Pin  1
 * SPI SCLK:                Pin  2
 * SPI MOSI:                Pin  3
 * GND:                     Pin  4
 * SPI MISO:                Pin  5
 * SPI Chip Select:         Pin  6
 * 
 */


/* Include the SPI library for the arduino boards */
#include <SPI.h>

/* Serial rates for UART */
#define BAUDRATE        115200

/* SPI commands */
#define AMT22_NOP       0x00
#define AMT22_RESET     0x60
#define AMT22_ZERO      0x70

/* Define special ascii characters */
#define NEWLINE         0x0A
#define TAB             0x09

/* We will use these define macros so we can write code once compatible with 12 or 14 bit encoders */
#define RES12           12
#define RES14           14


class AMT22
{
	uint8_t attempts;
	uint16_t encoderPosition;

	uint8_t csPin;
	uint8_t sclkPin;
	uint8_t misoPin;
	uint8_t mosiPin;

		
	/*
	* This function gets the absolute position from the AMT22 encoder using the SPI bus. The AMT22 position includes 2 checkbits to use
	* for position verification. Both 12-bit and 14-bit encoders transfer position via two bytes, giving 16-bits regardless of resolution.
	* For 12-bit encoders the position is left-shifted two bits, leaving the right two bits as zeros. This gives the impression that the encoder
	* is actually sending 14-bits, when it is actually sending 12-bit values, where every number is multiplied by 4. 
	* This function takes the pin number of the desired device as an input
	* This funciton expects res12 or res14 to properly format position responses.
	* Error values are returned as 0xFFFF
	*/
	uint16_t getPositionSPI(uint8_t resolution)
	{
		uint16_t currentPosition;       //16-bit response from encoder
		bool binaryArray[16];           //after receiving the position we will populate this array and use it for calculating the checksum

		//get first byte which is the high byte, shift it 8 bits. don't release line for the first byte
		currentPosition = spiWriteRead(AMT22_NOP, false) << 8;   

		//this is the time required between bytes as specified in the datasheet.
		//We will implement that time delay here, however the arduino is not the fastest device so the delay
		//is likely inherantly there already
		delayMicroseconds(3);

		//OR the low byte with the currentPosition variable. release line after second byte
		currentPosition |= spiWriteRead(AMT22_NOP, true);        

		//run through the 16 bits of position and put each bit into a slot in the array so we can do the checksum calculation
		for(int i = 0; i < 16; i++) binaryArray[i] = (0x01) & (currentPosition >> (i));

		//using the equation on the datasheet we can calculate the checksums and then make sure they match what the encoder sent
		if ((binaryArray[15] == !(binaryArray[13] ^ binaryArray[11] ^ binaryArray[9] ^ binaryArray[7] ^ binaryArray[5] ^ binaryArray[3] ^ binaryArray[1]))
				&& (binaryArray[14] == !(binaryArray[12] ^ binaryArray[10] ^ binaryArray[8] ^ binaryArray[6] ^ binaryArray[4] ^ binaryArray[2] ^ binaryArray[0])))
		{
			//we got back a good position, so just mask away the checkbits
			currentPosition &= 0x3FFF;
		}
		else
		{
			currentPosition = 0xFFFF; //bad position
		}

		//If the resolution is 12-bits, and wasn't 0xFFFF, then shift position, otherwise do nothing
		if ((resolution == RES12) && (currentPosition != 0xFFFF)) 
			currentPosition = currentPosition >> 2;

		return currentPosition;
	}

	/*
	* This function does the SPI transfer. sendByte is the byte to transmit. 
	* Use releaseLine to let the spiWriteRead function know if it should release
	* the chip select line after transfer.  
	* This function takes the pin number of the desired device as an input
	* The received data is returned.
	*/
	uint8_t spiWriteRead(uint8_t sendByte, uint8_t releaseLine)
	{
		//holder for the received over SPI
		uint8_t data;

		//set cs low, cs may already be low but there's no issue calling it again except for extra time
		setCSLine(LOW);

		//There is a minimum time requirement after CS goes low before data can be clocked out of the encoder.
		//We will implement that time delay here, however the arduino is not the fastest device so the delay
		//is likely inherantly there already
		delayMicroseconds(3);

		//send the command  
		data = SPI.transfer(sendByte);
		delayMicroseconds(3); //There is also a minimum time after clocking that CS should remain asserted before we release it
		setCSLine(releaseLine); //if releaseLine is high set it high else it stays low
		
		return data;
	}

	void setCSLine (uint8_t csLine)
	{
		digitalWrite(this->csPin, csLine);
	}

	/*
	* The AMT22 bus allows for extended commands. The first byte is 0x00 like a normal position transfer, but the 
	* second byte is the command.  
	* This function takes the pin number of the desired device as an input
	*/
	void setZeroSPI()
	{
		spiWriteRead(AMT22_NOP, false);

		//this is the time required between bytes as specified in the datasheet.
		//We will implement that time delay here, however the arduino is not the fastest device so the delay
		//is likely inherantly there already
		delayMicroseconds(3); 
		
		spiWriteRead(AMT22_ZERO, true);
		delay(250); //250 second delay to allow the encoder to reset
	}

	/*
	* The AMT22 bus allows for extended commands. The first byte is 0x00 like a normal position transfer, but the 
	* second byte is the command.  
	* This function takes the pin number of the desired device as an input
	*/
	void resetAMT22()
	{
		this->spiWriteRead(AMT22_NOP, false);

		//this is the time required between bytes as specified in the datasheet.
		//We will implement that time delay here, however the arduino is not the fastest device so the delay
		//is likely inherantly there already
		delayMicroseconds(3); 
		
		spiWriteRead(AMT22_RESET, true);
		
		delay(250); //250 second delay to allow the encoder to start back up
	}

public:
	AMT22(uint8_t csPin)
	{
		attempts = 0;
		encoderPosition = 0;

		this->csPin = csPin;
		pinMode(csPin, OUTPUT);
		
		// Set the CS line high which is the default inactive state
		setCSLine(HIGH);

		//set the clockrate. Uno clock rate is 16Mhz, divider of 32 gives 500 kHz.
		//500 kHz is a good speed for our test environment
		//SPI.setClockDivider(SPI_CLOCK_DIV2);   // 8 MHz
		//SPI.setClockDivider(SPI_CLOCK_DIV4);   // 4 MHz
		//SPI.setClockDivider(SPI_CLOCK_DIV8);   // 2 MHz
		//SPI.setClockDivider(SPI_CLOCK_DIV16);  // 1 MHz
		SPI.setClockDivider(SPI_CLOCK_DIV32);    // 500 kHz
		//SPI.setClockDivider(SPI_CLOCK_DIV64);  // 250 kHz
		//SPI.setClockDivider(SPI_CLOCK_DIV128); // 125 kHz
		
		//start SPI bus
		SPI.begin();
	}

	// returns the position of the encoder
	// 0xFFFF indicates a read error
	uint16_t getPosition()
	{
		attempts = 0;
		encoderPosition = 0xFFFF;

		// Make 3 attempts to read position
		while (encoderPosition == 0xFFFF && attempts++ < 3)
			encoderPosition = getPositionSPI(RES14);
		
		return encoderPosition;
	}

};

#endif