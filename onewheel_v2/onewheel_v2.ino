#include "util.h"
#include "MathUtil.h"

#define LOG_PORT Serial

double boardAngle = 0;
//double angleError = 0;
//double time = micros();
//double deltaTime = 0;
//double prevTime = 0;
//double errorAccumulated = 0;
//double previousError = 0;
//double deltaError = 0;
volatile double kP = 3.25; // old default was 2.0
volatile double kPExp = 2;
bool useExponentialKp = false;
volatile double kI = 0;
double iMax = 0;
volatile double kD = 0;
double currentSpeed = 0;
//double throttlePedal = 0.0;
//double overallGain = 0;
//double motorOutput = 0;
double maxMotorDutyCycle = 1.0;
//double calcedMotorOutput = 0;
double allowableChangePerCycle = 0.01; // duty cycle
//bool useRateChangeLimit = false;

double prevMotorSpeed = 0;
int boardStartRidingAngle = 10;

// motor parameters
//double maxCurrent = 15; // amps
//double maxSpeed = 25; // f/s
//double wheelDiameterIn = 10;
//double maxRPM = maxSpeed * 12 * 60 / 3.14 / wheelDiameterIn;

bool hasStartedRiding = false;
bool invertLEDsOnStartup = true;

enum BoardState {
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


#include "IMU.h"
#include "battery.h"
#include "fsr.h"
#include "leds.h"
#include "motor.h"
#include "BLEcomms.h"

void mySerialEvent();


void setup() {
  boardState = booting;
  LOG_PORT.begin(115200);

  // initialize LEDs
  setupLEDs();

  // quick flash LEDs
  for (int i = 0; i < 3; i++) {
    setFrontAll(100, 0, 100);
    setRearAll(100, 0, 100);
    leds.show();
    leds2.show();
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    setFrontAll(0, 0, 0);
    setRearAll(0, 0, 0);
    leds.show();
    leds2.show();
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }


  // start LEDs chasing
  // initialize IMU
  setupIMU();
  // intialize VESC
  setupMotor();

  // initialize BLE
  setupBLE();

  boardState = waitingForRider;
}



void loop() {
  // run updates of sensors, etc
  updateIMU(false);
  updateBattVoltage(false);
  updateFSRs(false);
  //updateMotor();
  //LOG_PORT.print(millis());
  //LOG_PORT.print(" board state: ");
  //LOG_PORT.print(boardState);

  if (LOG_PORT.available()) {
    mySerialEvent();
  }

  //boardState = riding;
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
      break;
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
  if (timingCycleCounter == 0) { // first run
    startTime = micros();
    timingCycleCounter++;
  } else if (timingCycleCounter >= TIMED_CYCLES) { // finished test, report output and reset
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
  // update BLE stuff
  updateMotorVals(false);
  updateBLE();
  readBLEVars();
  publishRegularBLE();



  // monitor FSRs
  if (isFrontFSRTriggered && isRearFSRTriggered) {
    LOG_PORT.print(", both FSRs triggered");
    if (abs(getBoardPitch()) < boardStartRidingAngle) {
      boardState = riding;
      LOG_PORT.print(", triggering riding");
    }
  }

  //setMotorDutyCycle(0.0);
  currentSpeed = 0;

  hasStartedRiding = false;
  prevMotorSpeed = 0;
}


int missingFootMaxTime = 500;
bool wasMissingFeetLastCycle = false;
long timeFeetLost = millis();

int tippedMaxTime = 500;
bool wasTippedLastCycle = false;;
long timeTipped = millis();

int rolledMaxTime = 500;
bool wasRolledLastCycle = false;
long timeRolled = millis();

int startupStiffeningTime = 1000;
long timeStartedRiding = millis();


void doRiding() {

  // both feet are not firmly on the board
  if (!isFrontFSRTriggered || !isRearFSRTriggered) {
    if (!wasMissingFeetLastCycle) {
      wasMissingFeetLastCycle = true;
      timeFeetLost = millis();
    } else if (millis() - timeFeetLost > missingFootMaxTime) { // rider appears to have fallen off
      LOG_PORT.println("!!!RIDER MISSING FEET!!!   returning to waiting for rider");
      boardState = justStopped;
      setMotorRPM(0);
      return;
    }
  } else {
    wasMissingFeetLastCycle = false;
  }

  // if we have not yet tried to level the board, set LEDs correctly
  if (!hasStartedRiding) {
    if (abs(boardAngle) > 1) {
      invertLEDsOnStartup = true;
      return;
    } else {
      hasStartedRiding = true;
      invertLEDsOnStartup = false;
      wasMissingFeetLastCycle = false;
      wasTippedLastCycle = false;
    }
  }



  // board angle exceeds acceptable riding angle (likely fallen over)
  if (abs(boardAngle) > 18) {
    if (!wasTippedLastCycle) {
      timeTipped = millis();
      wasTippedLastCycle = true;
    } else if (millis() - timeTipped > tippedMaxTime) {
      LOG_PORT.println("!!!BOARD TIPPED TOO FAR!!!   returning to waiting for rider");
      boardState = justStopped;
      setMotorRPM(0);
      return;
    }
  } else {
    wasTippedLastCycle = false;
  }

  if (abs(getBoardRoll()) > 20) {
    if (!wasRolledLastCycle) {
      timeRolled = millis();
      wasRolledLastCycle = true;
    } else if (millis() - timeRolled > rolledMaxTime) {
      LOG_PORT.println("!!!BOARD ROLLED TOO FAR!!!   returning to waiting for rider");
      boardState = justStopped;
      setMotorRPM(0);
      return;
    }
  } else {
    wasRolledLastCycle = false;
  }

  double motorSpeed = 0;

  if(!useExponentialKp) {
    motorSpeed = kP * scaleClipped(boardAngle, -30, 30, -1.0, 1.0);
  } else {
    motorSpeed = pow(abs(boardAngle),kPExp) * 2 / 60 * sign(boardAngle);
  }
  motorSpeed = constrain(motorSpeed, -maxMotorDutyCycle, maxMotorDutyCycle);
  prevMotorSpeed = motorSpeed;
  setMotorDutyCycle(motorSpeed);
  //LOG_PORT.print(", angle: ");
  //LOG_PORT.print(boardAngle);
  //LOG_PORT.print(", dutyCycle: ");
  //LOG_PORT.println(motorSpeed);
}

long timeStopped = millis();
long prevTimeStopped = millis();
long timeToStop = 5000;

void doJustStopped() {

  setMotorDutyCycle(0.0);
  if (millis() - prevTimeStopped > 100) { // long delay since last time we were here, assume we just stopped
    timeStopped = millis();
  }

  prevTimeStopped = millis();

  if ((!isFrontFSRTriggered || !isRearFSRTriggered) && (millis() > timeStopped + timeToStop)) {
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

  switch (data.charAt(0)) {
    case 'p':
      kP = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("kP: ");
      LOG_PORT.println(kP);
      break;

    case 'e':
      //motorEnabled = !motorEnabled;
      //LOG_PORT.print("motorEnabled: ");
      //LOG_PORT.println(motorEnabled);
      break;

    case 'd':
      //kD = data.substring(indexOfFirstNumber(data)).toFloat();
      //LOG_PORT.print("kD: ");
      //LOG_PORT.println(kD);
      break;

    case 'a':
      allowableChangePerCycle = data.substring(indexOfFirstNumber(data)).toFloat();
      LOG_PORT.print("accel: ");
      LOG_PORT.println(allowableChangePerCycle);
      break;

    case 'x':
      //iMax = data.substring(indexOfFirstNumber(data)).toFloat();
      //LOG_PORT.print("iMax: ");
      //LOG_PORT.println(iMax);
      break;

    case 'o':
      //overallGain = data.substring(indexOfFirstNumber(data)).toFloat();
      //LOG_PORT.print("overallGain: ");
      //LOG_PORT.println(overallGain);
      break;

    case 't':
      //throttlePedal = data.substring(indexOfFirstNumber(data)).toFloat();
      //LOG_PORT.print("throttlePedal: ");
      //LOG_PORT.println(throttlePedal);
      break;

    case 'm':
      missingFootMaxTime = data.substring(indexOfFirstNumber(data)).toInt();
      LOG_PORT.print("missingFootTimeout: ");
      LOG_PORT.println(missingFootMaxTime);
      break;

    case 'f':
      tippedMaxTime = data.substring(indexOfFirstNumber(data)).toInt();
      LOG_PORT.print("tippedMaxTimeout: ");
      LOG_PORT.println(tippedMaxTime);
      break;

    case 'l':
      LOG_PORT.println("LOG AND DIAGNOSTIC DATA");
      LOG_PORT.print("boardAngle: ");
      LOG_PORT.println(boardAngle);
      LOG_PORT.print("frontFSR: ");
      LOG_PORT.println(analogRead(FRONT_FSR_PIN));
      LOG_PORT.print("rearFSR: ");
      LOG_PORT.println(analogRead(BACK_FSR_PIN));
      LOG_PORT.print("battery voltage: ");
      LOG_PORT.println(getBattVoltage());
      LOG_PORT.print(": ");
      LOG_PORT.println();


    default:
      LOG_PORT.print("new input data not processed: ");
      LOG_PORT.println(data);
      LOG_PORT.print("kP: ");
      LOG_PORT.println(kP);
      /*LOG_PORT.print("throttlePedal: ");
        LOG_PORT.println(throttlePedal);
        LOG_PORT.print("boardAngle: ");
        LOG_PORT.println(boardAngle);
        LOG_PORT.print("currentSpeed: ");
        LOG_PORT.println(currentSpeed); */
      LOG_PORT.print("boardState: ");
      LOG_PORT.println((String)boardState);

  }
}

int indexOfFirstNumber(String data) {
  for (int i = 0; i < data.length(); i++) {
    if (isDigit(data.charAt(i))) {
      return i;
    }
  }

  // no numbers found
  return -1;
}

template <typename type>
type sign(type value) {
  return type((value > 0) - (value < 0));
}
