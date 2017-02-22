/*
  Project O-Vac 
  Accelerometer datalogger
  contributors: 
  Aaron Ramirez (airamire@ucsc.edu)
  Garrett Ding (gding@ucsc.edu)
  Garrett Stoll(gstoll@uccs.edu)              
  Josh Gutterman (jwgutter@uccs.edu)

 Circuit:
 ***I2C Connections***
  SDA - A4
  SCL - A5
 *** 
 *** SD card attached to SPI bus as follows**
  DI(MOSI) - pin 11
  DO(MISO) - pin 12
  SCK(CLK) - pin 13
  CS - pin 10 
 *** 
 */

#include <SPI.h>
#include <SD.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
/*
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
*/
unsigned long previousMillis = 0;        // the last time the Data was pulled

// constants won't change :
const long interval = 50;           // interval at which to collect data

MPU6050 accelgyro(0x68); // <-- use for AD0 high
int16_t ax, ay, az; //accelerometer variables
int16_t ax_offset = 887;
int16_t ay_offset = -487;
int16_t az_offset = 5437;
//int16_t gx, gy, gz; //gyroscope variables
// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO

const int chipSelect = 10; //CS pin assignment

void setup() {
  // Open serial communications and wait for port to open:
  Wire.begin(); //begin I2C wire comm
  Serial.begin(38400); //can also be 9600
  accelgyro.initialize();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  //SD Card Test
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  //I2C device initialization
  Serial.println("Initializing MPU6050...");
  //Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed"); 
  
  if (!accelgyro.testConnection()) {
    Serial.println("MPU6050 connection failed");
    // don't do anything more:
    return;
  }
  Serial.println("MPU6050 initialized.");
  accelgyro.setXAccelOffset(ax_offset);
  accelgyro.setYAccelOffset(ay_offset);
  accelgyro.setZAccelOffset(az_offset);
  // make a string for assembling the data to log:
  String dataString = "Working!";

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("test.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
    return;
  }
}

void loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
    // save the last time you collected data
    previousMillis = currentMillis;
    accelgyro.getAcceleration(&ax, &ay, &az);
    String dataString = "Accel Data: ax";
    File dataFile = SD.open("test.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      // print to the serial port too:
      #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        Serial.print(dataString);
    #endif 
    }
  }
}









