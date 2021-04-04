#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library

#define Serial SerialUSB

#define INTERRUPT_PIN 4 // MPU-9250 INT pin tied to D4
#define IMU_MISSED_DATA_COUNT_MAX = 200;

MPU9250_DMP imu;

float boardRoll = 0;
float boardPitch = 0;
float boardYaw = 0;

int imuMissedDataCount = 0;


void setup() {
  pinMode(INTERRUPT_PIN, INPUT_PULLUP); // Set interrupt as an input w/ pull-up resistor

  // start serial connection
  Serial.begin(115200);

  // attempt to start imu
  Serial.print("IMU STARTING");
  while(imu.begin() != INV_SUCCESS) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  Serial.println("IMU STARTED");
  // Use enableInterrupt() to configure the MPU-9250's 
  // interrupt output as a "data ready" indicator.
  imu.enableInterrupt();

  // The interrupt level can either be active-high or low. Configure as active-low.
  // Options are INT_ACTIVE_LOW or INT_ACTIVE_HIGH
  imu.setIntLevel(INT_ACTIVE_LOW);

  // The interrupt can be set to latch until data is read, or as a 50us pulse.
  // Options are INT_LATCHED or INT_50US_PULSE
  imu.setIntLatched(INT_LATCHED);

  imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT, 200); // Enable 6-axis quat & Set DMP FIFO rate to 200 Hz
}

void loop() {
  if(digitalRead(INTERRUPT_PIN) == LOW) { // if new value from the imu
    
    // reset counter
    imuMissedDataCount = 0;

    // get updated values
    //imu.update();
    imu.dmpUpdateFifo();

    /*Serial.print(millis());
    Serial.println(", NEW IMU DATA"); */
    
    // update angles
    imu.computeEulerAngles();

    // center all angles on 0
    boardRoll = getCenteredAngle(imu.roll);
    boardPitch = getCenteredAngle(imu.pitch);
    boardYaw = getCenteredAngle(imu.yaw);

    Serial.println("R/P/Y: " + String(boardRoll) + ", "
            + String(boardPitch) + ", " + String(boardYaw));
    /*Serial.println();
    Serial.println();
    Serial.println(); */
    
  } else { // we dont have new data :/
    imuMissedDataCount++;
    delay(1);
    if(imuMissedDataCount > 20) { // havent heard from the imu for a while, we gots a problem
      Serial.print("ERROR: NO IMU UPDATES - COUNT: ");
      Serial.println(imuMissedDataCount);
    }
  }

}

// returns a 0 -> 360 angle centered around 0 from -180 -> 180
float getCenteredAngle(float input) {
  if(input > 180) {
    input = input - 360;
  }
  return input;
}
