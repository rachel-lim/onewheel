/*
  Name:    getVescValues.ino
  Created: 19-08-2018
  Author:  SolidGeek
  Description:  This example is made using a Arduino Micro (Atmega32u4) that has a HardwareSerial port (Serial1) seperated from the Serial port. 
                A Arduino Nano or Uno that only has one Serial port will not be able to display the data returned.
*/
#include <util.h>
#include <VescUart.h>

#define Serial SERIAL_PORT_USBVIRTUAL

volatile boolean enabled = true;
volatile float motorSpeed = 0.0;
int VESCfailCounter = 0;
int VESCmaxFailCount = 10;

/** Initiate VescUart class */
VescUart UART;

void setup() {

  /** Setup Serial port to display data */
  Serial.begin(9600);

  /** Setup UART port (Serial1 on Atmega32u4) */
  Serial1.begin(115200);
  
  while (!Serial1) {;}

  /** Define which ports to use as UART */
  UART.setSerialPort(&Serial1);
}

void loop() {
  
  /** Call the function getVescValues() to acquire data from VESC */
  if ( UART.getVescValues() ) {

    Serial.println(UART.data.rpm);
    Serial.println(UART.data.inpVoltage);
    Serial.println(UART.data.ampHours);
    Serial.println(UART.data.tachometerAbs);
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
  }

  if(enabled) {
    UART.setRPM(motorSpeed);
  } else {
    UART.setRPM(0);
  }

  delay(50);
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
