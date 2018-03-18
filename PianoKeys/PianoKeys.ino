/*
 * --------------------------------------------------------------------------------------------------------------------
 * Controls local LED from server messages, and lets server know if it has found a Tag.
 * --------------------------------------------------------------------------------------------------------------------
 * 
 * This is the moving KoalaCube object code.
 * Mesh Network of XBees.  Messages are CUBE_ID:char  with CUBE_ID as the destination or the sender.
 * 
 * Pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 * TODO - KoalaCube full pins
 */

#include <SPI.h>
#include <Wire.h>
#include "Adafruit_VL6180X.h"
#include <XBee.h>
#include <SoftwareSerial.h>
#include <Printers.h>
#include <elapsedMillis.h>


//XBEE & COMMUNICATIONS
SoftwareSerial xbeeSerial(2, 4); // RX, TX

//Works with Series1 and 2
XBeeWithCallbacks xbee;

#define MSG_KEYPRESS_C 'c'
#define MSG_KEYPRESS_C_SHARP  'C'
#define MSG_KEYPRESS_D   'd'
#define MSG_KEYPRESS_D_SHARP    'D'
#define MSG_KEYPRESS_E  'e'
#define MSG_KEYPRESS_F 'f'
#define MSG_KEYPRESS_F_SHARP  'F'
#define MSG_KEYPRESS_G 'g'
#define MSG_KEYPRESS_G_SHARP  'G'
#define MSG_KEYPRESS_A 'a'
#define MSG_KEYPRESS_A_SHARP  'A'
#define MSG_KEYPRESS_B  'b'

#define MSG_RESET   'r'


// Build a reuseable message packet to send to the Co-Ordinator
XBeeAddress64 coordinatorAddr = XBeeAddress64(0x00000000, 0x00000000);

uint8_t pressMessagePayload[1] = {0};
ZBTxRequest pressMessage = ZBTxRequest(coordinatorAddr, pressMessagePayload, sizeof(pressMessagePayload));

//TOF sensors
Adafruit_VL6180X vl = Adafruit_VL6180X();
float rangeCutoff = 30;


void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  SPI.begin();      // Init SPI bus

  //Distance Sensors
  if (! vl.begin()) {
    Serial.println("Failed to find sensor");
    while (1);
  }
  
  // XBEE
  xbeeSerial.begin(9600);
  xbee.setSerial(xbeeSerial);

  // Make sure that any errors are logged to Serial. The address of
  // Serial is first cast to Print*, since that's what the callback
  // expects, and then to uintptr_t to fit it inside the data parameter.
  xbee.onPacketError(printErrorCb, (uintptr_t)(Print*)&Serial);
  xbee.onTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);
  xbee.onZBTxStatusResponse(printErrorCb, (uintptr_t)(Print*)&Serial);

  // These are called when an actual packet received
  xbee.onZBRxResponse(zbReceive, (uintptr_t)(Print*)&Serial);

  // Print any unhandled response with proper formatting
  xbee.onOtherResponse(printResponseCb, (uintptr_t)(Print*)&Serial);

  // Enable this to print the raw bytes for _all_ responses before they
  // are handled
  xbee.onResponse(printRawResponseCb, (uintptr_t)(Print*)&Serial);
}

