#include "Network.h"
#include <WiFi.h>

int handler()
{
  if (netStatus == 2) {
    log(3, "Network connection failed due to unknown error. Trying again in 5 seconds.");
  }
  if (netStatus == 0) log(1, "Disconnected from network");
  if (netStatus == 3) {
    log(3, "Network connection has been closed due to unknown error. Trying again in 5 seconds.");
    
  }

  _netStatus = netStatus;
  netStatus = WiFi.status();

  return 0;
}

void connect(int ssid, int password) {
  log(1, "Initializing Wi-Fi connection");
  WiFi.begin(ssid, password);

  return;
}

int getStatus()
{
  if (WiFi.status() == WL_DISCONNECTED) 
    return 0;
  
  if (WiFi.status() == WL_CONNECTED) 
    return 1;

  if (WiFi.status() == WL_CONNECT_FAILED) 
    return 2;

  if (WiFi.status() == WL_CONNECTION_LOST)
    return 3;
  
  if (WiFi.status() == WL_NO_SSID_AVAIL) 
    return -1;
  
  if (WiFi.status() == WL_WRONG_PASSWORD)
    return -2;
  
  return -3;
}