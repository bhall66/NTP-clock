/**************************************************************************
       Title:   NTP Clock
      Author:   Bruce E. Hall, w8bh.net
        Date:   11 Mar 2026
    Hardware:   ESP8266 or HiLetGo ESP32, 2.8" ILI9341 TFT
    Software:   Arduino IDE, ver 2.3.8
                Expressif ESP32 package, ver 2.0.17
                TFT_eSPI Library, ver 2.5.43
                ezTime Library, ver 0.8.3
       Legal:   Copyright (c) 2026  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Dual UTC/Local NTP Clock with TFT display 
                Time is refreshed via NTP every 30 minutes
                Optional time output to serial port 
                Status indicator for time freshness & WiFi strength

                Before using, modify TZ_RULE with your own Posix
                timezone string.

                See w8bh.net for a detailed, step-by-step tutorial.

  Version Hx:   11/27/20  Initial GitHub commit
                11/28/20  added code to handle dropped WiFi connection
                11/30/20  showTimeDate() mod by John Price (WA2FZW)
                12/01/20  showAMPM() added by John Price (WA2FZW)
                02/05/21  added support for ESP8266 modules
                02/07/21  added day-above-month option
                02/10/21  added date-leading-zero option
                03/11/26  added OTA WiFi credentials

 **************************************************************************/

#include <WiFiManager.h>                // https://github.com/tzapu/WiFiManager
#include <TFT_eSPI.h>                   // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>                     // https://github.com/ropg/ezTime

#define TITLE              "NTP TIME"
#define SSID               "NTP clock" 
#define NTP_SERVER         "pool.ntp.org"        // time.nist.gov, pool.ntp.org
#define TZ_RULE            "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"

#if defined(ESP32)                               // The WiFi reset button: 
#define SWITCH_PIN              22               // .. is GPIO 22 on ESP32
#elif defined(ESP8266)
#define SWITCH_PIN              D1               // .. is D1 on ESP8266
#endif

#define SCREEN_ORIENTATION       3               // portrait mode: 1 or 3
#define LED_PIN                  2               // built-in LED is GPIO 2
#define DEBUGLEVEL            INFO               // NONE, ERROR, INFO, or DEBUG
#define PRINTED_TIME             1               // 0=NONE, 1=UTC, or 2=LOCAL
#define TIME_FORMAT         COOKIE               // COOKIE,ISO8601,RFC822,RSS
#define BAUDRATE            115200               // serial output baudrate
#define SYNC_MARGINAL        10800               // orange if no sync for 3 hrs
#define SYNC_LOST            86400               // red if no sync for 1 day
#define LOCAL_FORMAT_12HR     true               // local time 11:34 vs 23:34
#define UTC_FORMAT_12HR      false               // UTC time 11:34 vs 23:34
#define DISPLAY_AMPM          true               // show A for AM, P for PM
#define HOUR_LEADING_ZERO    false               // "01:00" vs " 1:00"
#define DATE_LEADING_ZERO     true               // "Feb 07" vs. "Feb 7"
#define DATE_ABOVE_MONTH     false               // "12 Feb" vs. "Feb 12"
#define TIMECOLOR         TFT_CYAN               // color of time 
#define DATECOLOR       TFT_YELLOW               // color of month & day
#define LABEL_FGCOLOR   TFT_YELLOW               // color of label text
#define LABEL_BGCOLOR     TFT_BLUE               // color of label background


// ============ GLOBAL VARIABLES ==============================================

TFT_eSPI tft = TFT_eSPI();                       // display object 
Timezone local;                                  // local timezone variable
bool useLocalTime  = false;                      // flag for display updates


// ============ WiFi ROUTINES =================================================

bool switchPressed() {
  return !digitalRead(SWITCH_PIN);               // switch active low (gnd)
}

void APCallback (WiFiManager *myWiFiManager) {   // no WiFi network access,
  wifiScreen(); //                               // so ask for WiFi credentials
}

void wifiConnect() {
  WiFiManager wm;
  Serial.println("Connecting to WiFi");
  if (switchPressed())                           // reset switch is pressed?
    wm.resetSettings();                          // yes, erase WiFi credentials
  wm.setAPCallback(APCallback);
  wm.setConfigPortalTimeout(300);                // AP open for 5 minutes
  wm.setDebugOutput(false);                      // dont print WiFi debug info
  if(!wm.autoConnect(SSID)) {                    // create AP for credentials.
    delay(3000);                                 // allow time to close AP
    ESP.restart();                               // let's start again, OK?
    delay(5000);
  } 
}


// ============ DISPLAY ROUTINES ==============================================