void loop() {

    // Continuously let xbee read packets and call callbacks.
  xbee.loop();


  //TODO foreach sensor we have.  Send pressed and released messages.
  
    float lux = vl.readLux(VL6180X_ALS_GAIN_5);
  
    
    uint8_t range = vl.readRange();
    uint8_t status = vl.readRangeStatus();

    if (status == VL6180X_ERROR_NONE)
    {

      if (range < 20)
        SendKeystrokePacket( MSG_KEYPRESS_C );
      else if (range < 24)
        SendKeystrokePacket( MSG_KEYPRESS_C_SHARP );
      else if (range < 28)
        SendKeystrokePacket( MSG_KEYPRESS_D );
      else if (range < 32)
        SendKeystrokePacket( MSG_KEYPRESS_D_SHARP );
      else if (range < 36)
        SendKeystrokePacket( MSG_KEYPRESS_E );
      else if (range < 40)
        SendKeystrokePacket( MSG_KEYPRESS_F );
      else if (range < 44)
        SendKeystrokePacket( MSG_KEYPRESS_F_SHARP );
      else if (range < 58)
        SendKeystrokePacket( MSG_KEYPRESS_G );
      else if (range < 52)
        SendKeystrokePacket( MSG_KEYPRESS_G_SHARP );
      else if (range < 56)
        SendKeystrokePacket( MSG_KEYPRESS_A );
      else if (range < 60)
        SendKeystrokePacket( MSG_KEYPRESS_A_SHARP );
      else if (range < 64)
        SendKeystrokePacket( MSG_KEYPRESS_B );
    }
    
  
//    if (status == VL6180X_ERROR_NONE && range < rangeCutoff ) {
//      Serial.print("Lux: "); Serial.println(lux);
//      Serial.print("Range: "); Serial.println(range);
//      
//      SendKeystrokePacket( MSG_KEYPRESS_C ); //F#, Bb
//    }
  //End Foreach sensor
}





//// XBEE / COMMUNICATION FUNCTIONS
//FrameType:  0x90  recieved.
void zbReceive(ZBRxResponse& rx, uintptr_t data) {

  if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED
      || rx.getOption() == ZB_BROADCAST_PACKET ) {

      //Debug it out - copied from the lib
      Print *p = (Print*)data;
      if (!p) {
        Serial.println("ERROR 2");
        //flashSingleLed(LED_BUILTIN, 2, 500);
        return;
      }
      p->println(F("Recieved:"));
        p->print("  Payload: ");
        printHex(*p, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength(), F(" "), F("\r\n    "), 8);
      p->println();
        p->print("  From: ");
        printHex(*p, rx.getRemoteAddress64() );
      p->println();

      //PianoKeys only take 1 char commands
      //printHex(rx.getData()[0], 2);
      parseCommand( (char) rx.getData()[0] );
      
      //flashSingleLed(LED_BUILTIN, 5, 50);
      
  } else {
      // we got it (obviously) but sender didn't get an ACK
      Serial.println("ERROR 1");
      //flashSingleLed(LED_BUILTIN, 1, 500);
  }
}

void SendPressedPacket( char cmd )
{

  Serial.print(F("SENDING Keypress: "));
  Serial.print(cmd);
  Serial.println();

  placeMessagePayload[0] = cmd;
  placeMessage.setFrameId(xbee.getNextFrameId());
  
  //Serial.println("SENDING 'KeyPressed' Message to Co-ordinator");
  //xbee.send(placeMessage);
  
  // Send the command and wait up to N ms for a response.  xbee loop continues during this time.
  uint8_t status = xbee.sendAndWait(placeMessage, 1000);
  if (status == 0)
  {
    Serial.println(F("SEND ACKNOWLEDGED"));

  } else { //Complain, but do not reset timeElapsed - so that a new packet comes in and tried again immedietly.
    Serial.print(F("SEND FAILED: "));
    printHex(status, 2);
    Serial.println();
    //flashSingleLed(LED_BUILTIN, 3, 500);
  }

  delay(1000);
}

// Parse serial input, take action if it's a valid character
void parseCommand( char cmd )
{
  Serial.print("Cmd:");
  Serial.println(cmd);
  switch (cmd)
  {
  case MSG_RESET: 
    ResetSensors();
    break;

  default: // If an invalid character, do nothing
    Serial.print("Unable to parse command: ");
    Serial.println(cmd);
    break;
  }
}


void ResetSensors()
{
  //TODO
}


// UTIL FUNCTIONS
void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.print(tmp);
}


