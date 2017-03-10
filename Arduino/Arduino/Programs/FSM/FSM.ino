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
#define APPROX_28_PSI 0.33
#define WATER_SENSE_MIN 266
// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO
#define FREQUENCY 500                   //interrupt frequency in hertz
#define MA_WINDOW 15                    // Number of samples in the moving average window
#define BOT_THRESHOLD 20000             // Z-Aacceleration threshold for transition into Suction state
#define WAIT_TIME 1000                  //number of ISR calls until transition into data collection state
#define DATA_TIME 5000                   //number of ISR calls until transition into waiting state

MPU6050 accelgyro(0x68); // <-- use for AD0 high
int16_t ax, ay, az; //accelerometer variables
int16_t ax_offset = 957;
int16_t ay_offset = -317;
int16_t az_offset = 1606;
int16_t curr_press = 0, water_check = 0;
float   press_voltage = 0, prev_press_voltage = 0;
long id = 1, sum = 0;
int8_t pulses = 0;
uint32_t i = 0;
int16_t average = 0;
bool collect_flag = 0, dataflag = 0, wait_flag = 0;
unsigned long prev_time =0;
File dataFile;
char  info[100];
//int16_t gx, gy, gz; //gyroscope variables

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO

const int chipSelect = 10; //CS pin assignment
const int solenoidPin = 2; // Solenoid pin
const int pressurePin = 0; // Pressure pin
const int waterPin = 1; // Moisture sensor pin
const int LEDPin = 8; // Moisture sensor pin
const int LED1Pin = 7;

typedef enum {
  Wait_For_Water,
  Descending,
  Collecting,
  Ascending,
  Floating,
} states;

states state = Wait_For_Water;

void setup() {
  Serial.begin(38400); //can also be 9600
  Wire.begin(); //begin I2C wire comm
  accelgyro.initialize();
  pinMode(LEDPin, OUTPUT);
  pinMode(LED1Pin, OUTPUT);
  pinMode(solenoidPin, OUTPUT);
  pinMode(pressurePin, INPUT);
  pinMode(waterPin, INPUT);
  
// Timer code taken directly from: https://www.instructables.com/id/Arduino-Timer-Interrupts/

//set timer0 interrupt at 100Hz
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 100hz increments
  OCR0A = 156 / 5;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC modeer
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
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
//  //SD Card Test
  Serial.print("Initializing SD card...");
  
  // see if the card is present and can be initialized:

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  
//  Serial.println("card initialized.");
//
//  //I2C device initialization
  Serial.println("Initializing MPU6050...");
  //Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed"); 
  
  accelgyro.testConnection();
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
  dataFile = SD.open("test.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(", , , ,"); //Just a leading blank line, incase there was previous data
    String header = "Time(sec), Ax, Ay, Az";
    dataFile.println(header);
    dataFile.close();
    // print to the serial port too:
    //Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    //Serial.println("error opening datalog.txt");
    return;
  }
//end setup
}

void loop() {
  accelgyro.getAcceleration(&ax, &ay, &az);
  int curr_press = analogRead(A0);
  press_voltage = curr_press * 0.0049;
  
/* computation for running total accel */
  if (dataflag){
        if (id<MA_WINDOW)
          sum += az;     
        else if(id == MA_WINDOW){
          sum += az;
          average = sum/MA_WINDOW;  //compute baseline average
        }
        else
          average = ComputeMA(average, MA_WINDOW, az);
        id++;
        dataflag = 0;
  }

/* State machine, all use the average variable for acceleration to see if a state change is necessary */
  switch (state){
    case Wait_For_Water:
      Serial.println("case: Wait_for_water");
      if(average > 15500 || press_voltage > APPROX_28_PSI){
         digitalWrite(LEDPin, LOW);   //turn LED off  
         digitalWrite(LED1Pin, HIGH);   
         state = Descending;                               //Switch to Descending state 
                //dataFile.println(pressString);    
         id=0; 
         sum = 0;//reset sample counter
         average = 0;

      }
      break;
      
    case Descending: 
      // enable timer compare interrupt for data collection
      if (TIMSK1 ^ (1 << OCIE1A) ){
        TIMSK1 |= (1 << OCIE1A);
      }
      Serial.println("case: Descending");
      if(average > BOT_THRESHOLD){
         digitalWrite(LEDPin, HIGH);   //turn LED off  
         digitalWrite(LED1Pin, LOW);   
         state = Collecting;                               //Switch to Collecting state 
                //dataFile.println(pressString);  
                 //Serial.println("Entering Suction state");  
         id=0; 
         sum = 0;//reset sample counter
         average = 0;
         dataFile.close();
      }     
        //if desired amount of samples have been collected, switch states
//        if(id>=DATA_TIME){     
//            digitalWrite(LEDPin, LOW);                            //turn LED off
//            state = Wait_For_Water;                               //Switch to Waiting state
//            unsigned long cur_time = millis();                    //get current program run time
//            cur_time = cur_time - prev_time;                      //Calculate amount of time in collection state
//            dataFile.println("Entering Wait State, Time Elapsed:");
//            dataFile.println(cur_time);      
//            id=0;                                                 //reset sample counter
//            sum = 0;//reset sample counter
//            average = 0;
//            dataFile.close();
//        }
      break;
      
    case Collecting:
      if (OCR1A != 7812){ // set register to make Timer 1 2hz 
        OCR1A = 7812; 
      }
      if (collect_flag){
        Serial.println("case: Collecting");
        int pinread = digitalRead(solenoidPin);
        if (pinread == HIGH) digitalWrite(solenoidPin, LOW);
        if (pulses == 10){
          pulses = 0; // change to Ascending
          Serial.println("transitioned to Ascending");
          state = Ascending;
        }
        else 
          digitalWrite(solenoidPin, HIGH);
        pulses++;
        collect_flag = 0;
      }
      break;

    case Ascending:
      if (OCR1A != 15624){ // set register to back to make Timer 1 1hz 
        OCR1A = 15624; 
      }
      Serial.println("case: Ascending");
      if(average > BOT_THRESHOLD){
         digitalWrite(LEDPin, LOW);   //turn LED off  
         digitalWrite(LED1Pin, HIGH);   
         state = Floating;                               //Switch to Collecting state  
         id=0; 
         sum = 0;//reset sample counter
         average = 0;
      }
      break;

    case Floating:
      Serial.println("case: Floating");
      // disable timer compare interrupt, no longer needed
      TIMSK1 &= 0;
      break;
  }
}

// INT for ambient sensor data collection, set for every 1 second. Later set for 0.5 seconds for
// solenoid pulsing operation
ISR(TIMER1_COMPA_vect){
  Serial.println("in timer1"); 
  if (state == Collecting)
    collect_flag = 1;
  else if (state == Descending || state == Ascending)
    ;//dataflag = 1;
}

// INT for sensor related state changes
ISR(TIMER0_COMPA_vect){
  dataflag = 1;
}

int16_t ComputeMA(int16_t avg, int16_t n, int16_t sample){
    avg -= avg/n;
    avg += sample/n;
    return avg;    
}

