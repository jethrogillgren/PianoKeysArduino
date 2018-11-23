const int ledPin = 13;

const int numButtons = 12;
const int buttonPins[numButtons] = {2,3,4,5,6,7,8,9,10,11,12,A0};

const char messages[numButtons] = {'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G', 'a', 'A', 'b'};

#define MSG_RESET   'r'


void setup() {
  Serial.begin(9600);   // Initialize serial communications with the PC

   // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  for(int i=0; i < numButtons; i++)
  {
    Serial.println();
    pinMode(buttonPins[i], INPUT);// initialize the pushbutton pin as an input:
    digitalWrite(buttonPins[i], HIGH); // connect internal pull-up so it is not floating
  }
}

void loop() {

  int numHeld = 0;

  //Foreach button
  for(int i=0; i < numButtons; i++)
  {
    if (digitalRead(buttonPins[i]) == LOW)
    {
      Serial.println( messages[i] );
      numHeld++;
    }
  
  }//End Foreach sensor

  if(numHeld>0)
  {
    digitalWrite(ledPin, HIGH);
    delay(100); //Prevent message flood
  } else {
    digitalWrite(ledPin, LOW);
  }
}
