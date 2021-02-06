/**************************************************************************
       Title:   NTP Clock, step 6
      Author:   Bruce E. Hall, w8bh.net
        Date:   05 Feb 2021
    Hardware:   HiLetGo ESP32
    Software:   Arduino IDE 1.8.13 with Expressif ESP32 package 
                TFT_eSPI Library
                ezTime Library
       Legal:   Copyright (c) 2021  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Step 6 of building an NTP-based clock with TFT display.
                This step displays local time & date, in 12 hour format,
                with automatic DST adjustment.

                see w8bh.net for a detailled, step-by-step tutorial
               
 **************************************************************************/


#include <TFT_eSPI.h>                              // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>                                // https://github.com/ropg/ezTime
#include <WiFi.h>

#define TITLE              "NTP TIME"
#define WIFI_SSID          "yourSSID"               
#define WIFI_PWD           "yourPassword"        
#define TZ_RULE            "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"

#define USE_12HR_FORMAT    true                    // preferred format for local time
#define LEADING_ZERO       false                   // show "01:00" vs " 1:00"

#define TIMECOLOR          TFT_CYAN                // color of 7-segment time display
#define DATECOLOR          TFT_YELLOW              // color of displayed month & day
#define LABEL_FGCOLOR      TFT_YELLOW
#define LABEL_BGCOLOR      TFT_BLUE

TFT_eSPI tft = TFT_eSPI();                         // display object 
Timezone local;                                    // local timezone variable
time_t t,lt;    


void showAMPM (int hr) {
  int x=250,y=90,ft=4;                             // screen position & font
  tft.setTextColor(TIMECOLOR,TFT_BLACK);           // use same color as time
  if (!USE_12HR_FORMAT) 
    tft.fillRect(x,y,50,20,TFT_BLACK);             // 24hr display, so no AM/PM 
  else if (hr<12) 
    tft.drawString("AM",x,y,ft);                   // before noon, so draw AM
  else 
    tft.drawString("PM",x,y,ft);                   // after noon, so draw PM
}

void displayTime(time_t t) {
  int x=20, y=65, f=7;                             // screen position & font
  tft.setTextColor(TIMECOLOR, TFT_BLACK);          // set time color
  int h=hour(t); int m=minute(t); int s=second(t); // get hours, minutes, and seconds 
  showAMPM(h);                                     // display AM/PM, if needed
  if (USE_12HR_FORMAT) {                           // adjust hours for 12 vs 24hr format:
    if (h==0) h=12;                                // 00:00 becomes 12:00
    if (h>12) h-=12;                               // 13:00 becomes 01:00
  } 
  if (h<10) {                                      // is hour a single digit?
    if ((!USE_12HR_FORMAT)||(LEADING_ZERO))        // 24hr format: always use leading 0
      x+= tft.drawChar('0',x,y,f);                 // show leading zero for hours
    else {
      tft.setTextColor(TFT_BLACK,TFT_BLACK);       // black on black text     
      x+=tft.drawChar('8',x,y,f);                  // will erase the old digit
      tft.setTextColor(TIMECOLOR,TFT_BLACK);       // restore time color   
    }
  } 
  x+= tft.drawNumber(h,x,y,f);                     // hours
  x+= tft.drawChar(':',x,y,f);                     // hour:min separator
  if (m<10) x+= tft.drawChar('0',x,y,f);           // leading zero for minutes
  x+= tft.drawNumber(m,x,y,f);                     // show minutes          
  x+= tft.drawChar(':',x,y,f);                     // show ":"
  if (s<10) x+= tft.drawChar('0',x,y,f);           // add leading zero if needed
  x+= tft.drawNumber(s,x,y,f);                     // show seconds
}

void displayDate(time_t t) {
  int x=20,y=130,f=4;                              // screen position & font
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
    time_t newLt = local.now();                    // get local time
    displayTime(newLt);                            // and display it
    if (day(newLt)!=day(lt))                       // did date change? 
      displayDate(newLt);                          // yes, so display it   
    t=now();                                       // remember current UTC
    lt = newLt;                                    // remember current local time  
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
