// Joey Diamond
// useful stuff commonly used in arduino


/////////////////////////////////////////////////////
// Signal LED Utils
/////////////////////////////////////////////////////

/**
 * to run in setup to configure the built-in LED
 */
void setUpLED() {
	pinMode(LED_BUILTIN,OUTPUT);
}

/**
 * turns on the built-in LED
 */
void turnOnLED() {
	digitalWrite(LED_BUILTIN,HIGH);
}

/**
 * turns off the built-in LED
 */
void turnOffLED() {
	digitalWrite(LED_BUILTIN,LOW);
}

/**
 * turns the built-in LED on for 1s and then off for 1s
 */
void standardBlinkLED() {
	turnOnLED();
	delay(1000);
	turnOffLED();
	delay(1000);
}

/////////////////////////////////////////////////////
// Serial Utils
/////////////////////////////////////////////////////

/**
 * Starts the serial connection and waits until it is live
 */
void startSerial() {
	Serial.begin(9600);
	while(!Serial) {
		delay(5);
	}
}

/**
 * Designed to run in serialEvent to retrieve the entered text
 * @Param miliDelay this is how long after the event is triggered 
 * before we start reading. Make this longer for larger text inputs
 */
String onSerialEvent(int miliDelay) {
  String myString = "";
  myString.reserve(200); // reserve 200 bytes for string

  delay(miliDelay); // used to ensure the full serial line has appeared and is available

  while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read();
      // add it to the myString:
      myString += inChar;
  }
  return myString;
}


/////////////////////////////////////////////////////
// Signal LED Utils
/////////////////////////////////////////////////////

/**
 * debug tool to ensure that only chars you want are in the string
 */
void stringSplitPrinter(String split) {
	Serial.println("|||||");
	for(int i=0; i< split.length(); i++) {
		Serial.println(split.charAt(i));
	}
	Serial.println("|||||");
}


/////////////////////////////////////////////////////
// Array Tools
/////////////////////////////////////////////////////

/*
void printArrayLong(long _arr[]) {
	int _arrLength = sizeof(_arr) / sizeof(long);

	Serial.print("printing array of length: ");
	Serial.println(_arrLength);

	for(int i=0; i<_arrLength; i++) {
		Serial.print(i);
		Serial.print(": ");
		Serial.println(_arr[i]);
	}

	Serial.println("done printing array");
} */