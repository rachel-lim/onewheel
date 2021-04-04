//#define Serial SERIAL_PORT_USBVIRTUAL


//#include <FastLED.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    10
#define LED_PIN_2    11

// How many NeoPixels are attached to the Arduino?
#define NUM_LEDS 18

// Declare our NeoPixel strip object:
Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel leds2(NUM_LEDS, LED_PIN_2, NEO_GRB + NEO_KHZ800);


#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1


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
  } else { // middle of testing
    timingCycleCounter++;
  }

  //updateBounceFront(0xff, 0, 0, 1, 50, 20);
  //updateBounceRear(0xff, 0, 0, 1, 50, 20);
  //updateBounceSynced(128, 0, 128, 1, 50, 20);
  
  //fadeFrontInOut(0xff, 0x77, 0x00, 2000);
  //fadeRearInOut(0xff, 0, 0, 1000);
  fadeSyncedInOut(128, 0, 128, 1000);
  
  
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

// global to routines
bool newFrontRoutine = true;
int frontIdx = 0;
bool newRearRoutine = true;
int rearIdx = 0;
bool newSyncRoutine = true;
int syncIdx = 0;


// front bouncing LEDs
bool frontBounceDirection = true;
bool justChangedFrontBounceDirection = true;
long prevFrontTime = 0;

void updateBounceFront(byte red, byte green, byte blue, int eyeSize, int speedDelay, int returnDelay) {
  if(newFrontRoutine) { // just changed routine
    frontIdx = 4;
    frontBounceDirection = true;
    justChangedFrontBounceDirection = true;
    prevFrontTime = millis();
    newFrontRoutine = false;
  }

  if(!justChangedFrontBounceDirection && millis()-prevFrontTime < speedDelay) {
    //Serial.println("skipping due to speed delay");
    return; // skip cycle if too soon
  }
  
  if(justChangedFrontBounceDirection && millis()-prevFrontTime < returnDelay) {
    //Serial.println("skipping due to return delay");
    return; // skip cycle if too soon
  }

  //Serial.println("moving to next step");
  prevFrontTime = millis();
  
  if(frontBounceDirection) { // go forward
    //Serial.println("bounce forward");
    justChangedFrontBounceDirection = false;
    if(frontIdx < NUM_LEDS-eyeSize) {
      //Serial.println("moving along line");
      setFrontAll(0,0,0);
      setFrontPixel(frontIdx, red/10, green/10, blue/10);
      for(int j = 1; j <= eyeSize; j++) {
        setFrontPixel(frontIdx+j, red, green, blue);
      }
      setFrontPixel(frontIdx+eyeSize+1, red/10, green/10, blue/10);
      frontIdx++;
    } else {
      //Serial.println("hit the end of the strip");
      frontIdx = NUM_LEDS;
      frontBounceDirection = false;
      justChangedFrontBounceDirection = true;
    }
    
  } else { // bounce back
    justChangedFrontBounceDirection = false;
    
    if(frontIdx > 4) {
      setFrontAll(0,0,0);
      setFrontPixel(frontIdx, red/10, green/10, blue/10);
      for(int j = 1; j <= eyeSize; j++) {
        setFrontPixel(frontIdx+j, red, green, blue);
      }
      setFrontPixel(frontIdx+eyeSize+1, red/10, green/10, blue/10);
      frontIdx--;
    } else {
      frontIdx = 4;
      frontBounceDirection = true;
      justChangedFrontBounceDirection = true;
    }
  }
}

// rear bouncing LEDs
bool rearBounceDirection = true;
bool justChangedRearBounceDirection = true;
long prevRearTime = 0;

