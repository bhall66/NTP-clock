/**************************************************************************
       Title:   NTP Clock, step 1  (BLINK)
      Author:   Bruce E. Hall, w8bh.net
        Date:   05 Feb 2021
    Hardware:   HiLetGo ESP-WROOM-32, aka "NodeMCU-32S", aka "DevKit C"
    Software:   Arduino IDE 1.8.13, ESP32 package by Expressif
       Legal:   Copyright (c) 2021  Bruce E. Hall.
                Open Source under the terms of the MIT License. 

   IDE Setup:   Board > ESP32 Arduino > Esp32 Dev Module
                Upload Speed: 921600
                Port:  (Set port used by onboard Silicon Labs UART)

 Helpful Tip:   Add a 1uF electrolytic cap between the RESET pin and ground
                to enable auto-boot.  Otherwise, you will have to hold
                down the boot button every time you upload code.
                    
 Description:   Blink sketch; Tests basic microcontroller connections.
                Require no external components.
               
 **************************************************************************/


#define LED                 2                     // onboard LED on GPIO2

void setup() {
  pinMode(LED,OUTPUT);                            // LED pin is an output
}

void loop() { 
  digitalWrite(LED,HIGH);                         // turn LED on
  delay(500);                                     // for 0.5 seconds.
  digitalWrite(LED,LOW);                          // then turn LED off           
  delay(500);                                     // for 0.5 seconds.
}