void showClockStatus() {
  const int x=290,y=1,w=28,h=29,f=2;             // screen position & size
  int color;
  if (second()%10) return;                       // update every 10 seconds
  int syncAge = now()-lastNtpUpdateTime();       // seconds since last sync?
  if (syncAge < SYNC_MARGINAL)                   // GREEN: time is in sync
    color = TFT_GREEN;
  else if (syncAge < SYNC_LOST)                  // ORANGE: sync 1-24 hours old
    color = TFT_ORANGE;
  else color = TFT_RED;                          // RED: time is > 24 hrs old
  tft.fillRoundRect(x,y,w,h,10,color);           // show status as a color
  tft.setTextColor(TFT_BLACK,color);
  tft.drawNumber(-WiFi.RSSI(),x+8,y+6,f);        // WiFi strength as a + value
}

void showAMPM ( int hr, int x, int y ) {
  char  ampm;                                    // Will be either 'A' or 'P'
  if (hr <= 11) ampm = 'A';                      // 11 or less is morning
  else ampm = 'P';                               // Otherwise, it's afternoon
  tft.drawChar (ampm, x, y, 4);                  // Show AM/PM indicator 
}

void showTime(time_t t, bool hr12, int x, int y) {
  const int f=7;
  tft.setTextColor(TIMECOLOR, TFT_BLACK);
  int h=hour(t); int m=minute(t); 
  int s=second(t); 
  if (hr12) {                                    // if using 12hr time format,
    if (DISPLAY_AMPM) showAMPM(h,x+220,y+14);    // show AM/PM indicator
    if (h==0) h=12;                              // 00:00 becomes 12:00
    if (h>12) h-=12;                             // 13:00 becomes 01:00
  }
  if (h<10) {                                    // is hour a single digit?
    if ((!hr12)||(HOUR_LEADING_ZERO))            // 24hr fmt: always leading 0
      x+= tft.drawChar('0',x,y,f);               // show leading zero for hours
    else {
      tft.setTextColor(TFT_BLACK,TFT_BLACK);     // black on black text     
      x+=tft.drawChar('8',x,y,f);                // will erase the old digit
      tft.setTextColor(TIMECOLOR,TFT_BLACK);
    }
  }
  x+= tft.drawNumber(h,x,y,f);                   // hours
  x+= tft.drawChar(':',x,y,f);                   // show ":"
  if (m<10) x+= tft.drawChar('0',x,y,f);         // leading zero for minutes
  x+= tft.drawNumber(m,x,y,f);                   // show minutes          
  x+= tft.drawChar(':',x,y,f);                   // show ":"
  if (s<10) x+= tft.drawChar('0',x,y,f);         // add leading zero for secs
  x+= tft.drawNumber(s,x,y,f);                   // show seconds
}

void showDate(time_t t, int x, int y) {
  const int f=4,yspacing=30;                     // screen font, spacing
  const char* months[] = {"JAN","FEB","MAR",
     "APR","MAY","JUN","JUL","AUG","SEP","OCT",
     "NOV","DEC"};
  tft.setTextColor(DATECOLOR, TFT_BLACK);
  int i=0, m=month(t), d=day(t);                 // get date components 
  tft.fillRect(x,y,50,60,TFT_BLACK);             // erase previous date
  if (DATE_ABOVE_MONTH) {                        // show date on top -----
    if ((DATE_LEADING_ZERO) && (d<10))           // do we need a leading zero?
      i = tft.drawNumber(0,x,y,f);               // draw leading zero
    tft.drawNumber(d,x+i,y,f);                   // draw date 
    y += yspacing;                               // and below it, the month
    tft.drawString(months[m-1],x,y,f);           // draw month
  } else {                                       // put month on top ----
    tft.drawString(months[m-1],x,y,f);           // draw month
    y += yspacing;                               // and below it, the date
    if ((DATE_LEADING_ZERO) && (d<10))           // do we need a leading zero?
      x+=tft.drawNumber(0,x,y,f);                // draw leading zero
    tft.drawNumber(d,x,y,f);                     // draw date
  }
}

void showTimeZone (int x, int y) {
  const int f=4;                                 // text font
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR); // set text colors
  tft.fillRect(x,y,80,28,LABEL_BGCOLOR);         // erase previous TZ
  if (!useLocalTime) 
    tft.drawString("UTC",x,y,f);                 // UTC time
  else                                           // show local time zone
    tft.drawString(local.getTimezoneName(),
      x,y,f); 
}

