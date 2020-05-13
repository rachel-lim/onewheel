#include "battery.h"
#include "fsr.h"
#include "leds.h"
#include "motor.h"
#include "IMU.h"

double boardAngle = 0;
double error = 0;
double time = micros();
double deltaTime = 0;
double errorAccumulated = 0;
double previousError = 0;
double deltaError = 0;
double kP = 0.1;
double kI = 0;
double iMax = 10;
double kD = 0;
double kF = 0;
double motorOutput = 0;

// motor parameters
double maxCurrent = 15; // amps
double maxSpeed = 25; // f/s
double wheelDiameterIn = 10;
double maxRPM = maxSpeed*12*60/3.14/wheelDiameterIn;

enum BoardState{
	booting,
	waitingForRider,
	riding,
	limpMode,
	slowStop,
	detectedError
};

BoardState boardState = booting;


void setup() {
	boardState = booting;
  Serial.begin(115200);
	// initialize LEDs

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

	boardState = waitingForRider;
}


void loop() {
	// run updates of sensors, etc
  updateIMU();
  Serial1.print("p: ");
  Serial1.print(getBoardPitch());
  Serial1.print(" r: ");
  Serial1.print(getBoardRoll());
  Serial1.print(" y: ");
  Serial1.println(getBoardHeading());

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

}

void doWaitForRider() {
	// slow pulse LEDs
	// monitor FSRs
	if(isFrontFSRTriggered && isRearFSRTriggered) {
		if(abs(boardAngle) < 10) {
			boardState = riding;
		}
	}
}

void doRiding() {
	double motorSpeed = calcMotorOutput(getBoardPitch());
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
	error = _boardAngle;
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
	motorOutput = kP*error + kI*errorAccumulated + kD*deltaError/deltaTime;
}

template <typename type>
type sign(type value) {
 return type((value>0)-(value<0));
}
