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
double correctionFactor = 42.2/55.2; // measured vs calc'ed values
const double batteryScaler = ((rL+rS) / rS * correctionFactor)*voltsPerADC; // precalc once for speed

// comment out when ready for logging
#define LOG_PORT SERIAL_PORT_USBVIRTUAL

void updateBattVoltage(bool shouldPrint) {
  battVoltages[voltageIdx] = analogRead(BATT_MONITOR_PIN) * batteryScaler;
  voltageIdx++;
  if(voltageIdx>=10) {
    voltageIdx = 0;
  }

}


double getBattVoltage() {
  for(int i=0; i<10; i++) {
    battVoltage+=battVoltages[i];
  }

  battVoltage = battVoltage/10;
}
