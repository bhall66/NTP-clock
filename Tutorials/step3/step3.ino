/**************************************************************************
       Title:   NTP Clock, step 3 (there is no step 2)
      Author:   Bruce E. Hall, w8bh.net
        Date:   23 Nov 2020
    Hardware:   Adafruit ESP32 Feather; 2.8" ILI9341 TFT
    Software:   Arduino IDE 1.8.13, ESP32 boards by Expressif; TFT_eSPI Library
       Legal:   Copyright (c) 2020  Bruce E. Hall.
                Open Source under the terms of the MIT License. 
                    
 Description:   Hello, World!
                Blinking onboard LED confirms ESP32 config & code upload.
                Display confirms proper wiring & TFT_eSPI library config.

 Connections:   Display   ESP32 Feather
                -------   -------------
                1 Vcc     "3V"
                2 Gnd     "GND"
                3 CS      "RX"  GPIO 16
                4 RST     "3V"
                5 DC      "A5"  GPIO 4
                6 MOSI    "MO"  GPIO 18
                7 SCK     "SCK" GPIO 5
                8 LED     "3V"
                9 MISO    "MI"  GPIO 19            
               
 **************************************************************************/

#include <TFT_eSPI.h>
#define TITLE           "Hello, World!"
#define LED                13                      // LED is on GPIO Pin 13

TFT_eSPI tft = TFT_eSPI();                         // display object 

void setup() {
  pinMode(LED,OUTPUT);                             // pin for onboard LED
  tft.init();
  tft.setRotation(1);                              // portrait screen orientation
  tft.fillScreen(TFT_BLUE);                        // start with empty screen
  tft.setTextColor(TFT_YELLOW);                    // yellow on blue text
  tft.drawString(TITLE,50,50,4);                   // display the text
}

void loop() {
  digitalWrite(LED,HIGH);                          // turn on LED
  delay(500);                                      // for 0.5 seconds 
  digitalWrite(LED,LOW);                           // turn off LED
  delay(500);                                      // for 0.5 seconds
}
