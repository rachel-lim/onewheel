#include "util.h"
#include "MathUtil.h"

double boardAngle = 0;
double error = 0;
//double time = micros();
double deltaTime = 0;
double prevTime = 0;
double errorAccumulated = 0;
double previousError = 0;
double deltaError = 0;
volatile double kP = 500;
volatile double kI = 0;
double iMax = 10;
volatile double kD = 0;
double currentSpeed = 0;
double throttlePedal = 0.5;
double overallGain = 0.35;
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
  justStopped,
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
  //LOG_PORT.print(", p: ");
  //LOG_PORT.print(getBoardPitch());
  //LOG_PORT.print(boardAngle);
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
      case justStopped:
        doJustStopped();
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
  //LOG_PORT.println();
  prevBoardState = boardState;

  //runFrequencyTimingCheck();
}


// timing variables
long startTime = 0;
long endTime = 0;
int timingCycleCounter = 0;
int TIMED_CYCLES = 100;
long avgTime = 0;
long totalTime = 0;
float frequency = 0; // Hz

void runFrequencyTimingCheck() {
  if(timingCycleCounter == 0) { // first run
    startTime = micros();
    timingCycleCounter++;
  } else if(timingCycleCounter >= TIMED_CYCLES) { // finished test, report output and reset
    endTime = micros();
    totalTime = endTime - startTime;
    frequency = (float)TIMED_CYCLES / totalTime * 1000000;
    avgTime = totalTime / TIMED_CYCLES;
    LOG_PORT.print(TIMED_CYCLES);
    LOG_PORT.print(" cycles averaged ");
    LOG_PORT.print(avgTime);
    LOG_PORT.print(" microseconds per loop resulting in ");
    LOG_PORT.print(frequency);
    LOG_PORT.println(" Hz");
    timingCycleCounter = 0;
  } else { // middle of testing
    timingCycleCounter++;
  }

}

void doWaitForRider() {
	currentSpeed = 0;
 
	// monitor FSRs
	if(isFrontFSRTriggered && isRearFSRTriggered) {
		if(abs(boardAngle) < 10) {
			boardState = riding;
      
		}
	}

  setMotorDutyCycle(0.0);

  hasStartedRiding = false;
  prevMotorSpeed = 0;
}


int missingFootCounter = 0;
int missingFootMaxCount = 100;

void doRiding() {
  // both feet are not firmly on the board
  if(!isFrontFSRTriggered || !isRearFSRTriggered) {
    missingFootCounter++;
  } else {
    missingFootCounter = 0;
  }

  // rider appears to have fallen off or touched down
  if(missingFootCounter > missingFootMaxCount) {
    LOG_PORT.println("!!!RIDER MISSING FEET!!!   returning to waiting for rider");
    boardState = justStopped;
    setMotorRPM(0);
    return;
  }

  if(abs(boardAngle) > 18) {
    LOG_PORT.println("!!!BOARD TIPPED TOO FAR!!!   returning to waiting for rider");
    boardState = justStopped;
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
    if(abs(boardAngle) > 1) {
      invertLEDsOnStartup = true;
      return;
    } else {
      hasStartedRiding = true;
      invertLEDsOnStartup = false;
    }
  }


  
	//double motorSpeed = calcMotorSpeedOutput(getBoardPitch());
  //if(abs(prevMotorSpeed - motorSpeed) > allowableChangePerCycle) {
    //motorSpeed = prevMotorSpeed + (allowableChangePerCycle*sign(motorSpeed - prevMotorSpeed));
    
    /*if(motorSpeed > prevMotorSpeed) {
      motorSpeed = prevMotorSpeed + allowableChangePerCycle;
    } else {
      motorSpeed = prevMotorSpeed - allowableChangePerCycle;
    } */
  //}
 
  //setMotorRPM(motorSpeed);

  double motorSpeed = calcMotorDutyCycle();
  
  prevMotorSpeed = motorSpeed;
  //double motorCurrent = calcMotorOutput(getBoardPitch());
  //setMotorCurrent(motorCurrent);
  setMotorDutyCycle(motorSpeed);
  
  //LOG_PORT.print(", dutyCycle: ");
  //LOG_PORT.print(motorSpeed);
}

long timeStopped = millis();
long prevTimeStopped = millis();
long timeToStop = 1000;

void doJustStopped() {
  if(millis() - prevTimeStopped > 100) { // long delay since last time we were here, assume we just stopped
    timeStopped = millis();
  }
  
  prevTimeStopped = millis();
    
  if(!isFrontFSRTriggered || !isRearFSRTriggered && (millis() > timeStopped + timeToStop)) {
    boardState = waitingForRider;
  }
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

double calcMotorSpeedOutput(double _boardAngle) {
	// current system state
	previousError = error;
	error = squareMaintainSign(_boardAngle*0.01)*100;
	deltaError = error - previousError;

	deltaTime = micros() - prevTime;
	prevTime = micros();

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

double calcMotorDutyCycle() {
  float angleRads = boardAngle*0.017453;
  float balanceTorque = (float)(4.5*angleRads) + (0.5*gyroRateRads);
  //LOG_PORT.print(", balTorque: ");
  //LOG_PORT.print(balanceTorque);
  currentSpeed = (float)(currentSpeed + (throttlePedal*balanceTorque*deltaTime*0.000001)) * 0.999;
  //LOG_PORT.print(", curSpeed: ");
  //LOG_PORT.print(currentSpeed);
  motorOutput = (balanceTorque + currentSpeed) * overallGain;
  //LOG_PORT.print(", motorOutput: ");
  //LOG_PORT.print(motorOutput);
  motorOutput = limitValue(motorOutput, 0.7, -0.7);
  return motorOutput;
  
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

    case 'o':
      overallGain = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("overallGain: ");
      LOG_PORT.println(overallGain);
      break;

    case 't':
      throttlePedal = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("throttlePedal: ");
      LOG_PORT.println(throttlePedal);
      break;

    case 'f':
      //throttlePedal = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("overallGain: ");
      LOG_PORT.println(overallGain);
      LOG_PORT.print("throttlePedal: ");
      LOG_PORT.println(throttlePedal);
      break;
      
    default:
      LOG_PORT.print("new input data not processed: ");
      LOG_PORT.println(data);
      LOG_PORT.print("overallGain: ");
      LOG_PORT.println(overallGain);
      LOG_PORT.print("throttlePedal: ");
      LOG_PORT.println(throttlePedal);
      LOG_PORT.print("boardAngle: ");
      LOG_PORT.println(boardAngle);
      LOG_PORT.print("currentSpeed: ");
      LOG_PORT.println(currentSpeed);
      LOG_PORT.print("boardState: ");
      LOG_PORT.println((String)boardState);
      
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
