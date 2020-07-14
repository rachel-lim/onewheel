/******************************************************************************
SparkFun 9DoF Razor M0 Example Firmware
Jim Lindblom @ SparkFun Electronics
Original creation date: November 22, 2016
https://github.com/sparkfun/9DOF_Razor_IMU/Firmware

This example firmware for the SparkFun 9DoF Razor IMU M0 
demonstrates how to grab accelerometer, gyroscope, magnetometer,
and quaternion values from the MPU-9250's digital motion processor
(DMP). It prints those values to a serial port and, if a card is
present, an SD card.

Values printed can be configured using the serial port. Settings
can be modified using the included "config.h" file.

Resources:
SparkFun MPU9250-DMP Arduino Library:
  https://github.com/sparkfun/SparkFun_MPU-9250-DMP_Arduino_Library
FlashStorage Arduino Library
  https://github.com/cmaglie/FlashStorage

Development environment specifics:
  Firmware developed using Arduino IDE 1.6.12

Hardware:
  SparkFun 9DoF Razor IMU M0 (SEN-14001)
  https://www.sparkfun.com/products/14001
******************************************************************************/
// MPU-9250 Digital Motion Processing (DMP) Library
#include <SparkFunMPU9250-DMP.h>
// SD Library manages file and hardware control
#include <SD.h>
// config.h manages default logging parameters and can be used
// to adjust specific parameters of the IMU
#include "config.h"
// Flash storage (for nv storage on ATSAMD21)
#ifdef ENABLE_NVRAM_STORAGE
#include <FlashStorage.h>
#endif

MPU9250_DMP imu; // Create an instance of the MPU9250_DMP class

// Note: Some of these params can be overwritten using serial
//  commands. These are just defaults on initial programming
#define DMP_SAMPLE_RATE    200 // Logging/DMP sample rate(4-200 Hz)
#define IMU_COMPASS_SAMPLE_RATE 100 // Compass sample rate (4-100 Hz)
#define IMU_AG_SAMPLE_RATE 1000 // Accel/gyro sample rate Must be between 4Hz and 1kHz
#define IMU_GYRO_FSR       2000 // Gyro full-scale range (250, 500, 1000, or 2000)
#define IMU_ACCEL_FSR      2 // Accel full-scale range (2, 4, 8, or 16)
#define IMU_AG_LPF         5 // Accel/Gyro LPF corner frequency (5, 10, 20, 42, 98, or 188 Hz)
#define ENABLE_GYRO_CALIBRATION true

/////////////////////////////
// Logging Control Globals //
/////////////////////////////
// Defaults all set in config.h
bool enableSDLogging = ENABLE_SD_LOGGING;
bool enableSerialLogging = ENABLE_UART_LOGGING;
bool enableTimeLog = ENABLE_TIME_LOG;
bool enableCalculatedValues = ENABLE_CALCULATED_LOG;
bool enableAccel = ENABLE_ACCEL_LOG;
bool enableGyro = ENABLE_GYRO_LOG;
bool enableCompass = ENABLE_MAG_LOG;
bool enableQuat = ENABLE_QUAT_LOG;
bool enableEuler = ENABLE_EULER_LOG;
bool enableHeading = ENABLE_HEADING_LOG;
unsigned short accelFSR = IMU_ACCEL_FSR;
unsigned short gyroFSR = IMU_GYRO_FSR;
unsigned short fifoRate = DMP_SAMPLE_RATE;

/////////////////////
// SD Card Globals //
/////////////////////
bool sdCardPresent = false; // Keeps track of if SD card is plugged in
String logFileName; // Active logging file
String logFileBuffer; // Buffer for logged data. Max is set in config

///////////////////////
// LED Blink Control //
///////////////////////
//bool ledState = false;
uint32_t lastBlink = 0;
void blinkLED()
{
  static bool ledState = false;
  digitalWrite(HW_LED_PIN, ledState);
  ledState = !ledState;
}

