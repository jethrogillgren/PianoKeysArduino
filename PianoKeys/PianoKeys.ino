
///*
// * --------------------------------------------------------------------------------------------------------------------
// * Controls local LED from server messages, and lets server know if it has found a Tag.
// * --------------------------------------------------------------------------------------------------------------------
// * 
// * This is the moving KoalaCube object code.
// * Mesh Network of XBees.  Messages are CUBE_ID:char  with CUBE_ID as the destination or the sender.
// * 
// * Pin layout used:
// * -----------------------------------------------------------------------------------------
// *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
// *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
// * Signal      Pin          Pin           Pin       Pin        Pin              Pin
// * -----------------------------------------------------------------------------------------
// * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
// * SPI SS      SDA(SS)      10            53        D10        10               10
// * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
// * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
// * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
// * 
// * TODO - KoalaCube full pins
// */
//

#include <Wire.h>
#include <VL6180X.h>

#define RANGE 1



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
/* List of adresses for each sensor - after reset the address can be configured */
#define address0 0x20
#define address1 0x22
#define address2 0x24
#define address3 0x26
#define address4 0x28
#define address5 0x30
#define address6 0x32
#define address7 0x34
#define address8 0x36
#define address9 0x38
#define address10 0x40
#define address11 0x42

///* These Arduino pins must be wired to the IO0 pin of VL6180x */
int enablePin0 = 5;
int enablePin1 = 6;
int enablePin2 = 7;
int enablePin3 = 8;
int enablePin4 = 9;
int enablePin5 = 10;
int enablePin6 = 11;
int enablePin7 = 12;
int enablePin8 = 13;
int enablePin9 = 14; //A0
int enablePin10 = 15;//A1
int enablePin11 = 16;//A2

///* Create a new instance for each sensor */
VL6180X sensor0;
VL6180X sensor1;
VL6180X sensor2;
VL6180X sensor3;
VL6180X sensor4;
VL6180X sensor5;
VL6180X sensor6;
VL6180X sensor7;
VL6180X sensor8;
VL6180X sensor9;
VL6180X sensor10;
VL6180X sensor11;

float rangeCutoff = 30;


void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC
//  SPI.begin();      // Init SPI bus
  Wire.begin();

  
  //Distance Sensors
  SetSensorI2CAddresses();

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

//  Serial.print("\tDistance0: ");
//  Serial.print(sensor0.readRangeContinuousMillimeters());
//  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
//
//  Serial.print("\tDistance1: ");
//  Serial.print(sensor1.readRangeContinuousMillimeters());
//  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
//
//  Serial.print("\tDistance2: ");
//  Serial.print(sensor2.readRangeContinuousMillimeters());
//  if (sensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
//  Serial.println();

  if( sensor0.readRangeContinuousMillimeters() <= rangeCutoff )
    SendKeystrokePacket( MSG_KEYPRESS_C );
  if( sensor1.readRangeContinuousMillimeters() <= rangeCutoff )
    SendKeystrokePacket( MSG_KEYPRESS_C_SHARP );
  if( sensor2.readRangeContinuousMillimeters() <= rangeCutoff )
    SendKeystrokePacket( MSG_KEYPRESS_D );
  if( sensor3.readRangeContinuousMillimeters() <= rangeCutoff )
    SendKeystrokePacket( MSG_KEYPRESS_D_SHARP );
  if( sensor4.readRangeContinuousMillimeters() <= rangeCutoff )
    SendKeystrokePacket( MSG_KEYPRESS_E );
  if( sensor5.readRangeContinuousMillimeters() <= rangeCutoff )
    SendKeystrokePacket( MSG_KEYPRESS_F );
//  if( sensor6.readRangeContinuousMillimeters() <= rangeCutoff )
//    SendKeystrokePacket( MSG_KEYPRESS_F_SHARP );
//  if( sensor7.readRangeContinuousMillimeters() <= rangeCutoff )
//    SendKeystrokePacket( MSG_KEYPRESS_G );
//  if( sensor8.readRangeContinuousMillimeters() <= rangeCutoff )
//    SendKeystrokePacket( MSG_KEYPRESS_G_SHARP );
//  if( sensor9.readRangeContinuousMillimeters() <= rangeCutoff )
//    SendKeystrokePacket( MSG_KEYPRESS_A );
//  if( sensor10.readRangeContinuousMillimeters() <= rangeCutoff )
//    SendKeystrokePacket( MSG_KEYPRESS_A_SHARP );
//  if( sensor11.readRangeContinuousMillimeters() <= rangeCutoff )
//    SendKeystrokePacket( MSG_KEYPRESS_B );

