#define FRONT_FSR_PIN A2
#define BACK_FSR_PIN A1


int FSRThreshhold = 700;

// use these variables rather than reading the pin every time!!!
bool isFrontFSRTriggered = false;
bool isRearFSRTriggered = false;
bool _isFrontFSRTriggered();
bool _isRearFSRTriggered();



void updateFSRs(bool shouldPrint) {
  isFrontFSRTriggered = _isFrontFSRTriggered();
  isRearFSRTriggered = _isRearFSRTriggered();

  if(shouldPrint) {
    LOG_PORT.print(", ");
    LOG_PORT.print(isFrontFSRTriggered);
    LOG_PORT.print(", ");
    LOG_PORT.print(isRearFSRTriggered); 
    /*LOG_PORT.print(", ");
    LOG_PORT.print(analogRead(FRONT_FSR_PIN));
    LOG_PORT.print(", ");
    LOG_PORT.print(analogRead(BACK_FSR_PIN)); */
    
  }
  
}


bool _isFrontFSRTriggered() {
	return analogRead(FRONT_FSR_PIN) > FSRThreshhold;
}


bool _isRearFSRTriggered() {
	return analogRead(BACK_FSR_PIN) > FSRThreshhold;
}

boolean isFSRTriggered(int pin) {
  return analogRead(pin) > FSRThreshhold;
}