#ifdef ENABLE_NVRAM_STORAGE
  ///////////////////////////
  // Flash Storage Globals //
  ///////////////////////////
  // Logging parameters are all stored in non-volatile memory.
  // They should maintain the previously set config value.
  FlashStorage(flashEnableSDLogging, bool);
  FlashStorage(flashFirstRun, bool);
  FlashStorage(flashEnableSD, bool);
  FlashStorage(flashEnableSerialLogging, bool);
  FlashStorage(flashenableTime, bool);
  FlashStorage(flashEnableCalculatedValues, bool);
  FlashStorage(flashEnableAccel, bool);
  FlashStorage(flashEnableGyro, bool);
  FlashStorage(flashEnableCompass, bool);
  FlashStorage(flashEnableQuat, bool);
  FlashStorage(flashEnableEuler, bool);
  FlashStorage(flashEnableHeading, bool);
  FlashStorage(flashAccelFSR, unsigned short);
  FlashStorage(flashGyroFSR, unsigned short);
  FlashStorage(flashLogRate, unsigned short);
#endif


// function prototypes
void setupIMU();
void updateIMU();
void logIMUData();
void initHardware();
bool initIMU();
bool initSD();
bool sdLogString(String toLog);
String nextLogFile();
void parseSerialInput(char c);
#ifdef ENABLE_NVRAM_STORAGE
  void initLoggingParams(void);
#endif
void production_testing(void);
bool uSD_ping(void);
bool net_1_test();
bool net_2_test();
void set_nets_all_inputs();



void setupIMU()
{
  // Initialize LED, interrupt input, and serial port.
  // LED defaults to off:
  initHardware(); 
#ifdef ENABLE_NVRAM_STORAGE
  // Load previously-set logging parameters from nvram:
  initLoggingParams();
#endif

  // Initialize the MPU-9250. Should return true on success:
  if ( !initIMU() ) 
  {
    LOG_PORT.println("Error connecting to MPU-9250");
    while (1) ; // Loop forever if we fail to connect
    // LED will remain off in this state.
  }

  // Check for the presence of an SD card, and initialize it:
  if ( initSD() )
  {
    sdCardPresent = true;
    // Get the next, available log file name
    logFileName = nextLogFile(); 
  }
}

float gyroRate = 0;
float gyroRateRads = 0;

void updateIMU() {
  // Then check IMU for new data, and log it
  if ( !imu.fifoAvailable() ) {// If no new data is available
    //LOG_PORT.println("NO NEW FIFO");
    return;                   // return to the top of the loop
  }

  // Read from the digital motion processor's FIFO
  if ( imu.dmpUpdateFifo() != INV_SUCCESS ) {
    //LOG_PORT.println("NO NEW DMP");
    return; // If that fails (uh, oh), return to top
  }
  
  imu.computeEulerAngles();

  float accel = -imu.calcAccel(imu.ax) * 20/0.35;
  accel = limitValue(accel, 80, -80);
  
  gyroRate = imu.calcGyro(imu.gy);
  gyroRate = limitValue(gyroRate, 92, -92);

  float aa = 0.03; //how much we use the accelerometer in the calcs
  deltaTime = micros() - prevTime;
  float gyroAngleDt = gyroRate *  deltaTime * 0.000001; // how much the angle has changed
  prevTime = micros();
  gyroRateRads = gyroRate * 0.017453;
  boardAngle = float ((1-aa) * (boardAngle+gyroAngleDt)) + aa*accel;
  
}

float getBoardPitch() {
  /*float temp = imu.roll;
  
  if(temp > 180) {
    return temp-360;
  }
  return temp; */
  return boardAngle;
}

float getBoardRoll() {
  float temp = imu.pitch;
  
  if(temp > 180) {
    return temp-360;
  }
  return temp;
}

float getBoardHeading() {
  float temp = imu.yaw;
  
  if(temp > 180) {
    return temp-360;
  }
  return temp;
}

