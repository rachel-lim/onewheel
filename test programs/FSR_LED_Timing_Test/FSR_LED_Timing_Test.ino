#define Serial SERIAL_PORT_USBVIRTUAL


#include <FastLED.h>

#define NUM_LEDS 18

#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1

#define DATA_PIN 10
#define DATA_PIN_2 11

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

// timing variables
long startTime = 0;
long endTime = 0;
int timingCycleCounter = 0;
int TIMED_CYCLES = 100;
long avgTime = 0;
long totalTime = 0;
float frequency = 0; // Hz

void setup() { 
       FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
       FastLED.addLeds<WS2812, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
       Serial.begin(115200);
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
    timingCycleCounter = 0;
  } else { // middle of testing
    timingCycleCounter++;
  }
  if(isFSRTriggered(FRONT_FSR_PIN) && isFSRTriggered(REAR_FSR_PIN)) {
    for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
      leds[i] = CRGB::Green;
      leds2[i] = CRGB::Green;
    }
  } else if(isFSRTriggered(FRONT_FSR_PIN) || isFSRTriggered(REAR_FSR_PIN)) {
    for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
      leds[i] = CRGB::Blue;
      leds2[i] = CRGB::Blue;
    }
  } else {
    for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
      leds[i] = CRGB::Red;
      leds2[i] = CRGB::Red;
    }
  }
  /*
  Serial.print(analogRead(FRONT_FSR_PIN));
  Serial.print(", ");
  Serial.println(analogRead(REAR_FSR_PIN));
  */
  
  FastLED.show();
  //Serial.println("LEDs off");
  //delay(100);
}

boolean isFSRTriggered(int pin) {
  return analogRead(pin) > 900;
}