//    float lux = vl.readLux(VL6180X_ALS_GAIN_5);
//    
//    uint8_t range = vl.readRange();
//    uint8_t status = vl.readRangeStatus();
//
//    if (status == VL6180X_ERROR_NONE)
//    {
//
//      if (range < 20)
//        SendKeystrokePacket( MSG_KEYPRESS_C );
//      else if (range < 24)
//        SendKeystrokePacket( MSG_KEYPRESS_C_SHARP );
//      else if (range < 28)
//        SendKeystrokePacket( MSG_KEYPRESS_D );
//      else if (range < 32)
//        SendKeystrokePacket( MSG_KEYPRESS_D_SHARP );
//      else if (range < 36)
//        SendKeystrokePacket( MSG_KEYPRESS_E );
//      else if (range < 40)
//        SendKeystrokePacket( MSG_KEYPRESS_F );
//      else if (range < 44)
//        SendKeystrokePacket( MSG_KEYPRESS_F_SHARP );
//      else if (range < 58)
//        SendKeystrokePacket( MSG_KEYPRESS_G );
//      else if (range < 52)
//        SendKeystrokePacket( MSG_KEYPRESS_G_SHARP );
//      else if (range < 56)
//        SendKeystrokePacket( MSG_KEYPRESS_A );
//      else if (range < 60)
//        SendKeystrokePacket( MSG_KEYPRESS_A_SHARP );
//      else if (range < 64)
//        SendKeystrokePacket( MSG_KEYPRESS_B );
//    }
    
  
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

void SendKeystrokePacket( char cmd )
{

  Serial.print(F("SENDING Keypress: "));
  Serial.print(cmd);
  Serial.println();

  pressMessagePayload[0] = cmd;
  pressMessage.setFrameId(xbee.getNextFrameId());
  
  xbee.send(pressMessage);
  
//  // Send the command and wait up to N ms for a response.  xbee loop continues during this time.
//  uint8_t status = xbee.sendAndWait(pressMessage, 1000);
//  if (status == 0)
//  {
//    Serial.println(F("SEND ACKNOWLEDGED"));
//
//  } else { //Complain, but do not reset timeElapsed - so that a new packet comes in and tried again immedietly.
//    Serial.print(F("SEND FAILED: "));
//    printHex(status, 2);
//    Serial.println();
//    //flashSingleLed(LED_BUILTIN, 3, 500);
//  }
//
//  delay(1000);
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



void SetSensorI2CAddresses()
{
  // Reset all connected sensors
  pinMode(enablePin0,OUTPUT);
  pinMode(enablePin1,OUTPUT);
  pinMode(enablePin2,OUTPUT);
  pinMode(enablePin3,OUTPUT);
  pinMode(enablePin4,OUTPUT);
  pinMode(enablePin5,OUTPUT);
  pinMode(enablePin6,OUTPUT);
  pinMode(enablePin7,OUTPUT);
  pinMode(enablePin8,OUTPUT);
  pinMode(enablePin9,OUTPUT);
  pinMode(enablePin10,OUTPUT);
  pinMode(enablePin11,OUTPUT);
  
  digitalWrite(enablePin0, LOW);
  digitalWrite(enablePin1, LOW);
  digitalWrite(enablePin2, LOW);
  digitalWrite(enablePin3, LOW);
  digitalWrite(enablePin4, LOW);
  digitalWrite(enablePin5, LOW);
  digitalWrite(enablePin6, LOW);  
  digitalWrite(enablePin7, LOW);
  digitalWrite(enablePin8, LOW);
  digitalWrite(enablePin9, LOW);
  digitalWrite(enablePin10, LOW);
  digitalWrite(enablePin11, LOW);
  
  delay(1000);
  
  SetSensorI2CAddress(0, enablePin0, &sensor0, address0 );
  SetSensorI2CAddress(1, enablePin1, &sensor1, address1 );
  SetSensorI2CAddress(2, enablePin2, &sensor2, address2 );
  SetSensorI2CAddress(3, enablePin3, &sensor3, address3 );
  SetSensorI2CAddress(4, enablePin4, &sensor4, address4 );
  SetSensorI2CAddress(5, enablePin5, &sensor5, address5 );
  delay(1000);
 
  Serial.println("Sensors ready! Start reading sensors in 3 seconds ...!");
  delay(3000);

}

void SetSensorI2CAddress( int i, int enablePin, VL6180X *sensor, int address )
{

  Serial.print("Start Sensor: ");
  Serial.print(i);
  Serial.print(" using pin ");
  Serial.print(enablePin);
  Serial.print(" as I2C Address ");
  Serial.print( address);
  Serial.println();
  
  digitalWrite(enablePin, HIGH);
  delay(50);
  sensor->init();
  sensor->configureDefault();
  sensor->setAddress(address);
  Serial.println(sensor->readReg(0x212),HEX); // read I2C address
  sensor->writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor->writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
  sensor->setTimeout(500);
  sensor->stopContinuous();
  sensor->setScaling(RANGE); // configure range or precision 1, 2 oder 3 mm
  delay(300);
  sensor->startInterleavedContinuous(100);
  delay(100);
}

