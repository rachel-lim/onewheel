#include <servo.h>
#include <MathUtil.h>
#include <util.h>

Servo esc;

volatile float motorSpeed = 0;
int output = 0;
volatile bool enabled = true;

void setup() {
  Serial.begin(9600);
  
  esc.attach(9,1000,2000);

}

void loop() {
  output = map(motorSpeed, 0, 1023, 0, 180);   // scale it to use it with the servo library (value between 0 and 180)
  ESC.write(potValue);    // Send the signal to the ESC
  
  Serial.print(motorSpeed);
  Serial.print(", ");
  Serial.println(output);

}


void SerialEvent() {
  // capture string and normalize
  String input = onSerialEvent(50);
  input.toLowerCase();

  if(input == "enable") {
    enabled = true;
  } else if(input == "disable" || input == "d") {
    enabled = false;
  } else if(input == "reset") {
    motorSpeed = 0;
  } else {
    motorSpeed = input.toDouble();
  }
 
}