void logIMUData(void)
{
  String imuLog = ""; // Create a fresh line to log
  if (enableTimeLog) // If time logging is enabled
  {
    imuLog += String(imu.time) + ", "; // Add time to log string
  }
  if (enableAccel) // If accelerometer logging is enabled
  {
    if ( enableCalculatedValues ) // If in calculated mode
    {
      imuLog += String(imu.calcAccel(imu.ax)) + ", ";
      imuLog += String(imu.calcAccel(imu.ay)) + ", ";
      imuLog += String(imu.calcAccel(imu.az)) + ", ";
    }
    else
    {
      imuLog += String(imu.ax) + ", ";
      imuLog += String(imu.ay) + ", ";
      imuLog += String(imu.az) + ", ";
    }
  }
  if (enableGyro) // If gyroscope logging is enabled
  {
    if ( enableCalculatedValues ) // If in calculated mode
    {
      imuLog += String(imu.calcGyro(imu.gx)) + ", ";
      imuLog += String(imu.calcGyro(imu.gy)) + ", ";
      imuLog += String(imu.calcGyro(imu.gz)) + ", ";
    }
    else
    {
      imuLog += String(imu.gx) + ", ";
      imuLog += String(imu.gy) + ", ";
      imuLog += String(imu.gz) + ", ";
    }
  }
  if (enableCompass) // If magnetometer logging is enabled
  {
    if ( enableCalculatedValues ) // If in calculated mode
    {
      imuLog += String(imu.calcMag(imu.mx)) + ", ";
      imuLog += String(imu.calcMag(imu.my)) + ", ";
      imuLog += String(imu.calcMag(imu.mz)) + ", ";    
    }
    else
    {
      imuLog += String(imu.mx) + ", ";
      imuLog += String(imu.my) + ", ";
      imuLog += String(imu.mz) + ", ";
    }
  }
  if (enableQuat) // If quaternion logging is enabled
  {
    if ( enableCalculatedValues )
    {
      imuLog += String(imu.calcQuat(imu.qw), 4) + ", ";
      imuLog += String(imu.calcQuat(imu.qx), 4) + ", ";
      imuLog += String(imu.calcQuat(imu.qy), 4) + ", ";
      imuLog += String(imu.calcQuat(imu.qz), 4) + ", ";
    }
    else
    {
      imuLog += String(imu.qw) + ", ";
      imuLog += String(imu.qx) + ", ";
      imuLog += String(imu.qy) + ", ";
      imuLog += String(imu.qz) + ", ";      
    }
  }
  if (enableEuler) // If Euler-angle logging is enabled
  {
    imu.computeEulerAngles();
    imuLog += String(imu.pitch, 2) + ", ";
    imuLog += String(imu.roll, 2) + ", ";
    imuLog += String(imu.yaw, 2) + ", ";
  }
  if (enableHeading) // If heading logging is enabled
  {
    imuLog += String(imu.computeCompassHeading(), 2) + ", ";
  }
  
  // Remove last comma/space:
  imuLog.remove(imuLog.length() - 2, 2);
  imuLog += "\r\n"; // Add a new line

  if (enableSerialLogging)  // If serial port logging is enabled
    LOG_PORT.print(imuLog); // Print log line to serial port

  // If SD card logging is enabled & a card is plugged in
  if ( sdCardPresent && enableSDLogging)
  {
    // If adding this log line will put us over the buffer length:
    if (imuLog.length() + logFileBuffer.length() >=
        SD_LOG_WRITE_BUFFER_SIZE)
    {
      sdLogString(logFileBuffer); // Log SD buffer
      logFileBuffer = ""; // Clear SD log buffer 
      blinkLED(); // Blink LED every time a new buffer is logged to SD
    }
    // Add new line to SD log buffer
    logFileBuffer += imuLog;
  }
  else
  {
    // Blink LED once every second (if only logging to serial port)
    if ( millis() > lastBlink + UART_BLINK_RATE )
    {
      blinkLED(); 
      lastBlink = millis();
    }
  }
}

void initHardware(void)
{
  // Set up LED pin (active-high, default to off)
  pinMode(HW_LED_PIN, OUTPUT);
  digitalWrite(HW_LED_PIN, LOW);

  // Set up MPU-9250 interrupt input (active-low)
  pinMode(MPU9250_INT_PIN, INPUT_PULLUP);
}

