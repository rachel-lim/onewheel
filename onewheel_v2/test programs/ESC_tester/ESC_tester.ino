/*
  Name:    getVescValues.ino
  Created: 19-08-2018
  Author:  SolidGeek
  Description:  This example is made using a Arduino Micro (Atmega32u4) that has a HardwareSerial port (Serial1) seperated from the Serial port. 
                A Arduino Nano or Uno that only has one Serial port will not be able to display the data returned.
*/
//#include <util.h>
#include <VescUart.h>

//#define Serial SERIAL_PORT_USBVIRTUAL

volatile boolean enabled = true;
volatile float motorSpeed = 0.0;
int VESCfailCounter = 0;
int VESCmaxFailCount = 10;


const int motorPolePairs = 15;


/** Initiate VescUart class */
VescUart UARTVesc;

// timing variables
long startTime = 0;
long endTime = 0;
int timingCycleCounter = 0;
int TIMED_CYCLES = 100;
long avgTime = 0;
long totalTime = 0;
float frequency = 0; // Hz

float rpm = 0;
float voltage = 0;
float ampHrs = 0;
float tachAbs = 0;


void setup() {

  /** Setup Serial port to display data */
  Serial.begin(9600);

  /** Setup UART port (Serial1 on Atmega32u4) */
  Serial1.begin(115200);
  
  while (!Serial1) {;}

  /** Define which ports to use as UART */
  UARTVesc.setSerialPort(&Serial1);
}

void loop() {
  if(timingCycleCounter == 0) { // first run
    startTime = micros();
    timingCycleCounter++;
  } else if(timingCycleCounter >= TIMED_CYCLES) { // finished test, report output and reset
    endTime = micros();
    totalTime = endTime - startTime;
    frequency = (float)TIMED_CYCLES / totalTime * 1000000;
    avgTime = totalTime / TIMED_CYCLES;
    Serial.print(TIMED_CYCLES);
    Serial.print(" cycles averaged ");
    Serial.print(avgTime);
    Serial.print(" microseconds per loop resulting in ");
    Serial.print(frequency);
    Serial.println(" Hz");

    Serial.print(rpm);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.print(", ");
    Serial.print(ampHrs);
    Serial.print(", ");
    Serial.println(tachAbs); 
    timingCycleCounter = 0;
  } else { // middle of testing
    timingCycleCounter++;
  }
  
  /** Call the function getVescValues() to acquire data from VESC */
  /*if ( UARTVesc.getVescValues() ) {

    /*Serial.print(UARTVesc.data.rpm / motorPolePairs);
    Serial.print(", ");
    Serial.print(UARTVesc.data.inpVoltage);
    Serial.print(", ");
    Serial.print(UARTVesc.data.ampHours);
    Serial.print(", ");
    Serial.println(UARTVesc.data.tachometerAbs); */
 /*   rpm = UARTVesc.data.rpm / motorPolePairs;
    voltage = UARTVesc.data.inpVoltage;
    ampHrs = UARTVesc.data.ampHours;
    tachAbs = UARTVesc.data.tachometerAbs;
    VESCfailCounter = 0;
  }
  else
  {
    Serial.println("Failed to get data!");
    VESCfailCounter++;
    if(VESCfailCounter >=10) {
      Serial1.begin(115200);
      Serial.println("RESETING VESC CONNECTION");
      VESCfailCounter = 0;
    }
  } */

  if(enabled) {
    UARTVesc.setDuty(motorSpeed);
  } else {
    UARTVesc.setDuty(0);
  }

  if(Serial.available()) {
    SerialEvent();
  }

  //delay(50);
}


void SerialEvent() {
  // capture string and normalize
  String input = onSerialEvent(50);
  input.toLowerCase();

  /*if(input == "enable") {
    enabled = true;
  } else if(input == "disable" || input == "d") {
    enabled = false;
  } else if(input == "reset") {
    motorSpeed = 0;
  } else {
    motorSpeed = input.toDouble();
  }*/
  motorSpeed = input.toDouble();

  Serial.println(input);
 
}

/**
 * Designed to run in serialEvent to retrieve the entered text
 * @Param miliDelay this is how long after the event is triggered 
 * before we start reading. Make this longer for larger text inputs
 */
String onSerialEvent(int miliDelay) {
  String myString = "";
  myString.reserve(200); // reserve 200 bytes for string

  delay(miliDelay); // used to ensure the full serial line has appeared and is available

  while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read();
      // add it to the myString:
      myString += inChar;
  }
  return myString;
}
