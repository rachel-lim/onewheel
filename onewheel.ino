#include "util.h"
#include "MathUtil.h"

double boardAngle = 0;
double error = 0;
double time = micros();
double deltaTime = 0;
double errorAccumulated = 0;
double previousError = 0;
double deltaError = 0;
volatile double kP = 500;
volatile double kI = 0;
double iMax = 10;
volatile double kD = 0;
double kF = 0;
double motorOutput = 0;
double calcedMotorOutput = 0;
double allowableChangePerCycle = 10; // rpm
double prevMotorSpeed = 0;

// motor parameters
double maxCurrent = 15; // amps
double maxSpeed = 25; // f/s
double wheelDiameterIn = 10;
double maxRPM = maxSpeed*12*60/3.14/wheelDiameterIn;

bool hasStartedRiding = false;
bool invertLEDsOnStartup = true;

enum BoardState{
	booting,
	waitingForRider,
	riding,
	limpMode,
	slowStop,
	detectedError
};

BoardState boardState = booting;
BoardState prevBoardState = booting;

// comment out when ready for logging
#define LOG_PORT SERIAL_PORT_USBVIRTUAL

#include "IMU.h"
#include "battery.h"
#include "fsr.h"
#include "leds.h"
#include "motor.h"


void setup() {
	boardState = booting;
  LOG_PORT.begin(115200);
	// initialize LEDs

  setupLEDs();
	// quick flash LEDs
	for(int i=0; i<3; i++) {
    setFrontLEDs(100,0,100);
    setRearLEDs(100,0,100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    setFrontLEDs(0,0,0);
    setRearLEDs(0,0,0);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
	}
 

	// start LEDs chasing
	// initialize IMU
  setupIMU();
	// intialize VESC
  setupMotor();

	boardState = waitingForRider;
}


void loop() {
	// run updates of sensors, etc
  updateIMU();
  updateBattVoltage(false);
  updateFSRs();
  //updateMotor();
  //LOG_PORT.print(millis());
  LOG_PORT.print(", p: ");
  LOG_PORT.print(getBoardPitch());
  /*LOG_PORT.print(" r: ");
  LOG_PORT.print(getBoardRoll());
  LOG_PORT.print(" y: ");
  LOG_PORT.print(getBoardHeading());*/
  //LOG_PORT.print("board state: ");
  //LOG_PORT.println(boardState);

  if(LOG_PORT.available()) {
    mySerialEvent();
  }
  
	switch (boardState) {
	    case waitingForRider:
	      doWaitForRider();
	      break;
	    case riding:
	      doRiding();
	      break;
	    case limpMode:
	      // do something
	      break;
	    case slowStop:
	      // do something
	      break;
	    case detectedError:
	      // do something
	      break;
	    default:
	      boardState = detectedError;
	}

  updateLEDs();
  LOG_PORT.println();
  prevBoardState = boardState;

}



void doWaitForRider() {
	// slow pulse LEDs
	/*if(isFSRTriggered(FRONT_FSR_PIN) && isFSRTriggered(REAR_FSR_PIN)) {
    for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
      leds[i] = CRGB::Green;
      leds2[i] = CRGB::Green;
    }
  } else if(isFSRTriggered(FRONT_FSR_PIN) || isFSRTriggered(REAR_FSR_PIN)) {
    for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
      leds[i] = CRGB::Blue;
      leds2[i] = CRGB::Blue;
    }
  } else { //TODO: Replace with red pulsing
    for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
      leds[i] = CRGB::Red;
      leds2[i] = CRGB::Red;
    }
  } */
	
	// monitor FSRs
	if(isFrontFSRTriggered && isRearFSRTriggered) {
		if(abs(boardAngle) < 10) {
			boardState = riding;
      
		}
	}

  hasStartedRiding = false;
  prevMotorSpeed = 0;
}


int missingFootCounter = 0;
int missingFootMaxCount = 10;

void doRiding() {
  // both feet are not firmly on the board
  if(!isFrontFSRTriggered || !isRearFSRTriggered) {
    missingFootCounter++;
  } else {
    missingFootCounter = 0;
  }

  // rider appears to have fallen off
  if(missingFootCounter > missingFootMaxCount) {
    LOG_PORT.println("!!!RIDER MISSING FEET!!!   returning to waiting for rider");
    boardState = waitingForRider;
    setMotorRPM(0);
    return;
  }

  /*if(invertLEDsOnStartup) {
    if(getBoardPitch() > 0) {
      setFrontLEDs(255,255,255);
      setRearLEDs(255,0,0);
    } else {
      setFrontLEDs(255,0,0);
      setRearLEDs(255,255,255);
    }
  } else {
    if(getBoardPitch() < 0) {
      setFrontLEDs(255,255,255);
      setRearLEDs(255,0,0);
    } else {
      setFrontLEDs(255,0,0);
      setRearLEDs(255,255,255);
    }
  } */

  if(!hasStartedRiding) {
    if(abs(getBoardPitch()) > 5) {
      invertLEDsOnStartup = true;
      return;
    } else {
      hasStartedRiding = true;
      invertLEDsOnStartup = false;
    }
  }


  
	double motorSpeed = calcMotorOutput(getBoardPitch());
  if(abs(prevMotorSpeed - motorSpeed) > allowableChangePerCycle) {
    motorSpeed = prevMotorSpeed + (allowableChangePerCycle*sign(motorSpeed - prevMotorSpeed));
    
    /*if(motorSpeed > prevMotorSpeed) {
      motorSpeed = prevMotorSpeed + allowableChangePerCycle;
    } else {
      motorSpeed = prevMotorSpeed - allowableChangePerCycle;
    } */
  }
 
  setMotorRPM(motorSpeed);
  prevMotorSpeed = motorSpeed;
  //double motorCurrent = calcMotorOutput(getBoardPitch());
  //setMotorCurrent(motorCurrent);
  
  LOG_PORT.print(", rpm: ");
  LOG_PORT.print(motorSpeed);
}

void doLimpMode() {

}

void doSlowStop() {

}

void doError() {

}



bool isVESCconnected() {
	// TODO
	return true;
}

bool isIMUworking() {
	// TODO
	return true;
}

double calcMotorOutput(double _boardAngle) {
	// current system state
	previousError = error;
	error = squareMaintainSign(_boardAngle*0.01)*100;
	deltaError = error - previousError;

	deltaTime = micros() - time;
	time = micros();

	errorAccumulated += error;
	if(abs(errorAccumulated) > iMax) {
		errorAccumulated = iMax * sign(errorAccumulated);
	}
	if(sign(error) != sign(previousError)){
		errorAccumulated = 0;
	}

	// calculate output
	return motorOutput = kP*error + kI*errorAccumulated + kD*deltaError/deltaTime;
}



bool shouldRide() {
  
}

void mySerialEvent() {
  String data = "";
  data.reserve(200); // reserve 200 bytes for string

  delay(50); // used to ensure the full serial line has appeared and is available

  while (LOG_PORT.available()) {
      // get the new byte:
      char inChar = (char)LOG_PORT.read();
      // add it to the myString:
      data += inChar;
  }
  
  switch(data.charAt(0)) {
    case 'p':
      kP = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("kP: ");
      LOG_PORT.println(kP);
      break;

    case 'i':
      kI = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("kI: ");
      LOG_PORT.println(kI);
      break;

    case 'd':
      kD = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("kD: ");
      LOG_PORT.println(kD);
      break;

    case 'a':
      allowableChangePerCycle = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("accel: ");
      LOG_PORT.println(allowableChangePerCycle);
      break;

    case 'm':
      iMax = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("iMax: ");
      LOG_PORT.println(iMax);
      break;
      
    default:
      LOG_PORT.print("input data not processed: ");
      LOG_PORT.println(data);
  }
}

int indexOfFirstNumber(String data) {
  for(int i=0; i<data.length(); i++) {
    if(isDigit(data.charAt(i))) {
      return i;
    }
  }

  // no numbers found
  return -1;
}

template <typename type>
type sign(type value) {
 return type((value>0)-(value<0));
}
