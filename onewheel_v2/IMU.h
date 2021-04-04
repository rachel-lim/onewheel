#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define BNO055_SAMPLERATE_DELAY_MS (1);
Adafruit_BNO055 bno = Adafruit_BNO055(55);

double boardRoll = 0;
double boardHeading = 0;

float getBoardPitch();
float getBoardRoll();
float getBoardHeading();


void setupIMU() {
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);
}


void updateIMU(bool shouldPrint) {
   /* Get a new sensor event */ 
  sensors_event_t event; 
  bno.getEvent(&event);

  boardAngle = event.orientation.y;
  boardRoll = event.orientation.z;
  boardHeading = event.orientation.x;

  if(shouldPrint) {
    LOG_PORT.print(", p: ");
    LOG_PORT.print(getBoardPitch());
    LOG_PORT.print(", r: ");
    LOG_PORT.print(getBoardRoll());
    LOG_PORT.print(", y: ");
    LOG_PORT.print(getBoardHeading());
  }
}

float getBoardPitch() {
  return boardAngle;
}

float getBoardRoll() {
  return boardRoll;
}

float getBoardHeading() {
  return boardHeading;
}
