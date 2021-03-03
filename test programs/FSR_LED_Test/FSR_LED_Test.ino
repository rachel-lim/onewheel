#define Serial SERIAL_PORT_USBVIRTUAL


#include <FastLED.h>

#define NUM_LEDS 100

#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1

#define DATA_PIN 10
#define DATA_PIN_2 11

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

void setup() { 
       FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
       FastLED.addLeds<WS2812, DATA_PIN_2, GRB>(leds2, NUM_LEDS);
       Serial.begin(115200);
}

void loop() { 
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

  Serial.print(analogRead(FRONT_FSR_PIN));
  Serial.print(", ");
  Serial.println(analogRead(REAR_FSR_PIN));
  
  
  FastLED.show();
  //Serial.println("LEDs off");
  delay(100);
}

boolean isFSRTriggered(int pin) {
  return analogRead(pin) > 900;
}
