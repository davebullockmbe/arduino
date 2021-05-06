#include <SPI.h>
#include <SD.h>

const int chipSelect = 4;
const int analogPin = 2;

File dataFile;

void setup() {
    // Set serial baud to 9600
    Serial.begin(9600);

    Serial.print("Initializing SD card...");

    // mount the SD card on SPI with the given CS pin
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        return;
    }

    Serial.println("Card initialized.");

    // open a file in overwrite mode
    dataFile = SD.open("datalog.txt", FILE_WRITE);

    // if we failed to open the file, complain
    if(!dataFile)
        Serial.println("error opening datalog.txt");
}

void loop() {
    // return early if datafile isnt open/ready
    if(!dataFile)
        return;

    // read analog pin
    int sensor = analogRead(analogPin);

    // print it to the file
    dataFile.println(sensor);
    // flush the changes
    dataFile.flush();

    // also print to the serial port for debug
    Serial.println(sensor);

    // delay 200ms
    delay(200);
}
