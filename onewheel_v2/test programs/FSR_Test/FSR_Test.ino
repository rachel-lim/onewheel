#define FRONT_FSR_PIN A2
#define REAR_FSR_PIN A1

void setup() { 
       
       Serial.begin(115200);
}

void loop() { 
  if(isFSRTriggered(FRONT_FSR_PIN) && isFSRTriggered(REAR_FSR_PIN)) {
    Serial.println("Both FSRs Triggered");
  } else if(isFSRTriggered(FRONT_FSR_PIN) || isFSRTriggered(REAR_FSR_PIN)) {
    Serial.println("One FSRs Triggered");
  } else {
    Serial.println("No FSRs Triggered");
  }

  Serial.print(analogRead(FRONT_FSR_PIN));
  Serial.print(", ");
  Serial.println(analogRead(REAR_FSR_PIN));
  
  
  //Serial.println("LEDs off");
  delay(100);
}

boolean isFSRTriggered(int pin) {
  return analogRead(pin) > 900;
}
