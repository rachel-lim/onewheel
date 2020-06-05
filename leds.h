
#include <FastLED.h>

#define NUM_LEDS 20

#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1

#define DATA_PIN 10
#define DATA_PIN_2 11

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

void setupLEDs() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.addLeds<WS2812, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
  
}

void updateLEDs() {
  /*if(isFSRTriggered(FRONT_FSR_PIN) && isFSRTriggered(REAR_FSR_PIN)) {
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
  }*/

  FastLED.show();
}


void setFrontLEDs(int R, int G, int B) {
	for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    leds[i] = CRGB(R,G,B);
  }
  FastLED.show();
}

void setRearLEDs(int R, int G, int B) {
	for(int i=0; i<(sizeof(leds)/sizeof(leds[0])); i++) {
    leds2[i] = CRGB(R,G,B);
  }
  FastLED.show();
}
