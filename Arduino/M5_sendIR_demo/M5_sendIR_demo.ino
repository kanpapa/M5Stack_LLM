/*
 *  M5_sendIR_demo
 *  IRsend demo for M5CoreS3 and M5Core2
 *  @kanpapa 2025-02-02
 *  License: MIT
 */  
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

//const uint16_t kIrLed = 9;  // M5CoreS3 GPIO pin 9
const uint16_t kIrLed = 26;  // M5Core2 GPIO pin 26
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

void setup() {
  irsend.begin();

  irsend.sendPanasonic64(0x344A9034A4, 40);  // LIGHT ON

  //irsend.sendPanasonic64(0x344A90F464, 40);  // LIGHT OFF
}

void loop() {
}
