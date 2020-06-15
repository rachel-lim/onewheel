
#include <FastLED.h>

#define NUM_LEDS 17

#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1

#define DATA_PIN 10
#define DATA_PIN_2 11

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

// global to routines
bool newFrontRoutine = true;
int frontIdx = 0;
bool newRearRoutine = true;
int rearIdx = 0;
bool newSyncRoutine = true;
int syncIdx = 0;

void updateBounceFront(byte red, byte green, byte blue, int eyeSize, int speedDelay, int returnDelay);
void updateBounceRear(byte red, byte green, byte blue, int eyeSize, int speedDelay, int returnDelay);
void updateBounceSynced(byte red, byte green, byte blue, int eyeSize, int speedDelay, int returnDelay);

void fadeSyncedInOut(byte red, byte green, byte blue, long fadeTimeMillis);
void fadeRearInOut(byte red, byte green, byte blue, long fadeTimeMillis);
void fadeFrontInOut(byte red, byte green, byte blue, long fadeTimeMillis);

void setFrontPixel(int Pixel, byte red, byte green, byte blue);
void setRearPixel(int Pixel, byte red, byte green, byte blue);

void setFrontAll(byte red, byte green, byte blue);
void setRearAll(byte red, byte green, byte blue);

void setFrontLEDs(int red, int green, int blue);
void setRearLEDs(int red, int green, int blue);

void setupLEDs() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
  setFrontLEDs(0,0,0);
  setRearLEDs(0,0,0);
}

void updateLEDs() {
  if(prevBoardState != boardState) {
    newFrontRoutine = true;
    newRearRoutine = true;
    LOG_PORT.print("new mode: ");
    LOG_PORT.println(boardState);
  }

  switch(boardState) {
    case waitingForRider:
      if(!isFrontFSRTriggered && !isRearFSRTriggered) { // no footpads detected
        fadeSyncedInOut(128,0,128,1500);
      } else if(isFrontFSRTriggered && !isRearFSRTriggered) { // only front footpad detected
        fadeFrontInOut(128,0,128,500);
        updateBounceRear(128,0,128,1,50,20);
      } else if(isRearFSRTriggered && !isFrontFSRTriggered) { // only rear footpad detected
        fadeRearInOut(128,0,128,500);
        updateBounceFront(128,0,128,1,50,20);
      } else { // both FSRs triggered, but still tipped 
        updateBounceSynced(0,255,0,1,50,20);
      }
      break;
    case riding:
      setFrontAll(0,0,0);
      setRearAll(0,0,0);
      if(boardAngle < -2) {
        setFrontAll(255,255,255);
        setRearAll(255,0,0);
      } else if(boardAngle > 2) {
        setFrontAll(255,0,0);
        setRearAll(255,255,255);
      } else {
        setFrontAll(255,255,255);
        setRearAll(255,255,255);
      }
      break;
    case justStopped:
      fadeSyncedInOut(255,165,0,500);
      break;
    case limpMode:
      
      break;
    case slowStop:
      
      break;
    case detectedError:
      
      break;
    default:
      break;
  }
  
  FastLED.show();
}


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
}

void setFrontPixel(int Pixel, byte red, byte green, byte blue) {
   // FastLED
   Pixel = Pixel > NUM_LEDS ? NUM_LEDS : Pixel;
   Pixel = Pixel < 0 ? 0 : Pixel;
   
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
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
   
   leds2[Pixel].r = red;
   leds2[Pixel].g = green;
   leds2[Pixel].b = blue;
}

void setRearAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setRearPixel(i, red, green, blue);
  }
}

void setFrontLEDs(int R, int G, int B) {
	for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    leds[i] = CRGB(R,G,B);
  }
  FastLED.show();
}

void setRearLEDs(int R, int G, int B) {
	for(int i=0; i<(sizeof(leds)/sizeof(leds2[0])); i++) {
    leds2[i] = CRGB(R,G,B);
  }
  FastLED.show();
} 
