/**************************************************************************
       Title:   NTP Clock, step 1  (BLINK)
      Author:   Bruce E. Hall, w8bh.net
        Date:   26 Nov 2020
    Hardware:   Adafruit ESP32 Feather
    Software:   Arduino IDE 1.8.13, ESP32 package by Expressif
       Legal:   Copyright (c) 2020  Bruce E. Hall.
                Open Source under the terms of the MIT License. 

   IDE Setup:   Board > ESP32 Arduin0 > Adafruit ESP32 Feather
                Upload Speed: 921600
                Port:  (Set port used by onboard Silicon Labs UART)
                    
 Description:   Blink sketch; Tests basic microcontroller connections.
                Require no external components.
               
 **************************************************************************/


#define LED                13                     // onboard LED on GPIO13

void setup() {
  pinMode(LED,OUTPUT);                            // LED pin is an output
}

void loop() { 
  digitalWrite(LED,HIGH);                         // turn LED on
  delay(500);                                     // for 0.5 seconds.
  digitalWrite(LED,LOW);                          // then turn LED off           
  delay(500);                                     // for 0.5 seconds.
}
