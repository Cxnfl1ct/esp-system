#ifndef _IO_H
#define _IO_H
#endif

typedef struct {
  void (writeString *)();
  void (writeChar *)();
  char (readChar *)();
  char (readString *)();
  void (init *)();
} serial;

typedef struct {
  uint8_t sclk;
  uint8_t mosi;
  uint8_t miso;
  uint8_t dc;
  uint8_t cs;
  uint8_t rst;
} spi_interface;

typedef struct {
  uint8_t sck;
  uint8_t sda;
  uint8_t rst;
} i2c_interface;

void writeString(char str[]);
void writeChar(char character);
char readChar();
char readString();
void initialize(int baud);
