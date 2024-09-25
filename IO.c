#include "IO.h"

serial serial;
spi_interface spi_interface;
i2c_interface i2c_interface;

/* Start of Serial functions */

void writeString(char str[]) {
  Serial.write(*str, strlen(str));
}

void writeChar(char character) {
  Serial.write(character);
}

char readChar() {
  char r = Serial.read();
  return r;
}

char readString() { // Read until Newline character
  char *r = Serial.readStringUntil('\n');
  return r;
}

void initialize(int baud) {
  Serial.begin(baud);
}

/* End of Serial functions */