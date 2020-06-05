// motor code here
#include <VescUart.h>

/** Initiate VescUart class */
VescUart UART;
int VESCfailCounter = 0;
int VESCmaxFailCount = 10;

float rpmScaler = 14.28571429;

void setupMotor() {
  /** Setup UART port (Serial1 on Atmega32u4) */
  Serial1.begin(115200);
  
  /** Define which ports to use as UART */
  UART.setSerialPort(&Serial1);

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
  
  if(rpm > 7000) {
    rpm = 7000;
  } else if(rpm < -7000) {
    rpm = -7000;
  }
  UART.setRPM(rpm);
}

void setMotorCurrent(float current) {
  if(current > 20) {
    current = 20;
  } else if(current < -20) {
    current = -20;
  }
  UART.setCurrent(current);
}

void setMotorDutyCycle(float dutyCycle) {
  dutyCycle = limitValue(dutyCycle, 1.0, -1.0);
  UART.setDuty(dutyCycle);
}
