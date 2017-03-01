/*
  Project O-Vac 
  FSM w/ Accelerometer datalogger
  contributors: 
  Aaron Ramirez (airamire@ucsc.edu)
  Garrett Ding (gding@ucsc.edu)
  Garrett Stoll(gstoll@uccs.edu)              
  Josh Gutterman (jwgutter@uccs.edu)

 Uses:
 Timer1, Timer0
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
/* DEFINES */
#define FT_PER_SEC 250
#define APPROX_24_PSI 0.3

MPU6050 accelgyro(0x68); // <-- use for AD0 high
int16_t ax, ay, az; //accelerometer variables
int16_t ax_offset = 887;
int16_t ay_offset = -487;
int16_t az_offset = 5437;
int16_t curr_press = 0;
float   press_voltage = 0, prev_press_voltage = 0;
int8_t  curr_acc = 0, prev_acc = 0;
int8_t  pulses = 0;
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
const int solenoidPin = 5; // Solenoid pin
const int pressure_pin = 0;

typedef enum {
  Wait_For_Water,
  Descending,
  Collecting,
  Ascending,
  Floating,
} states;

states state = Wait_For_Water;

void setup() {
// Timer code taken directly from: https://www.instructables.com/id/Arduino-Timer-Interrupts/
cli();//stop interrupts

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
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  

/* Setup SD card
/  Open serial communications and wait for port to open: *****************************/
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
  if (!dataFile) {
    //dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    //Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
//  else {
//    Serial.println("error opening datalog.txt");
//    return;
//  }

sei();//allow interrupts
//end setup
}

void loop() {
  switch (state){
    case Wait_For_Water:
      break;
      
    case Descending:
      // enable timer compare interrupt for data collection
      if (TIMSK1 ^ (1 << OCIE1A) ){
        TIMSK1 |= (1 << OCIE1A);
      }
      break;
      
    case Collecting:
      if (OCR1A != 7812){ // set register to make Timer 1 2hz 
        OCR1A = 7812; 
      }
      break;

    case Ascending:
      if (OCR1A != 15624){ // set register to back to make Timer 1 1hz 
        OCR1A = 15624; 
      }
      break;

    case Floating:
      // disable timer compare interrupt, no longer needed
      TIMSK1 |= 0;
      break;
  }
}

// INT for ambient sensor data collection, set for every 1 second. Later set for 0.5 seconds for
// solenoid pulsing operation
ISR(TIMER1_COMPA_vect){ 
  if (state == Collecting){ // pulse 5 times on/off
    if (pulses == 10){
        pulses = 0; // change to Ascending
        state = Ascending;
    }
    int pinread = digitalRead(solenoidPin);
    if (pinread == HIGH) digitalWrite(solenoidPin, LOW);
    else digitalWrite(solenoidPin, HIGH);
    pulses++;
  }
  else if (state == Descending || state == Ascending)
    ;// Data collection code
}

// INT for sensor related state changes
ISR(TIMER0_COMPA_vect){
  accelgyro.getAcceleration(&ax, &ay, &az);
  curr_acc = (ax ^ 2) + (ay ^ 2) + (az ^ 2);
  curr_press = analogRead(pressure_pin); // get pressure
  press_voltage = (5.0/ 1023.0) * curr_press;
  
  if (state == Wait_For_Water) {
    if ((curr_acc > prev_acc + FT_PER_SEC) && (press_voltage > APPROX_24_PSI)) 
      state = Descending;
  }
  else if (state == Descending){
    if ((curr_acc - prev_acc < FT_PER_SEC) && (press_voltage - prev_press_voltage < 0.0049))
      state = Collecting;
  }
  else if (state == Ascending){
    if ((curr_acc - prev_acc < FT_PER_SEC) && press_voltage < APPROX_24_PSI) 
      state = Floating;
  }
  prev_press_voltage = press_voltage;
  prev_acc = curr_acc;
}