void updateBounceRear(byte red, byte green, byte blue, int eyeSize, int speedDelay, int returnDelay) {
  if(newRearRoutine) { // just changed routine
    rearIdx = 0;
    rearBounceDirection = true;
    justChangedRearBounceDirection = true;
    prevRearTime = millis();
    newRearRoutine = false;
  }

  if(!justChangedRearBounceDirection && millis()-prevRearTime < speedDelay) {
    //Serial.println("skipping due to speed delay");
    return; // skip cycle if too soon
  }
  
  if(justChangedRearBounceDirection && millis()-prevRearTime < returnDelay) {
    //Serial.println("skipping due to return delay");
    return; // skip cycle if too soon
  }

  //Serial.println("moving to next step");
  prevRearTime = millis();
  
  if(rearBounceDirection) { // go forward
    //Serial.println("bounce forward");
    justChangedRearBounceDirection = false;
    if(rearIdx < NUM_LEDS-eyeSize-4) {
      //Serial.println("moving along line");
      setRearAll(0,0,0);
      setRearPixel(rearIdx, red/10, green/10, blue/10);
      for(int j = 1; j <= eyeSize; j++) {
        setRearPixel(rearIdx+j, red, green, blue);
      }
      setRearPixel(rearIdx+eyeSize+1, red/10, green/10, blue/10);
      rearIdx++;
    } else {
      //Serial.println("hit the end of the strip");
      rearIdx = NUM_LEDS-eyeSize-4;
      rearBounceDirection = false;
      justChangedRearBounceDirection = true;
    }
    
  } else { // bounce back
    justChangedRearBounceDirection = false;
    
    if(rearIdx > 0) {
      setRearAll(0,0,0);
      setRearPixel(rearIdx, red/10, green/10, blue/10);
      for(int j = 1; j <= eyeSize; j++) {
        setRearPixel(rearIdx+j, red, green, blue);
      }
      setRearPixel(rearIdx+eyeSize+1, red/10, green/10, blue/10);
      rearIdx--;
    } else {
      rearIdx = 0;
      rearBounceDirection = true;
      justChangedRearBounceDirection = true;
    }
  }
}

// synchronized bouncing LEDs
bool syncBounceDirection = true;
bool justChangedSyncBounceDirection = true;
long prevSyncTime = 0;

void updateBounceSynced(byte red, byte green, byte blue, int eyeSize, int speedDelay, int returnDelay) {
  if(newSyncRoutine) { // just changed routine
    syncIdx = 0;
    syncBounceDirection = true;
    justChangedSyncBounceDirection = true;
    prevSyncTime = millis();
    newSyncRoutine = false;
  }

  if(!justChangedSyncBounceDirection && millis()-prevSyncTime < speedDelay) {
    //Serial.println("skipping due to speed delay");
    return; // skip cycle if too soon
  }
  
  if(justChangedSyncBounceDirection && millis()-prevSyncTime < returnDelay) {
    //Serial.println("skipping due to return delay");
    return; // skip cycle if too soon
  }

  //Serial.println("moving to next step");
  prevSyncTime = millis();
  
  if(syncBounceDirection) { // go forward
    //Serial.println("bounce forward");
    justChangedSyncBounceDirection = false;
    if(syncIdx < NUM_LEDS-eyeSize) {
      //Serial.println("moving along line");
      setFrontAll(0,0,0);
      setRearAll(0,0,0);
      setFrontPixel(syncIdx, red/10, green/10, blue/10);
      setRearPixel(syncIdx-4, red/10, green/10, blue/10);
      
      for(int j = 1; j <= eyeSize; j++) {
        setFrontPixel(syncIdx+j, red, green, blue);
        setRearPixel(syncIdx+j-4, red, green, blue);
      }
      setFrontPixel(syncIdx+eyeSize+1, red/10, green/10, blue/10);
      setRearPixel(syncIdx+eyeSize+1-4, red/10, green/10, blue/10);
      syncIdx++;
    } else {
      //Serial.println("hit the end of the strip");
      syncIdx = NUM_LEDS-eyeSize;
      syncBounceDirection = false;
      justChangedSyncBounceDirection = true;
    }
    
  } else { // bounce back
    justChangedSyncBounceDirection = false;
    
    if(syncIdx > 4) {
      setFrontAll(0,0,0);
      setRearAll(0,0,0);
      setFrontPixel(syncIdx, red/10, green/10, blue/10);
      setRearPixel(syncIdx-4, red/10, green/10, blue/10);
      for(int j = 1; j <= eyeSize; j++) {
        setFrontPixel(syncIdx+j, red, green, blue);
        setRearPixel(syncIdx+j-4, red, green, blue);
      }
      setFrontPixel(syncIdx+eyeSize+1, red/10, green/10, blue/10);
      setRearPixel(syncIdx+eyeSize+1-4, red/10, green/10, blue/10);
      syncIdx--;
    } else {
      syncIdx = 4;
      syncBounceDirection = true;
      justChangedSyncBounceDirection = true;
    }
  }
}


