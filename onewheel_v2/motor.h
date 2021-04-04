// motor code here
#include <VescUart.h>

/** Initiate VescUart class */
VescUart UARTVesc;
int VESCfailCounter = 0;
int VESCmaxFailCount = 10;

float rpmScaler = 15; // number of pole pairs

float vescRPM = 0;
float vescInpVoltage = 0;
float vescAmpHours = 0;
float vescTachAbs = 0;
float vescTach = 0;
float vescAvgMotorCurrent = 0;
float vescAvgInputCurrent = 0;
float dutyCycleNow = 0;
float vescAmpHoursCharged = 0;


void setupMotor() {
  /** Setup UART port (Serial1 on Atmega32u4) */
  Serial1.begin(115200);
  
  /** Define which ports to use as UART */
  UARTVesc.setSerialPort(&Serial1);

  //UART.setCurrent(30);
}

void updateMotor() {
  /** Call the function getVescValues() to acquire data from VESC */
  /*if ( UART.getVescValues() ) {

    Serial.println(UART.data.rpm);
    Serial.println(UART.data.inpVoltage);
    Serial.println(UART.data.ampHours);
    Serial.println(UART.data.tachometerAbs); 
    VESCfailCounter = 0;
  }
  else
  {
    LOG_PORT.println("Failed to get data!");
    VESCfailCounter++;
    if(VESCfailCounter >=10) {
      Serial1.begin(115200);
      LOG_PORT.println("RESETING VESC CONNECTION");
      VESCfailCounter = 0;
    }
  }*/
}


void setMotorRPM(float rpm) {
  rpm = rpm*rpmScaler;
  
  if(rpm > 7500) {
    rpm = 7500;
  } else if(rpm < -7500) {
    rpm = -7500;
  }
  UARTVesc.setRPM(rpm);
}

void setMotorCurrent(float current) {
  if(current > 30) {
    current = 30;
  } else if(current < -30) {
    current = -30;
  }
  UARTVesc.setCurrent(current);
}

void setMotorDutyCycle(float dutyCycle) {
  dutyCycle = limitValue(dutyCycle, 1.0, -1.0);
  UARTVesc.setDuty(dutyCycle);
}


// USE ONLY WHEN NOT RIDING!!!! TIME EXPENSIVE FUNCTION!!!
void updateMotorVals(bool shouldPrint) {
  if(UARTVesc.getVescValues()) {
    vescRPM = UARTVesc.data.rpm;
    vescInpVoltage = UARTVesc.data.inpVoltage;
    vescAmpHours = UARTVesc.data.ampHours;
    vescTachAbs = UARTVesc.data.tachometerAbs;
    vescTach = UARTVesc.data.tachometer;
    vescAvgMotorCurrent = UARTVesc.data.avgMotorCurrent;
    vescAvgInputCurrent = UARTVesc.data.avgInputCurrent;
    dutyCycleNow = UARTVesc.data.dutyCycleNow;
    vescAmpHoursCharged = UARTVesc.data.ampHoursCharged;
    //vescAmpHoursCharged = UARTVesc.mc_values.amp_hours_charged;
    //vescTemp = UARTVesc.data.temperature;
    if(shouldPrint) {
      LOG_PORT.println("Successfully read VESC Values");
    }
  } else if(shouldPrint) {
    LOG_PORT.println(", FAILED TO READ VALUES FROM VESC!");
  }
}
