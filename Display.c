#include "Display.h"
#include "io.h"

#ifdef DISPLAY_SSD1306
#ifdef SPI_INTERFACE
U8glib_SSD1306_128X64_NONAME_F_SW_I2C display(/*sclk=*/ 13, /*data=*/ 14, /*cd=*/ 5 , /*cs=*/ 12, /*reset=*/ 4);
#endif
#endif
#ifdef DISPLAY_SSD1351
Ucglib_SSD1351_18x128x128_SWSPI display(/*sclk=*/ 13, /*data=*/ 14, /*cd=*/ 5 , /*cs=*/ 12, /*reset=*/ 4);
#endif