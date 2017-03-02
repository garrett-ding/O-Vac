#include <SPI.h>
#include <SD.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 accelgyro(0x68); // <-- use for AD0 high
int16_t ax = 0, ay = 0, az = 0; //accelerometer variables
int16_t ax_offset = 887;
int16_t ay_offset = -487;
int16_t az_offset = 5437;
int16_t i = 0, j = 0;
int pressurePin = 0, voltage_int = 0;
float voltage = 0;
char buff[30];

const int chipSelect = 10;

void setup() {
  Serial.begin(38400);
  delay(2000);
  pinMode(A0, INPUT);
  while (!Serial){
  }

  // Open serial communications and wait for port to open:
  Wire.begin(); //begin I2C wire comm
  accelgyro.initialize();
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
  // put your setup code here, to run once:
//set timer0 interrupt at 100Hz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 100hz increments
  OCR0A = 156;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS02 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

// set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 7812;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  //interrupts();
}

void loop() {
  accelgyro.getAcceleration(&ax, &ay, &az);
  // put your main code here, to run repeatedly:
}

ISR(TIMER1_COMPA_vect){
    i++;
  pressurePin = analogRead(A0);
  voltage = (5.0 / 1023.0) * pressurePin;
  
//  Serial.print("Raw ADC: ");
//  Serial.println(pressurePin);
//  Serial.print("voltage:");
  Serial.print("X: ");
  Serial.print(ax);
  Serial.print("Y: ");
  Serial.print(ay);
  Serial.print("Z: ");
  Serial.println(az);
//  Serial.println(voltage);
    if (i == 5){
//      if (OCR1A == 15624) OCR1A = 7812;
//      else OCR1A = 15624;
      //Serial.println("5 seconds");
      i = 0;
    }
    else
      ;//Serial.println(i);
}

ISR(TIMER0_COMPA_vect){
    //Serial.println(j);
    j++;
    if (j == 100){
      j = 0;
      //Serial.println("1 second on 100Hz");
    }
}
