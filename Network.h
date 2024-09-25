#ifndef _NETWORK_H
#define _NETWORK_H
#endif

int handler();
void connect(int ssid, int password);
int getStatus();

typedef struct {
  int (handler)();
  int (connect)();
  int (getStatus)();
} network;

