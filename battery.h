#define BATT_MONITOR_PIN A0
//#define Serial SERIAL_PORT_USBVIRTUAL

const long rL = 212000; // load resistor ohms
const long rS = 10900; // sense resistor ohms
double senseVoltage = 0; // voltage measured
double battVoltage = 0;
int rawSensor = 0;
double voltsPerADC = 0.0008;
double battVoltages[] = {0,0,0,0,0,0,0,0,0,0};
int voltageIdx = 0;


void updateBattVoltage(bool print) {
  rawSensor = analogRead(BATT_MONITOR_PIN);
  senseVoltage = rawSensor*voltsPerADC;
  battVoltages[voltageIdx] = senseVoltage*(rL+rS) / rS;
  voltageIdx++;
  if(voltageIdx>=10) {
    voltageIdx = 0;
  }

  if(print) {
    Serial.print(", ");
    Serial.print(battVoltage);
  }

}


double getBattVoltage() {
  for(int i=0; i<10; i++) {
    battVoltage+=battVoltages[i];
  }

  battVoltage = battVoltage/10;
}
