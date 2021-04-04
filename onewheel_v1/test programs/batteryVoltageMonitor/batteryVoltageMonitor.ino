#define BATT_MONITOR_PIN A0
//#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL


const long rL = 197000; // load resistor ohms
const long rS = 9700; // sense resistor ohms
double senseVoltage = 0; // voltage measured
double battVoltage = 0;
int rawSensor = 0;
double voltsPerADC = 0.0047;

bool LEDstate = true;


void setup() {
  Serial.begin(115200);

  Serial.println("ready");

}

void loop() {
  rawSensor = analogRead(BATT_MONITOR_PIN);
  senseVoltage = rawSensor*voltsPerADC;
  battVoltage = senseVoltage*(rL+rS) / rS;

  //Serial.print(voltsPerADC, 10);
  //Serial.print(", ");
  //Serial.print(rawSensor);
  //Serial.print(", ");
  //Serial.print(senseVoltage);
  //Serial.print(", ");
  Serial.println(battVoltage);
  if(!LEDstate) {
    LEDstate = true;
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    LEDstate = false;
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  }
    
  
  delay(500);
  

}
