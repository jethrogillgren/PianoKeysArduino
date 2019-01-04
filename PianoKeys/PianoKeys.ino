/*
 * --------------------------------------
 * Arduino with several switches under piano keys, and an XBee for comms.
 * --------------------------------------
 * 
 * PINOUT
 * 
 * TODO
 * 
 * XBEE > Arduino
 * 10 Gnd > Gnd
 * 1 VCC > 3.3V
 * 2 DOUT > 2
 * 3 DIN > 4
 */
 
#include <Wire.h>

#include <XBee.h>
#include <SoftwareSerial.h>

#include <Printers.h>
#include <elapsedMillis.h>


// INPUT
const int ledPin = 13;

const int numButtons = 13;
const int buttonPins[numButtons] = {2,3,4,5,6,7,8,9,10,11,12,A0,A1};

const char messages[numButtons] = {'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g',
                                    'G', 'a', 'A', 'b', '('};
                                    
int lastNumHeld = 0;
int numHeld = 0;
char msg[numButtons*2] = {'\0'};


//XBEE & COMMUNICATIONS
SoftwareSerial xbeeSerial(A2, A3); // RX, TX

//Works with Series1 and 2
XBeeWithCallbacks xbee;

// Build a reuseable message packet to send to the Co-Ordinator
XBeeAddress64 coordinatorAddr = XBeeAddress64(0x00000000, 0x00000000);

int8_t keysMessagePayload[numButtons] = {0};
ZBTxRequest keysMessage = ZBTxRequest(coordinatorAddr, keysMessagePayload,
  numButtons);

#define MSG_RESET   'r'

elapsedMillis timeElapsed; //declare global if you don't want it reset every time loop runs




void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
  
  Wire.begin();

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  // INPUT
  for(int i=0; i < numButtons; i++)
  {
    Serial.println();
    // initialize the pushbutton pin as an input:
    pinMode(buttonPins[i], INPUT);
     // connect internal pull-up so it is not floating
    digitalWrite(buttonPins[i], HIGH);
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

  Serial.println("SETUP");
}


void loop() {

  //READ STATE
  numHeld = 0;
  for(int i=0; i < numButtons; i++)
  {
    if (digitalRead(buttonPins[i]) == LOW)
    {
      //Include this key in the message we'll send
      msg[i] = messages[i];
      numHeld++;
    }
    else
    {
      //Don't include this key in the message
      msg[i] = '\0';
    }
  
  }//End Foreach button

  //SEND UPDATE
  if (timeElapsed > 60000 )
  {
    Serial.println("Sending 60000ms packet");
    
    SendKeysPacket();
    timeElapsed = 0; // reset the counter to 0 so the counting starts over...
  }
  else if(numHeld != lastNumHeld)
  {

    //Tell the server.
    lastNumHeld = numHeld;
    SendKeysPacket();

  }

  digitalWrite(ledPin, numHeld>0);

}


// Parse input, take action if it's a valid character
void parseCommand( char cmd )
{
  Serial.print("Cmd:");
  Serial.println(cmd);
  switch (cmd)
  {
  case MSG_RESET:
    //TODO reset self
    break;

  default: // If an invalid character, do nothing
    Serial.print("Unable to parse command: ");
    Serial.println(cmd);
    break;
  }
}




//// XBEE / COMMUNICATION FUNCTIONS

void SendKeysPacket()
{

  //pressMessagePayload[0] = cmd;
  Serial.println(F("SENDING MESSAGE:"));
    for ( uint8_t i = 0; i < numButtons; i++) {  //
      keysMessagePayload[i] = msg[i];
      Serial.print(keysMessagePayload[i], HEX);
    }
    Serial.println();


  keysMessage.setFrameId(xbee.getNextFrameId());
  
  //xbee.send(pressMessage);
  
  // Send the command and wait up to N ms for a response.  xbee loop continues during this time.
  uint8_t status = xbee.sendAndWait(keysMessage, 3000);
  if (status == 0)
  {
    Serial.println(F("SEND ACKNOWLEDGED"));
    timeElapsed = 0;              // reset the counter to 0 so the counting starts over...

  }
  else
  { //Complain, but do not reset timeElapsed - so that a new packet comes in and
    //tried again immedietly.
    Serial.print(F("SEND FAILED: "));
    printHex(status, 2);
    Serial.println();

    lastNumHeld = 999; //Trigger a resend right away

    //flashSingleLed(LED_BUILTIN, 3, 500);
  }
}


//FrameType:  0x90  recieved.
void zbReceive(ZBRxResponse& rx, uintptr_t data) {

  if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED
      || rx.getOption() == ZB_BROADCAST_PACKET ) {

      //Debug it out - copied from the lib
      Print *p = (Print*)data;
      if (!p) {
        Serial.println("ERROR 2");
        //(LED_BUILTIN, 2, 500);
        return;
      }
      p->println(F("Recieved:"));
        p->print("  Payload: ");
        printHex(*p, rx.getFrameData() + rx.getDataOffset(),
        rx.getDataLength(), F(" "), F("\r\n    "), 8);
      p->println();
        p->print("  From: ");
        printHex(*p, rx.getRemoteAddress64() );
      p->println();

      //This Project only ever takes a 1 char command
      //printHex(rx.getData()[0], 2);
      parseCommand( (char) rx.getData()[0] );
      
      //flashSingleLed(LED_BUILTIN, 5, 50);
      
  } else {
      // we got it (obviously) but sender didn't get an ACK
      Serial.println("ERROR 1");
      //flashSingleLed(LED_BUILTIN, 1, 500);
  }
}


// UTIL FUNCTIONS
void printHex(int num, int precision) {
     char tmp[16];
     char format[128];

     sprintf(format, "0x%%.%dX", precision);

     sprintf(tmp, format, num);
     Serial.print(tmp);
}
