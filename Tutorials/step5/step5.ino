/**************************************************************************
       Title:   NTP Clock, step 5
      Author:   Bruce E. Hall, w8bh.net
        Date:   26 Nov 2020
    Hardware:   Adafruit ESP32 Feather, 2.8" ILI9341 TFT
    Software:   Arduino IDE 1.8.13 with Expressif ESP32 package 
                TFT_eSPI Library
                ezTime Library
       Legal:   Copyright (c) 2020  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Step 5 of building an NTP-based clock with TFT display.
                This step improves display formatting.

                see w8bh.net for a detailled, step-by-step tutorial
               
 **************************************************************************/


#include <TFT_eSPI.h>                              // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>                                // https://github.com/ropg/ezTime
#include <WiFi.h>

#define WIFI_SSID          "yourSSID"               
#define WIFI_PWD           "yourPassword"        
#define TZ_RULE            "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"

#define TITLE              "NTP TIME (UTC)"
#define TIMECOLOR          TFT_CYAN                // color of 7-segment time display
#define DATECOLOR          TFT_YELLOW              // color of displayed month & day
#define LABEL_FGCOLOR      TFT_YELLOW
#define LABEL_BGCOLOR      TFT_BLUE

TFT_eSPI tft = TFT_eSPI();                         // display object 
Timezone local;                                    // local timezone variable
time_t t;                                      

void displayTime() {
  int x=10, y=50, f=7;                             // screen position & font
  tft.setTextColor(TIMECOLOR, TFT_BLACK);          // set time color
  int h=hour(t); int m=minute(t); int s=second(t); // get hours, minutes, and seconds 
  if (h<10) x+= tft.drawChar('0',x,y,f);           // leading zero for hours
  x+= tft.drawNumber(h,x,y,f);                     // hours
  x+= tft.drawChar(':',x,y,f);                     // hour:min separator
  if (m<10) x+= tft.drawChar('0',x,y,f);           // leading zero for minutes
  x+= tft.drawNumber(m,x,y,f);                     // show minutes          
  x+= tft.drawChar(':',x,y,f);                     // show ":"
  if (s<10) x+= tft.drawChar('0',x,y,f);           // add leading zero if needed
  x+= tft.drawNumber(s,x,y,f);                     // show seconds
}

void displayDate() {
  int x=50,y=130,f=4;                              // screen position & font
  const char* days[] = {"Sunday","Monday","Tuesday",
    "Wednesday","Thursday","Friday","Saturday"};
  tft.setTextColor(DATECOLOR, TFT_BLACK);
  tft.fillRect(x,y,265,26,TFT_BLACK);              // erase previous date  
  x+=tft.drawString(days[weekday()-1],x,y,f);      // show day of week
  x+=tft.drawString(", ",x,y,f);                   // and     
  x+=tft.drawNumber(month(),x,y,f);                // show date as month/day/year
  x+=tft.drawChar('/',x,y,f);
  x+=tft.drawNumber(day(),x,y,f);
  x+=tft.drawChar('/',x,y,f);
  x+=tft.drawNumber(year(),x,y,f);
}

void updateDisplay() {
  if (t!=now()) {                                  // is it a new second yet?
    displayTime();                                 // and display it
    if (day(t)!=day())                             // did date change? 
      displayDate();                               // yes, so display it   
    t=now();                                       // Remember current time  
  }
}

void newScreen() {
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  tft.fillRoundRect(2,6,316,32,10,LABEL_BGCOLOR);  // put title bar at top
  tft.drawRoundRect(2,6,316,234,10,TFT_WHITE);     // draw edge around screen
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set label colors
  tft.drawCentreString(TITLE,160,12,4);            // show title at top
}

void setup() {
  tft.init();
  tft.setRotation(1);                              // portrait screen orientation
  newScreen();
  WiFi.begin(WIFI_SSID,WIFI_PWD);                  // attempt WiFi connection
  waitForSync();                                   // wait for NTP packet return
  local.setPosix(TZ_RULE);                         // estab. local TZ by rule  
}

void loop() {
  events();                                        // allow NTP updates
  updateDisplay();                                 // update clock every second
}