void showTimeDate(time_t t, time_t oldT, 
  bool hr12, int x, int y) {   
  showTime(t,hr12,x,y);                          // display time HH:MM:SS 
  if ((!oldT)||(hour(t)!=hour(oldT)))            // did hour change?
    showTimeZone(x,y-42);                        // update time zone
  if ((!oldT)||(day(t)!=day(oldT)))              // date change? (Tnx WA2FZW)
    showDate(t,x+250,y);                         // update date
}

void updateDisplay() {
  static time_t t,oldT,lt,oldLt;
  t=now();  lt=local.now();                           // get latest time
  useLocalTime = true;                                // use local timezone
  showTimeDate(lt,oldLt,LOCAL_FORMAT_12HR,10,46);     // show new local time
  useLocalTime = false;                               // use UTC timezone
  showTimeDate(t,oldT,UTC_FORMAT_12HR,10,172);        // show new UTC time
  showClockStatus();                                  // and clock status
  oldT=t; oldLt=lt;                                   // remember time values
}

void timeScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.fillRoundRect(0,0,319,32,10,LABEL_BGCOLOR);     // local time title bar
  tft.fillRoundRect(0,126,319,32,10,LABEL_BGCOLOR);   // UTC title bar
  tft.setTextColor(LABEL_FGCOLOR,LABEL_BGCOLOR);      // set label colors
  tft.drawCentreString(TITLE,160,4,4);                // show title at top
  tft.drawRoundRect(0,0,319,110,10,TFT_WHITE);        // edge around local
  tft.drawRoundRect(0,126,319,110,10,TFT_WHITE);      // edge around UTC
}

void wifiScreen() {  
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Enter WiFi credentials:",4,10,4);
  tft.drawString("1. Open WiFi on phone",12,50,4);
  tft.drawString("2. Choose 'NTP clock'",12,76,4);
  tft.drawString("3. Open phone browser",12,102,4);
  tft.drawString("4. Press 'Configure WiFi'",12,128,4);
  tft.drawString("5. Enter SSID & password",12,154,4);
  tft.drawString("6. Press 'Save'",12,180,4);
}

void showConnectionProgress(){  
  int elapsed = 0;    
  tft.fillScreen(TFT_BLACK);       
  tft.drawString("WiFi starting",5,50,4);
  while (WiFi.status()!=WL_CONNECTED) {          // while waiting for WiFi
    tft.drawNumber(elapsed++,230,50,4);          // show we are trying!
    delay(1000);
  }
  tft.drawString("IP = " +                       // connected to LAN now
    WiFi.localIP().toString(),5,80,4);           // so show IP address 
  elapsed = 0;
  tft.drawString("Waiting for NTP",5,140,4);     // Now get NTP info
  while (timeStatus()!=timeSet) {                // wait until time retrieved
    events();                                    // allow ezTime to work
    tft.drawNumber(elapsed++,230,140,4);         // show we are trying
    delay(1000);
  }
}

// ============ MISC ROUTINES =================================================

void printTime() {                               // print time to serial port
  if (!PRINTED_TIME) return;                     // 0: dont print
  Serial.print("TIME: ");
  if (PRINTED_TIME==1)
    Serial.println(dateTime(TIME_FORMAT));       // 1: print UTC time
  else 
    Serial.println(local.dateTime(TIME_FORMAT)); // 2: print local time
}

void blink(int count=1) {                        // diagnostic LED blink
  pinMode(LED_PIN,OUTPUT);                       // make sure pin is an output
  for (int i=0; i<count; i++) {                  // blink counter
    digitalWrite(LED_PIN,0);                     // turn LED on 
    delay(200);                                  // for 0.2s
    digitalWrite(LED_PIN,1);                     // and then turn LED off
    delay(200);                                  // for 0.2s
  } 
  pinMode(LED_PIN,INPUT);                        // works for Vcc & Gnd LEDs.
}

void setup() {
  pinMode(SWITCH_PIN,INPUT_PULLUP);              // pushbutton is active low
  blink(3);                                      // show sketch is starting
  tft.init();
  tft.setRotation(SCREEN_ORIENTATION);           // portrait screen orientation
  Serial.begin(BAUDRATE);                        // open serial port
  Serial.println(TITLE);
  setDebug(DEBUGLEVEL);                          // enable NTP debug info
  setServer(NTP_SERVER);                         // set NTP server
  local.setPosix(TZ_RULE);                       // estab. local TZ by rule  
  wifiConnect();                                 // establish WiFi connection    
  showConnectionProgress();
  timeScreen();                                  // format screen: 2 timezones
}

void loop() {
  events();                                      // get periodic NPT updates
  if (secondChanged()) {                         // is it a new second yet? 
    updateDisplay();                             // update clock every second
    printTime();                                 // send timestamp to comport
  }
}