bool initIMU(void)
{
  // imu.begin() should return 0 on success. Will initialize
  // I2C bus, and reset MPU-9250 to defaults.
  if (imu.begin() != INV_SUCCESS)
    return false;

  // Set up MPU-9250 interrupt:
  imu.enableInterrupt(); // Enable interrupt output
  imu.setIntLevel(1);    // Set interrupt to active-low
  imu.setIntLatched(1);  // Latch interrupt output

  // Configure sensors:
  // Set gyro full-scale range: options are 250, 500, 1000, or 2000:
  imu.setGyroFSR(gyroFSR);
  // Set accel full-scale range: options are 2, 4, 8, or 16 g 
  imu.setAccelFSR(accelFSR);
  // Set gyro/accel LPF: options are5, 10, 20, 42, 98, 188 Hz
  imu.setLPF(IMU_AG_LPF); 
  // Set gyro/accel sample rate: must be between 4-1000Hz
  // (note: this value will be overridden by the DMP sample rate)
  imu.setSampleRate(IMU_AG_SAMPLE_RATE); 
  // Set compass sample rate: between 4-100Hz
  imu.setCompassSampleRate(IMU_COMPASS_SAMPLE_RATE); 

  // Configure digital motion processor. Use the FIFO to get
  // data from the DMP.
  unsigned short dmpFeatureMask = 0;
  if (ENABLE_GYRO_CALIBRATION)
  {
    // Gyro calibration re-calibrates the gyro after a set amount
    // of no motion detected
    dmpFeatureMask |= DMP_FEATURE_GYRO_CAL;
    dmpFeatureMask |= DMP_FEATURE_SEND_CAL_GYRO;
  }
  else
  {
    // Otherwise add raw gyro readings to the DMP
    dmpFeatureMask |= DMP_FEATURE_SEND_RAW_GYRO;
  }
  // Add accel and quaternion's to the DMP
  dmpFeatureMask |= DMP_FEATURE_SEND_RAW_ACCEL;
  //dmpFeatureMask |= DMP_FEATURE_6X_LP_QUAT;

  // Initialize the DMP, and set the FIFO's update rate:
  imu.dmpBegin(dmpFeatureMask, fifoRate);

  return true; // Return success
}

bool initSD(void)
{
  // SD.begin should return true if a valid SD card is present
  if ( !SD.begin(SD_CHIP_SELECT_PIN) )
  {
    return false;
  }

  return true;
}

// Log a string to the SD card
bool sdLogString(String toLog)
{
  // Open the current file name:
  File logFile = SD.open(logFileName, FILE_WRITE);
  
  // If the file will get too big with this new string, create
  // a new one, and open it.
  if (logFile.size() > (SD_MAX_FILE_SIZE - toLog.length()))
  {
    logFileName = nextLogFile();
    logFile = SD.open(logFileName, FILE_WRITE);
  }

  // If the log file opened properly, add the string to it.
  if (logFile)
  {
    logFile.print(toLog);
    logFile.close();

    return true; // Return success
  }

  return false; // Return fail
}

// Find the next available log file. Or return a null string
// if we've reached the maximum file limit.
String nextLogFile(void)
{
  String filename;
  int logIndex = 0;

  for (int i = 0; i < LOG_FILE_INDEX_MAX; i++)
  {
    // Construct a file with PREFIX[Index].SUFFIX
    filename = String(LOG_FILE_PREFIX);
    filename += String(logIndex);
    filename += ".";
    filename += String(LOG_FILE_SUFFIX);
    // If the file name doesn't exist, return it
    if (!SD.exists(filename))
    {
      return filename;
    }
    // Otherwise increment the index, and try again
    logIndex++;
  }

  return "";
}


#ifdef ENABLE_NVRAM_STORAGE
  // Read from non-volatile memory to get logging parameters
  void initLoggingParams(void)
  {
    // Read from firstRun mem location, should default to 0 on program
    if (flashFirstRun.read() == 0) 
    {
      // If we've got a freshly programmed board, program all of the
      // nvm locations:
      flashEnableSDLogging.write(enableSDLogging);
      flashEnableSerialLogging.write(enableSerialLogging);
      flashenableTime.write(enableTimeLog);
      flashEnableCalculatedValues.write(enableCalculatedValues);
      flashEnableAccel.write(enableAccel);
      flashEnableGyro.write(enableGyro);
      flashEnableCompass.write(enableCompass);
      flashEnableQuat.write(enableQuat);
      flashEnableEuler.write(enableEuler);
      flashEnableHeading.write(enableHeading);
      flashAccelFSR.write(accelFSR);
      flashGyroFSR.write(gyroFSR);
      flashLogRate.write(fifoRate);
      
      flashFirstRun.write(1); // Set the first-run boolean
    }
    else // If values have been previously set:
    {
      // Read from NVM and set the logging parameters:
      enableSDLogging = flashEnableSDLogging.read();
      enableSerialLogging = flashEnableSerialLogging.read();
      enableTimeLog = flashenableTime.read();
      enableCalculatedValues = flashEnableCalculatedValues.read();
      enableAccel = flashEnableAccel.read();
      enableGyro = flashEnableGyro.read();
      enableCompass = flashEnableCompass.read();
      enableQuat = flashEnableQuat.read();
      enableEuler = flashEnableEuler.read();
      enableHeading = flashEnableHeading.read();
      accelFSR = flashAccelFSR.read();
      gyroFSR = flashGyroFSR.read();
      fifoRate = flashLogRate.read();
    }
  }
#endif
