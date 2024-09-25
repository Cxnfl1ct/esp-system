#include "Terminal.h"
#include "IO.h"

terminal_element primary;
terminal terminal;

boolean echoBack = false;

void setEcho(boolean b) {
  echoBack = b;
}

void input() {
  if (echoBack) {
  } 
}