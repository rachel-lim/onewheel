#include <VescUart.h>

VescUart UARTv;

void setup() {
  Serial.begin(115200);

  while (!Serial) {;}

  UARTv.setSerialPort(&Serial);
}
