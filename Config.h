/*
A file for library inclusions and configs
*/

// Libraries

#include <SPI.h>
#include "Ucglib.h"
#include "icons.h"
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LittleFS.h>

// Configs

#define ENABLE_WDT false
#define FORCE_FAIL_WIFI false
#define FORCE_PANIC false
#define ANTIALIASED_TEXT false // Experimental option
#define PRINT_INTERNAL_TIMER true
#define DEBUG_MODE false

#define DISPLAY_SSD1351