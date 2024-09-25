#ifndef _TERMINAL_H
#define _TERMINAL_H
#endif

typedef struct {
  int xcur;
  int ycur;
} terminal_element;

int xlen;
int ylen;

int buf[xlen*ylen];

typedef struct {

} terminal;