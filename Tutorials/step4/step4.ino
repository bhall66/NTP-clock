/**************************************************************************
       Title:   NTP Clock, step 4
      Author:   Bruce E. Hall, w8bh.net
        Date:   13 Feb 2021
    Hardware:   ESP8266 or HiLetGo ESP32, 2.8" ILI9341 TFT
    Software:   Arduino IDE 1.8.13 with Expressif ESP32 package 
                TFT_eSPI Library
                ezTime Library
       Legal:   Copyright (c) 2021  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Added ezTime Library
                See previous step for ESP32/display connections.
               
 **************************************************************************/


#include <TFT_eSPI.h>                              // https://github.com/Bodmer/TFT_eSPI
#include <ezTime.h>                                // https://github.com/ropg/ezTime
#if defined(ESP32)
#include <WiFi.h>                                  // use this WiFi lib for ESP32, or
#elif defined (ESP8266)
#include <ESP8266WiFi.h>                           // use this WiFi lib for ESP8266
#endif

#define WIFI_SSID          "yourSSID"               
#define WIFI_PWD           "yourPassword"        
#define TZ_RULE            "EST5EDT,M3.2.0/2:00:00,M11.1.0/2:00:00"
//#define TZ_NAME          "US/Eastern"            // zone name in TZ database 

TFT_eSPI tft = TFT_eSPI();                         // display object 
Timezone local;                                    // local timezone variable

void showNetworkStartup() {                        
  Serial.print("WiFi starting");                   
  while (WiFi.status()!=WL_CONNECTED) {            // while waiting for connection                             
    Serial.print(".");                             // show we are trying!
    delay(300);
  }
  Serial.print("\nWiFi connected. IP = ");         // Connected to LAN now
  Serial.println(WiFi.localIP());                  // so show IP address
}

void setup() {
  tft.init();
  tft.setRotation(1);                              // portrait screen orientation
  tft.fillScreen(TFT_BLACK);                       // start with empty screen
  Serial.begin(115200);                            // open serial port
  WiFi.begin(WIFI_SSID,WIFI_PWD);                  // attempt WiFi connection
  showNetworkStartup();                            // show progress
  waitForSync();                                   // wait for NTP packet return
  Serial.println("UTC:   "+UTC.dateTime());        // display UTC time
  //local.setLocation(TZ_NAME);                    // estab. local TZ by name
  local.setPosix(TZ_RULE);                         // estab. local TZ by rule  
}

void loop() {
  events();                                        // refresh time every 30 min
  if (secondChanged())                             // is it a new second yet? 
    tft.drawString(local.dateTime("h:i:s"),        // display time on screen
      50,50,4);                        
    Serial.println("Local: "+local.dateTime());    // if so, display local time
}
