#define FRONT_FSR_PIN A1
#define BACK_FSR_PIN A2


const int FSRThreshhold = 512;


bool isFrontFSRTriggered() {
	return analogRead(FRONT_FSR_PIN) > FSRThreshhold;
}


bool isRearFSRTriggered() {
	return analogRead(BACK_FSR_PIN) > FSRThreshhold;
}
