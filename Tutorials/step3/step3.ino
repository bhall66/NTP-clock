/**************************************************************************
       Title:   NTP Clock, step 3 (there is no step 2)
      Author:   Bruce E. Hall, w8bh.net
        Date:   05 Feb 2021
    Hardware:   HiLetGo ESP32; 2.8" ILI9341 TFT
    Software:   Arduino IDE 1.8.13, ESP32 boards by Expressif; TFT_eSPI Library
       Legal:   Copyright (c) 2021  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Hello, World!
                Blinking onboard LED confirms ESP32 config & code upload.
                Display confirms proper wiring & TFT_eSPI library config.

 Connections:   Display   ESP32 DevKitC
                -------   -------------
                1 Vcc     "5V"
                2 Gnd     "GND"
                3 CS      GPIO 5
                4 RST     "3v3" or (10K resistor to 5V)
                5 DC      GPIO 21
                6 MOSI    GPIO 23
                7 SCK     GPIO 5
                8 LED     "5V"
                9 MISO    n/c            
               
 **************************************************************************/

#include <TFT_eSPI.h>
#define TITLE              "Hello, World!"
#define LED                2                       // LED is on GPIO Pin 2
#define SCREEN_ROTATION    3                       // landscape: use '1' or '3'

TFT_eSPI tft = TFT_eSPI();                         // display object 

void setup() {
  pinMode(LED,OUTPUT);                             // pin for onboard LED
  tft.init();
  tft.setRotation(SCREEN_ROTATION);                // landscape screen orientation
  tft.fillScreen(TFT_BLUE);                        // start with empty screen
  tft.setTextColor(TFT_YELLOW);                    // yellow on blue text
  tft.drawString(TITLE,50,50,4);                   // display the text
}

void loop() {
  digitalWrite(LED,HIGH);                          // turn on LED
  delay(200);                                      // for 0.2 seconds 
  digitalWrite(LED,LOW);                           // turn off LED
  delay(200);                                      // for 0.2 seconds
}
