#define FRONT_FSR_PIN A1
#define BACK_FSR_PIN A2


#define FSRThreshhold 900;

// use these variables rather than reading the pin every time!!!
bool isFrontFSRTriggered = false;
bool isRearFSRTriggered = false;
bool _isFrontFSRTriggered();
bool _isRearFSRTriggered();



void updateFSRs() {
  isFrontFSRTriggered = _isFrontFSRTriggered();
  isRearFSRTriggered = _isRearFSRTriggered();
}


bool _isFrontFSRTriggered() {
	return analogRead(FRONT_FSR_PIN) > FSRThreshhold;
}


bool _isRearFSRTriggered() {
	return analogRead(BACK_FSR_PIN) > FSRThreshhold;
}

boolean isFSRTriggered(int pin) {
  return analogRead(pin) > 900;
}
