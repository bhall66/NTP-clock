/**************************************************************************
       Title:   NTP Clock, step 8
      Author:   Bruce E. Hall, w8bh.net
        Date:   05 Nov 2021
    Hardware:   HiLetGo ESP32, 2.8" ILI9341 TFT
    Software:   Arduino IDE 1.8.13 with Expressif ESP32 package 
                TFT_eSPI Library
                ezTime Library
       Legal:   Copyright (c) 2021  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Step 8 of building an NTP-based clock with TFT display.
                This step creates a dual clock for UTC and local time.

                see w8bh.net for a detailled, step-by-step tutorial
               
 **************************************************************************/


#include <TFT_eSPI.h>                              // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>                                // https://github.com/ropg/ezTime
#include <WiFi.h>

#define TITLE              "NTP TIME"
#define WIFI_SSID          "yourSSID"               
#define WIFI_PWD           "yourPassword"        
#define TZ_RULE            "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"

#define USE_12HR_FORMAT       true                 // preferred format for local time
#define LEADING_ZERO         false                 // show "01:00" vs " 1:00"
#define SYNC_MARGINAL         3600                 // orange status if no sync for 1 hour
#define SYNC_LOST            86400                 // red status if no sync for 1 day   
#define LOCAL_FORMAT_12HR     true                 // local time format 12hr "11:34" vs 24hr "23:34"
#define UTC_FORMAT_12HR      false                 // UTC time format 12 hr "11:34" vs 24hr "23:34"
#define TIMECOLOR          TFT_CYAN                // color of 7-segment time display
#define DATECOLOR          TFT_YELLOW              // color of displayed month & day
#define LABEL_FGCOLOR      TFT_YELLOW
#define LABEL_BGCOLOR      TFT_BLUE

TFT_eSPI tft = TFT_eSPI();                         // display object 
Timezone local;                                    // local timezone variable
time_t t,oldT;                                     // current & displayed UTC
time_t lt,oldLt;                                   // current & displayed local time  
bool useLocalTime  = false;                        // temp flag used for display updates


void showClockStatus() {
  const int x=290,y=1,w=28,h=29,f=2;               // screen position & size
  int color;
  if (second()%10) return;                         // update every 10 seconds
  int syncAge = now()-lastNtpUpdateTime();         // how long has it been since last sync?
  if (syncAge < SYNC_MARGINAL)                     // time is good & in sync
    color = TFT_GREEN;
  else if (syncAge < SYNC_LOST)                    // sync is 1-24 hours old
    color = TFT_ORANGE;
  else color = TFT_RED;                            // time is stale & should not be trusted
  tft.fillRoundRect(x,y,w,h,10,color);             // show clock status as a color
  tft.setTextColor(TFT_BLACK,color);
  tft.drawNumber(-WiFi.RSSI(),x+8,y+6,f);          // WiFi strength as a positive value
}

void showAMPM (int hr) {
}

void showTime(time_t t, bool hr12, int x, int y) {
  const int f=7;                                   // screen font
  tft.setTextColor(TIMECOLOR, TFT_BLACK);          // set time color
  int h=hour(t); int m=minute(t); int s=second(t); // get hours, minutes, and seconds
  showAMPM(h);                                     // display AM/PM, if needed
  if (hr12) {                                      // adjust hours for 12 vs 24hr format:
    if (h==0) h=12;                                // 00:00 becomes 12:00
    if (h>12) h-=12;                               // 13:00 becomes 01:00
  }
  if (h<10) {                                      // is hour a single digit?
    if ((!hr12)||(LEADING_ZERO))                   // 24hr format: always use leading 0
      x+= tft.drawChar('0',x,y,f);                 // show leading zero for hours
    else {
      tft.setTextColor(TFT_BLACK,TFT_BLACK);       // black on black text     
      x+=tft.drawChar('8',x,y,f);                  // will erase the old digit
      tft.setTextColor(TIMECOLOR,TFT_BLACK);      
    }
  }
  x+= tft.drawNumber(h,x,y,f);                     // hours
  x+= tft.drawChar(':',x,y,f);                     // show ":"
  if (m<10) x+= tft.drawChar('0',x,y,f);           // leading zero for minutes
  x+= tft.drawNumber(m,x,y,f);                     // show minutes          
  x+= tft.drawChar(':',x,y,f);                     // show ":"
  if (s<10) x+= tft.drawChar('0',x,y,f);           // add leading zero for seconds
  x+= tft.drawNumber(s,x,y,f);                     // show seconds
}

void showDate(time_t t, int x, int y) {
  const int f=4,yspacing=30;                       // screen font, spacing
  const char* months[] = {"JAN","FEB","MAR",
     "APR","MAY","JUN","JUL","AUG","SEP","OCT",
     "NOV","DEC"};
  tft.setTextColor(DATECOLOR, TFT_BLACK);
  int m=month(t), d=day(t);                        // get date components  
  tft.fillRect(x,y,50,60,TFT_BLACK);               // erase previous date       
  tft.drawString(months[m-1],x,y,f);               // show month on top
  y += yspacing;                                   // put day below month
  if (d<10) x+=tft.drawNumber(0,x,y,f);            // draw leading zero for day
  tft.drawNumber(d,x,y,f);                         // draw day
}

void showTimeZone (int x, int y) {
  const int f=4;                                   // text font
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set text colors
  tft.fillRect(x,y,80,28,LABEL_BGCOLOR);           // erase previous TZ
  if (!useLocalTime) 
    tft.drawString("UTC",x,y,f);                   // UTC time
  else 
    tft.drawString(local.getTimezoneName(),x,y,f); // show local time zone
}

void showTimeDate(time_t t, time_t oldT, bool hr12, int x, int y) {   
  showTime(t,hr12,x,y);                            // display time HH:MM:SS 
  if ((!oldT)||(hour(t)!=hour(oldT)))              // did hour change?
    showTimeZone(x,y-42);                          // update time zone
  if (day(t)!=day(oldT))                           // did date change? 
    showDate(t,x+250,y);                           // update date
}

void updateDisplay() {
  t = now();                                       // check latest time
  if (t!=oldT) {                                   // are we in a new second yet?
    lt = local.now();                              // keep local time current
    useLocalTime = true;                           // use local timezone
    showTimeDate(lt,oldLt,LOCAL_FORMAT_12HR,10,46);// show new local time
    useLocalTime = false;                          // use UTC timezone
    showTimeDate(t,oldT,UTC_FORMAT_12HR,10,172);   // show new UTC time
    showClockStatus();                             // and clock status
    oldT=t; oldLt=lt;                              // remember currently displayed time
  }
}

void newDualScreen() {
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set label colors
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  tft.fillRoundRect(0,0,319,32,10,LABEL_BGCOLOR);  // title bar for local time
  tft.fillRoundRect(0,126,319,32,10,LABEL_BGCOLOR);// title bar for UTC
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);   // set label colors
  tft.drawCentreString(TITLE,160,4,4);             // show title at top
  tft.drawRoundRect(0,0,319,110,10,TFT_WHITE);     // draw edge around local time
  tft.drawRoundRect(0,126,319,110,10,TFT_WHITE);   // draw edge around UTC
}

void setup() {
  tft.init();
  tft.setRotation(1);                              // portrait screen orientation
  newDualScreen();
  WiFi.begin(WIFI_SSID,WIFI_PWD);                  // attempt WiFi connection
  waitForSync();                                   // wait for NTP packet return
  local.setPosix(TZ_RULE);                         // estab. local TZ by rule  
}

void loop() {
  events();                                        // allow NTP updates
  updateDisplay();                                 // update clock every second
}
