#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define NUM_LEDS 18

#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1

#define LED_PIN 10
#define LED_PIN_2 11

// Define the array of leds
Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leds2(NUM_LEDS, LED_PIN_2, NEO_GRB + NEO_KHZ800);

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

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  leds.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  leds.show();            // Turn OFF all pixels ASAP
  leds.setBrightness(255); // Set BRIGHTNESS (max = 255)     

  leds2.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  leds2.show();            // Turn OFF all pixels ASAP
  leds2.setBrightness(255); // Set BRIGHTNESS (max = 255)     
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

    Serial.print(analogRead(FRONT_FSR_PIN));
    Serial.print(", ");
    Serial.println(analogRead(REAR_FSR_PIN));
  
  } else { // middle of testing
    timingCycleCounter++;
  }
  if(isFSRTriggered(FRONT_FSR_PIN) && isFSRTriggered(REAR_FSR_PIN)) {
    setFrontAll(0, 255, 0);
    setRearAll(0, 255, 0);
  } else if(isFSRTriggered(FRONT_FSR_PIN) || isFSRTriggered(REAR_FSR_PIN)) {
    setFrontAll(0, 0, 255);
    setRearAll(0, 0, 255);
  } else {
    setFrontAll(255, 0, 0);
    setRearAll(255, 0, 0);
  }
  /*
  Serial.print(analogRead(FRONT_FSR_PIN));
  Serial.print(", ");
  Serial.println(analogRead(REAR_FSR_PIN));
  */
  
  leds.show();
  leds2.show();
  //Serial.println("LEDs off");
  //delay(100);
}

boolean isFSRTriggered(int pin) {
  return analogRead(pin) > 900;
}

void setFrontPixel(int Pixel, byte red, byte green, byte blue) {
   // FastLED
   Pixel = Pixel > NUM_LEDS ? NUM_LEDS : Pixel;
   Pixel = Pixel < 0 ? 0 : Pixel;
   
   leds.setPixelColor(Pixel, leds.Color(red, green, blue));
}

void setFrontAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setFrontPixel(i, red, green, blue);
  }
}

void setRearPixel(int Pixel, byte red, byte green, byte blue) {
   // FastLED
   Pixel = Pixel > NUM_LEDS ? NUM_LEDS : Pixel;
   Pixel = Pixel < 0 ? 0 : Pixel;
   
   leds2.setPixelColor(Pixel, leds2.Color(red, green, blue));
}

void setRearAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setRearPixel(i, red, green, blue);
  }
}
