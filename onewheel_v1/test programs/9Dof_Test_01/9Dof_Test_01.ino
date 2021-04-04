#include <SparkFunMPU9250-DMP.h> // Include SparkFun MPU-9250-DMP library

#define Serial SerialUSB

MPU9250_DMP imu;

void setup() {
  delay(5000);
  // start serial connection
  Serial.begin(115200);

  // attempt to start imu
  while(imu.begin() != INV_SUCCESS) {
    Serial.println("trying to start IMU");
    delay(100);
  }

  Serial.println("IMU STARTED");

  // Use setSensors to turn on or off MPU-9250 sensors.
  // Any of the following defines can be combined:
  // INV_XYZ_GYRO, INV_XYZ_ACCEL, INV_XYZ_COMPASS,
  // INV_X_GYRO, INV_Y_GYRO, or INV_Z_GYRO
  imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS); // Enable all sensors

  // Use setGyroFSR() and setAccelFSR() to configure the
  // gyroscope and accelerometer full scale ranges.
  // Gyro options are +/- 250, 500, 1000, or 2000 dps
  imu.setGyroFSR(2000); // Set gyro to 2000 dps
  // Accel options are +/- 2, 4, 8, or 16 g
  imu.setAccelFSR(16); // Set accel to +/-16

  // setLPF() can be used to set the digital low-pass filter
  // of the accelerometer and gyroscope.
  // Can be any of the following: 188, 98, 42, 20, 10, 5
  // (values are in Hz).
  imu.setLPF(5); // Set LPF corner frequency to 5Hz
  
  // The sample rate of the accel/gyro can be set using
  // setSampleRate. Acceptable values range from 4Hz to 1kHz
  imu.setSampleRate(1000); // Set sample rate to 10Hz
  
  // Likewise, the compass (magnetometer) sample rate can be
  // set using the setCompassSampleRate() function.
  // This value can range between: 1-100Hz
  imu.setCompassSampleRate(100); // Set mag rate to 10Hz

  // Call update() to update the imu objects sensor data. You can specify 
  // which sensors to update by OR'ing UPDATE_ACCEL, UPDATE_GYRO, 
  // UPDATE_COMPASS, and/or UPDATE_TEMPERATURE.
  // (The update function defaults to accel, gyro, compass, so you don't 
  // have to specify these values.)
  imu.update();//(UPDATE_ACCEL | UPDATE_GYRO | UPDATE_COMPASS);

  Serial.println("IMU CONFIG COMPLETE");
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("looping");
  delay(2000);
}
