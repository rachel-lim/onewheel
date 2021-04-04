#include <ArduinoBLE.h>

// Device name
const char* nameOfPeripheral = "Onewheel";


int oldBatteryLevel = 0;  // last battery level reading from analog input
long previousMillis = 0;  // last time the battery level was checked, in ms


BLEService onewheelService("2A66");

// create switch characteristic and allow remote device to read and write
const char * kPUuid = "2901";
const char * kPDescVal = "proportional gain";
const char * useExpKPUuid = "2931";
const char * useExpKPDesc = "use exponential kP";
const char * expKPUuid = "2936";
const char * expKPDesc = "exponential kP";

const char * kIUuid = "2906";
const char * kIDescVal = "integral gain";
const char * kDUuid = "29011";
const char * kDDescVal = "derivative gain";
const char * vescAvgMotorCurUuid = "2916";
const char * vescAvgMotorCurDesc = "avg motor current";
const char * vescAvgInputCurUuid = "2921";
const char * vescAvgInputCurDesc = "avg input current";
const char * vescAmpHrsUuid = "2926";
const char * vescAmpHrsDesc = "vesc amp hrs";


BLEStringCharacteristic kPCharacteristic(kPUuid, BLERead | BLEWrite, 10);
BLEStringCharacteristic useExpKPCharacteristic(useExpKPUuid, BLERead | BLEWrite, 10);
BLEStringCharacteristic expKPCharacteristic(expKPUuid, BLERead | BLEWrite, 10);

BLEStringCharacteristic kICharacteristic(kIUuid, BLERead | BLEWrite, 10);
BLEStringCharacteristic kDCharacteristic(kDUuid, BLERead | BLEWrite, 10);
BLEStringCharacteristic batteryLevelCharacteristic("2A19", BLERead | BLENotify, 10);
BLEStringCharacteristic vescBatteryLevelCharacteristic("180F", BLERead | BLENotify, 10);
BLEStringCharacteristic vescAvgMotorCurCharacteristic(vescAvgMotorCurUuid, BLERead | BLENotify, 10);
BLEStringCharacteristic vescAvgInputCurCharacteristic(vescAvgInputCurUuid, BLERead | BLENotify, 10);
BLEStringCharacteristic vescAmpHrsCharacteristic(vescAmpHrsDesc, BLERead | BLENotify, 10);


BLEDescriptor kPDescriptor(kPUuid, kPDescVal);
BLEDescriptor useExpKPDescriptor(useExpKPUuid, useExpKPDesc);
BLEDescriptor expKPDescriptor(expKPUuid, expKPDesc);

BLEDescriptor kIDescriptor(kIUuid, kIDescVal);
BLEDescriptor kDDescriptor(kDUuid, kDDescVal);
BLEDescriptor vescAvgMotorCurDescriptor(vescAvgMotorCurUuid, vescAvgMotorCurDesc);
BLEDescriptor vescAvgInputCurDescriptor(vescAvgInputCurUuid, vescAvgInputCurDesc);
BLEDescriptor vescAmpHrsDescriptor(vescAmpHrsUuid, vescAmpHrsDesc);


long lastBattPublish = 0;
int battPublishInterval = 1000;
long currentMillis = 0;


void publishRegularBLE();




// setup routine
void setupBLE() {
  if (!BLE.begin()) {
    LOG_PORT.println("starting BLE failed!");
    //while (1);
  }

  
  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(onewheelService);

  //kPDescriptor.setValue(kPDescVal);
  //kIDescriptor.setValue(kIDescVal);
  //kDDescriptor.setValue(kDDescVal);

  kPCharacteristic.addDescriptor(kPDescriptor);
  onewheelService.addCharacteristic(kPCharacteristic);

  useExpKPCharacteristic.addDescriptor(useExpKPDescriptor);
  onewheelService.addCharacteristic(useExpKPCharacteristic);

  expKPCharacteristic.addDescriptor(expKPDescriptor);
  onewheelService.addCharacteristic(expKPCharacteristic);

  
  //kICharacteristic.addDescriptor(kIDescriptor);
  //onewheelService.addCharacteristic(kICharacteristic);
  
  //kDCharacteristic.addDescriptor(kDDescriptor);
  //onewheelService.addCharacteristic(kDCharacteristic);

  vescAvgMotorCurCharacteristic.addDescriptor(vescAvgMotorCurDescriptor);
  onewheelService.addCharacteristic(vescAvgMotorCurCharacteristic);
  
  vescAvgInputCurCharacteristic.addDescriptor(vescAvgInputCurDescriptor);
  onewheelService.addCharacteristic(vescAvgInputCurCharacteristic);
  
  vescAmpHrsCharacteristic.addDescriptor(vescAmpHrsDescriptor);
  onewheelService.addCharacteristic(vescAmpHrsCharacteristic);

  onewheelService.addCharacteristic(vescBatteryLevelCharacteristic);
  onewheelService.addCharacteristic(batteryLevelCharacteristic);

  BLE.addService(onewheelService);

  // start advertising
  BLE.advertise();

  // inialize values for the first time
  kPCharacteristic.writeValue((String) kP);
  useExpKPCharacteristic.writeValue((String) useExponentialKp);
  expKPCharacteristic.writeValue((String) kPExp);
  
  publishRegularBLE();
}


// runs every loop cycle
void updateBLE() {
  BLE.poll();
}


// update variables set externally via BLE
void readBLEVars() {
  if (kPCharacteristic.written()) {
    String temp = kPCharacteristic.value();
    kP = temp.toFloat();
    Serial.print("kP written: ");
    Serial.println(kP);
  }

  if (useExpKPCharacteristic.written()) {
    String temp = useExpKPCharacteristic.value();
    if(temp.toInt() == 1)
      useExponentialKp = true;
    else
      useExponentialKp = false;

    Serial.print("use EXP kP written: ");
    Serial.println(useExponentialKp);
  }

  if (expKPCharacteristic.written()) {
    String temp = expKPCharacteristic.value();
    kPExp = temp.toFloat();
    Serial.print("kPExp written: ");
    Serial.println(kPExp);
  }
  

  if (kICharacteristic.written()) {
    String temp = kICharacteristic.value();
    kI = temp.toFloat();
    Serial.print("kI written: ");
    Serial.println(kI);
  }

  if (kDCharacteristic.written()) {
    String temp = kDCharacteristic.value();
    kD = temp.toFloat();
    Serial.print("kD written: ");
    Serial.println(kD);
  }
}


// publish some data to BLE regularly
void publishRegularBLE() {
  BLEDevice central = BLE.central();
  if(central.connected()) {
    currentMillis = millis();
    if(currentMillis - lastBattPublish >= battPublishInterval) {
      batteryLevelCharacteristic.writeValue((String) getBattVoltage());
      vescBatteryLevelCharacteristic.writeValue((String) vescInpVoltage);
      vescAvgMotorCurCharacteristic.writeValue((String) vescAvgMotorCurrent);
      vescAvgInputCurCharacteristic.writeValue((String) vescAvgInputCurrent);
      vescAmpHrsCharacteristic.writeValue((String) vescAmpHours);
    }
  }
}
