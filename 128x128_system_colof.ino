#include "Network.h"
#include "Display.h"
#include "System.h"
#include "IRQ.h"
#include "Terminal.h"
#include "Config.h"
#include "Settings.h"
#include <Ucglib.h>

#define UL_PER_SEC 251658240

// 2024/8/27

/* 
TO-DO: Modulize system components
*/

String days[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

WiFiUDP ntpUDP;

Ucglib_SSD1351_18x128x128_SWSPI display(/*sclk=*/ 13, /*data=*/ 14, /*cd=*/ 5 , /*cs=*/ 12, /*reset=*/ 4);

NTPClient timeClient(ntpUDP, timeZone * 3600);

int inst_per_process;
int netStatus = 0;
int _netStatus = 0;
int secsSinceBoot = 0;
int minsSinceBoot = 0;
int hrsSinceBoot = 0;
int daysSinceBoot = 0;
int d, h, m, s = 0;

typedef struct {
  int isActive;
  int interval_tick;
  bool executeOnce;
  bool repeatExecution;
  int (*tick_function)();
} process;


typedef struct {
  process processes[256];
  int procCount = 0;
  int nextPID = 0;
} processStack;

processStack process_stack;

void log(int type, char text[])
{
  switch (type) {
    case 1:
      Serial.printf("\n[INFO] %s", text);
      break;
    case 2:
      Serial.printf("\n[NOTE] %s", text);
      break;
    case 3:
      Serial.printf("\n[WARN] %s", text);
      break;
    case 4:
      Serial.printf("\n[ERR] %s", text);
      break;
    case 5:
      Serial.printf("\n[FATAL] %s", text);
      break;
    case 6:
      Serial.printf("\n[PANIC] %s", text);
      display.setPrintPos(0, 0);
      display.print("Kernel panic"); // Print Kernel panic message to screen
      break;
    default:
      Serial.printf("\n%s", text);
      break;
  }
}

int dummy() {
  return 0;
}

void startProcess(int tick_interval, int (*function)(), bool execute_once, bool repeat_execution) {
  process tmp = process_stack.processes[process_stack.nextPID];

  tmp.interval_tick = tick_interval; // Set tick interval
  tmp.tick_function = function+1;
  tmp.executeOnce = execute_once;
  tmp.repeatExecution = repeat_execution;

  process_stack.procCount++;
  process_stack.nextPID++;

  #if DEBUG_MODE == true
    process debug;
    debug = process_stack.processes[process_stack.nextPID - 1];

    Serial.printf("[INFO] Started process > pid: %d, int_tick: %d execute_once: %d repeat_execution; %d\n", process_stack.nextPID - 1, debug.interval_tick, debug.executeOnce, debug.repeatExecution);
  #endif
}

void endProcess(int pid) {
  process_stack.processes[pid].interval_tick = 0; // Delete the process from process list array
  process_stack.processes[pid].tick_function = dummy; // Turns out replacing function pointer with NULL caused exception, am I just stupid?
  process_stack.processes[pid].executeOnce = 0;
  process_stack.processes[pid].repeatExecution = false;

  if (process_stack.nextPID > pid) process_stack.nextPID = pid; // Check for free process slot and reserve it for next process
}

void initDisplay() {
  display.begin(UCG_FONT_MODE_SOLID);
  display.setRotate270();
  display.clearScreen(); // Init display
}

void tickProcesses()
{
  int i;

  for (i=0; i<process_stack.procCount; i++) {
    if (s % process_stack.processes[i].interval_tick == 0 // Tick process if repeatExecution is true and s % [tick interval] is 0
       && process_stack.processes[i].repeatExecution == true) {
      process_stack.processes[i].tick_function();
      
      if (process_stack.processes[i].executeOnce == true)
        endProcess(i); // End process to prevent it from executing again
    }
    if (process_stack.processes[i].interval_tick == s // Tick process if repeatExecution is false and [tick interval] == s
       && process_stack.processes[i].repeatExecution == false) {
      process_stack.processes[i].tick_function();
      
      if (process_stack.processes[i].executeOnce == true)
        endProcess(i); // End process to prevent it from executing again
    }
  }

  inst_per_process = UL_PER_SEC / process_stack.procCount;
}

void parseCommand() // Basic command parser
{
  String command = Serial.readString();
  String parsedCommand[128];

  int i, base = 0;
  int len = command.length();
  int isString, isString2 = 0;
  int commands = 0;
  char char_i[1];

  for (i=0; i<len; i++)
  {
    char_i[0] = command.charAt(i);

    if (char_i == "'") {
      if (isString == 1) {
        isString = 0;
      } else {
        isString = 1;
      }
    }
    if (char_i == "\"") {
      if (isString2 == 1) {
        isString2 = 0;
      } else {
        isString2 = 1;
      }
    }
    if (char_i == " ") {
      base = i+1;
      parsedCommand[commands] = command.substring(base, i);
    }
  }

  if (parsedCommand[0] == "screen")
  {
    if (parsedCommand[1] == "1" || parsedCommand[1] == "true" || parsedCommand[1] == "enable") {
      display.powerUp();
      Serial.printf("\nEnabled Screen");
    }
      
    else if (parsedCommand[1] == "0" || parsedCommand[1] == "false" || parsedCommand[1] == "disable") {
      display.powerDown();
      Serial.printf("\nDisabled Screen");
    }
  }

  if (parsedCommand[0] == "time")
    Serial.printf("\n%s", timeClient.getFormattedTime());

  if (parsedCommand[0] == "day")
    if (parsedCommand[1] == "raw" || parsedCommand[1] == "int")
      Serial.printf("\n%s", timeClient.getDay());
    else
      Serial.printf("\n%s", days[ timeClient.getDay() ]);
  if (parsedCommand[0] == "echo")
    Serial.printf("\n%s", parsedCommand[1]);
}

void drawImage(const int *image, int ix, int iy)
{
  int x = *image;
  int y = *(image+1);
  int color = *(image+2);
  
  int i, j;
  const int *image_ = image + 3;

  for (i=0; i<y; i++)
  {
    for (j=0; j<x; j++)
    {
      display.setColor(*(image_+(i*x + j)*3), *(image_+(i*x + j)*3+1), *(image_+(i*x + j)*3+2) );
      display.drawPixel(j+ix, i+iy);
    }
  }
}

void kpanic(char reason[])
{
  log(6, reason);
  // ESP.restart();
}

void drawConnectionStatus(int type)
{
  switch (type)
  {
    case 1: // Draw black box to erase connection status message
      display.setColor(0, 0, 0);
      display.drawBox(0, 0, 127, 37);
      break;
    case 2:
      display.setColor(255, 255, 255);
      display.setFont(ucg_font_6x13_tf);
      display.setPrintPos(8, 18);
      display.print("Connecting to WiFi");
      break;
    case 3:
      display.setColor(255, 255, 255);
      display.setFont(ucg_font_6x13_tf);
      display.setPrintPos(8, 18);
      display.print("Failed to connect");
      break;
    case 4:
      display.setColor(255, 255, 255);
      display.setFont(ucg_font_6x13_tf);
      display.setPrintPos(8, 18);
      display.print("Connected to WiFi");
      break;
    default:
      log(4, "Invalid argument has been passed to drawConnectionStatus");
      break;
  }
}

void drawLoadingIcon(int num, int x, int y)
{
  int i = 0;

  if (num == -1)
  {
    for (i=0; i<8; i++)
      drawImage(blankDot, loadingDot_PosX[i] + x, loadingDot_PosY[i] + y);
  } else {
    for (i=0; i<8; i++)
      if (i != loadingDot_skipDraw[num % 8])
        drawImage(loadingDot, loadingDot_PosX[i] + x, loadingDot_PosY[i] + y);
      else
        drawImage(blankDot, loadingDot_PosX[i] + x, loadingDot_PosY[i] + y);
  }
}

void drawNumberDigit(int num, int x, int y, int fontNo)
{
  switch (num)
  {
    case 48: {
      if (fontNo == 0) {
        drawImage(arial12_0, x, y);
      }
      break;
    }
    case 49: {
      if (fontNo == 0) {
        drawImage(arial12_1, x, y);
      }
      break;
    }
    case 50: {
      if (fontNo == 0) {
        drawImage(arial12_2, x, y);
      }
      break;
    }
    case 51: {
      if (fontNo == 0) {
        drawImage(arial12_3, x, y);
      }
      break;
    }
    case 52: {
      if (fontNo == 0) {
        drawImage(arial12_4, x, y);
      }
      break;
    }
    case 53: {
      if (fontNo == 0) {
        drawImage(arial12_5, x, y);
      }
      break;
    }
    case 54: {
      if (fontNo == 0) {
        drawImage(arial12_6, x, y);
      }
      break;
    }
    case 55: {
      if (fontNo == 0) {
        drawImage(arial12_7, x, y);
      }
      break;
    }
    case 56: {
      if (fontNo == 0) {
        drawImage(arial12_8, x, y);
      }
      break;
    }
    case 57: {
      if (fontNo == 0) {
        drawImage(arial12_9, x, y);
      }
      break;
    }
    case 58: {
      if (fontNo == 0) {
        drawImage(arial12_colon, x, y);
      }
      break;
    }
  }
}

void drawNumber(String numstr, int x, int y)
{
  int i, ascii;
  int len = numstr.length();

  for (i=0; i<len; i++) {
    ascii = (int) numstr.charAt(i);
    drawNumberDigit(ascii, i*8+1, y, 0);
  }
}

int rizz()
{
  display.setColor(secsSinceBoot & 0xFF, ~(secsSinceBoot) & 0xFF, ~(secsSinceBoot) & 170);
  display.setPrintPos(20, 20);
  display.print("why am i doing");
  display.setPrintPos(20, 30);
  display.print("this");

  return 0;
}

int NTPService()
{
  timeClient.update();

  return 0;
}

int drawOverlay(bool drawClock, bool drawNetworkStat)
{
  log(1, "Rendering information bar");
  if (drawClock) {
    display.setColor(0, 0, 0);
    display.drawBox(0, 111, 127, 127); // Erase previous time
  }

  display.setColor(255, 255, 255);
  display.drawHLine(0, 109, 127); // Draw horizontal bar
  display.drawHLine(0, 110, 127);

  //display.setPrintPos(1, 126);
  // display.print(timeClient.getFormattedTime().substring(0, 5)); // Print current time

  if (drawClock) {
    drawNumber(timeClient.getFormattedTime().substring(0, 5), 1, 115);
  }

  if (drawNetworkStat) {
    if (WiFi.status() == WL_CONNECTED) {
      drawImage(WiFiIcon, 112, 114); // Wifi Icon
    } else {
      // Commented due to icon being unavailable

      // drawImage(NoWiFiIcon, 112, 114); // No connection icon
    }
  }

  return 0;
}

void setup(void)
{
  int n = 0;

  Serial.begin(115200); // Init serial interface
  Serial.print("\nKernel init");

  if (!ENABLE_WDT) {  
    ESP.wdtDisable();
    *((volatile uint32_t*) 0x60000900) &= ~(1); // Disable Watchdog timer
  } else {
    log(2, "WDT is enabled. It is suggested to disable WDT as it can cause the system to crash.");
  }

  LittleFS.begin(); // Init file system

  initDisplay();

  if (FORCE_PANIC)
    kpanic("Kernel panic triggered by FORCE_PANIC option in Config.h");
 
  drawConnectionStatus(2); // Display WiFi Connection status

  log(1, "Connecting to WiFi");

  network.connect();

  while ( WiFi.status() != WL_CONNECTED ) {
    n++;
    delay(200);

    drawLoadingIcon((n % 8) + 1, 57, 48);

    if (FORCE_FAIL_WIFI) {
      WiFi.disconnect(true);
      n = 300; // Set n to 300 to cause WiFi connection to fail
    }

    if (network.getStatus() == -1 |
        network.getStatus() == -2) {
      log(4, "WiFi connection has failed either due to wrong SSID or password. Check WiFi credentials on Settings.h");
    }

    if (ENABLE_WDT) yield(); // Execute yield() to prevent system from crashing if WDT is enabled

    Serial.print(".");
    if (n >= 75) {
      log(2, "15 seconds has elapsed, but WiFi connection is still not estabilished. Considering that as error, WiFi connection will fail.");
      Serial.printf("\n[ERR] Failed to connect to WiFi network [%s]", ssid);

      drawConnectionStatus(1); // Erase previous status text
      drawConnectionStatus(3); // Print "Failed to connect"

      break;
    }
  }

  drawLoadingIcon(-1, 57, 48);

  if (WiFi.status() != WL_CONNECTED)
  {
    drawConnectionStatus(1); // Erase previous status text
    drawConnectionStatus(3); // Print "Failed to connect"
  } else if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[INFO] Successfully connected to WiFi network [%s]", ssid);
    drawConnectionStatus(1);
    drawConnectionStatus(4);
  }

  timeClient.begin(); // Init NTP Client
  timeClient.update(); // Update time

  drawOverlay(true, true); // Boot completed; Now drawing the overlay

  netStatus = WiFi.status();
  _netStatus = netStatus; // Initially update network status to prevent visual glitch


  startProcess(0, NTPService, false, false); // NTP Information Update Service (Runs every minute)
  //startProcess(1, rizz, false, true);
} 

void loop(void)
{
  delay(10);
  s++;

  tickProcesses();

  if (PRINT_INTERNAL_TIMER) // Print internal timer if 
    Serial.printf("\n%dd %d:%d:%d [%dh] [%dm] [%ds]", d, h, m, s, hrsSinceBoot, minsSinceBoot, secsSinceBoot);

  secsSinceBoot++;

  if (secsSinceBoot == 2)
    drawConnectionStatus(1);

  if (minsSinceBoot % NTPUpdateInterval == 0)
    

  parseCommand();

  if (_netStatus != netStatus)
    drawOverlay(false, true);

  if (s >= 60) {
    s = 0;
    drawOverlay(true, false);

    m++;
    minsSinceBoot++;
  }

  if (m >= 60) {
    m = 0;
    h++;
    hrsSinceBoot++;
  }

  if (h >= 24) {
    h = 0;
    d++;
    daysSinceBoot++;
  }
}