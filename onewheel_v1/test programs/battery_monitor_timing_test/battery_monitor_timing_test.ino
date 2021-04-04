#define BATT_MONITOR_PIN A0
//#define LOG_PORT SERIAL_PORT_USBVIRTUAL
#define Serial SERIAL_PORT_USBVIRTUAL


const long rL = 212000; // load resistor ohms
const long rS = 10900; // sense resistor ohms
double senseVoltage = 0; // voltage measured
double battVoltage = 0;
int rawSensor = 0;
double voltsPerADC = 0.0047;
double correctionFactor = 42.2/55.2; // measured vs calc'ed values
const double batteryScaler = ((rL+rS) / rS * correctionFactor)*voltsPerADC;

bool LEDstate = true;

// timing variables
long startTime = 0;
long endTime = 0;
int timingCycleCounter = 0;
int TIMED_CYCLES = 100;
long avgTime = 0;
long totalTime = 0;
float frequency = 0; // Hz

void setup() {
  Serial.begin(115200);

  Serial.println("ready");

}

void loop() {
  if(timingCycleCounter == 0) { // first run
    startTime = micros();
    timingCycleCounter++;
  } else if(timingCycleCounter >= TIMED_CYCLES) { // finished test, report output and reset
    endTime = micros();
    totalTime = endTime - startTime;
    frequency = (float)TIMED_CYCLES / totalTime * 1000000;
    avgTime = totalTime / TIMED_CYCLES;
    Serial.print(TIMED_CYCLES);
    Serial.print(" cycles averaged ");
    Serial.print(avgTime);
    Serial.print(" microseconds per loop resulting in ");
    Serial.print(frequency);
    Serial.println(" Hz");
    Serial.println(battVoltage);
    timingCycleCounter = 0;
  } else { // middle of testing
    timingCycleCounter++;
  }
  
  battVoltage = analogRead(BATT_MONITOR_PIN)*batteryScaler;
  //senseVoltage = rawSensor*voltsPerADC;
  //battVoltage = rawSensor*batteryScaler;

  //Serial.print(voltsPerADC, 10);
  //Serial.print(", ");
  //Serial.print(rawSensor);
  //Serial.print(", ");
  //Serial.print(senseVoltage);
  //Serial.print(", ");
  //Serial.println(battVoltage);
  /*if(!LEDstate) {
    LEDstate = true;
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else {
    LEDstate = false;
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
  }*/
    
  
  //delay(500);
  

}