// fade in/out front
bool fadeFrontDirection = true;

void fadeFrontInOut(byte red, byte green, byte blue, long fadeTimeMillis) {
  if(newFrontRoutine) { // just changed routine
    prevFrontTime = millis();
    fadeFrontDirection = true;
    newFrontRoutine = false;
  }

  if(millis()-prevFrontTime > fadeTimeMillis) { // full fade happened, switch directions
    fadeFrontDirection = !fadeFrontDirection;
    prevFrontTime = millis();
  }
  
  float r, g, b;
  float brightness = 0;
  if(fadeFrontDirection) { // brighten the lights
    brightness = (float)(millis()-prevFrontTime)/fadeTimeMillis;
    r = brightness*red;
    g = brightness*green;
    b = brightness*blue;
    setFrontAll(r,g,b);
  } else { // dim the lights
    brightness = 1.0 - (float)(millis()-prevFrontTime)/fadeTimeMillis;
    r = brightness*red;
    g = brightness*green;
    b = brightness*blue;
    setFrontAll(r,g,b);
  }
}

// fade in/out rear
bool fadeRearDirection = true;

void fadeRearInOut(byte red, byte green, byte blue, long fadeTimeMillis) {
  if(newRearRoutine) { // just changed routine
    prevRearTime = millis();
    fadeRearDirection = true;
    newRearRoutine = false;
  }

  if(millis()-prevRearTime > fadeTimeMillis) { // full fade happened, switch directions
    fadeRearDirection = !fadeRearDirection;
    prevRearTime = millis();
  }
  
  float r, g, b;
  float brightness = 0;
  if(fadeRearDirection) { // brighten the lights
    brightness = (float)(millis()-prevRearTime)/fadeTimeMillis;
    r = brightness*red;
    g = brightness*green;
    b = brightness*blue;
    setRearAll(r,g,b);
  } else { // dim the lights
    brightness = 1.0 - (float)(millis()-prevRearTime)/fadeTimeMillis;
    r = brightness*red;
    g = brightness*green;
    b = brightness*blue;
    setRearAll(r,g,b);
  }
}

// fade in/out synced
bool fadeSyncDirection = true;

/*void fadeSyncedInOut(byte red, byte green, byte blue, long fadeTimeMillis) {
  if(newSyncRoutine) { // just changed routine
    prevSyncTime = millis();
    fadeSyncDirection = true;
    newSyncRoutine = false;
  }

  if(millis()-prevSyncTime > fadeTimeMillis) { // full fade happened, switch directions
    fadeSyncDirection = !fadeSyncDirection;
    prevSyncTime = millis();
  }
  
  float r, g, b;
  float brightness = 0;
  if(fadeSyncDirection) { // brighten the lights
    brightness = (float)(millis()-prevSyncTime)/fadeTimeMillis;
    r = brightness*red;
    g = brightness*green;
    b = brightness*blue;
    setFrontAll(r,g,b);
    setRearAll(r,g,b);
  } else { // dim the lights
    brightness = 1.0 - (float)(millis()-prevSyncTime)/fadeTimeMillis;
    r = brightness*red;
    g = brightness*green;
    b = brightness*blue;
    setFrontAll(r,g,b);
    setRearAll(r,g,b);
  }
} */

void fadeSyncedInOut(byte red, byte green, byte blue, long fadeTimeMillis) {
  if(newSyncRoutine) { // just changed routine
    prevSyncTime = millis();
    fadeSyncDirection = true;
    newSyncRoutine = false;
  }

  if(millis()-prevSyncTime > fadeTimeMillis) { // full fade happened, switch directions
    fadeSyncDirection = !fadeSyncDirection;
    prevSyncTime = millis();
  }
  
  float brightness = 0;

  setFrontAll(red,green,blue);
  setRearAll(red,green,blue);
    
  if(fadeSyncDirection) { // brighten the lights
    brightness = (float)(millis()-prevSyncTime)/fadeTimeMillis*255;
    leds.setBrightness(brightness);
    leds2.setBrightness(brightness);
  } else { // dim the lights
    brightness = (1.0 - (float)(millis()-prevSyncTime)/fadeTimeMillis)*255;
    leds.setBrightness(brightness);
    leds2.setBrightness(brightness);
  }
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
