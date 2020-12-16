#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Rotary.h>
#include "RotaryEncOverMCP.h"
#include <Key.h>
#include <Keypad.h>
#include <Joystick.h>


/* Our I2C MCP23017 GPIO expanders */
Adafruit_MCP23017 mcp;

//Array of pointers of all MCPs if there is more than one
Adafruit_MCP23017* allMCPs[] = { &mcp };
constexpr int numMCPs = (int)(sizeof(allMCPs) / sizeof(*allMCPs));

/* function prototypes */
void RotaryEncoderChanged(bool clockwise, int id);

/* Array of all rotary encoders and their pins */
RotaryEncOverMCP rotaryEncoders[] = {
        // outputA,B on GPA7,GPA6, register with callback and ID=1
        RotaryEncOverMCP(&mcp, 6, 5, &RotaryEncoderChanged, 1),
        RotaryEncOverMCP(&mcp, 3, 2, &RotaryEncoderChanged, 2),
        RotaryEncOverMCP(&mcp, 10, 9, &RotaryEncoderChanged, 3),
        RotaryEncOverMCP(&mcp, 12, 13, &RotaryEncoderChanged, 4),
        RotaryEncOverMCP(&mcp, 15, 1, &RotaryEncoderChanged, 5)
        
};

 
constexpr int numEncoders = (int)(sizeof(rotaryEncoders) / sizeof(*rotaryEncoders));

const int ButPins[numEncoders] = {// Switch pin
   7,   // Enc 1 --> GPA7
   4,   // Enc 2 --> GPA4
   8,   // Enc 3 --> GPB0
   11,  // Enc 4 --> GPB5
   14}; // Enc 5 --> GPB6

int PrevEncButtonState[numEncoders];

//keypad
const byte ROWS = 3; //three rows
const byte COLS = 5; //5 columns

char keys[ROWS][COLS] = {
    {'1', '2', '3', '4', '5'},
    {'6', '7', '8', '9', '0'},
    {'a', 'b', 'c', 'd', 'e'}
};

byte rowPins[ROWS] = {10, 16, 14}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9 , 8 , 7, 6 , 5}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//keypad
int switchpins[3] = {15 , 19 , 21};
int gearswitchpins[2] = {20, 18};
int prevstate[5] = {0,0,0};
int prevstategear[2];

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  35, 0,                  // Button Count, Hat Switch Count
  false, false, false,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

  
// setup the push buttons
void butPinsSetup(const int pin) {
  mcp.pinMode(pin, INPUT);
  mcp.pullUp(pin, HIGH); 
}


void RotaryEncoderChanged(bool clockwise, int id) {
    //Serial.println("Encoder " + String(id) + ": "
     //       + (clockwise ? String("clockwise") : String("counter-clock-wise")));
            switch(id){
    case 1:
    if(clockwise)
      {
     Joystick.setButton(6, 1); 
     delay(50);
     Joystick.setButton(6, 0);
      }
      else
      {
     Joystick.setButton(5, 1); 
     delay(50);
     Joystick.setButton(5, 0);
      } 
     break;
    case 2:
     if(clockwise)
      {
     Joystick.setButton(8, 1); 
     delay(50);
     Joystick.setButton(8, 0);
      }
      else
      {
     Joystick.setButton(7, 1); 
     delay(50);
     Joystick.setButton(7, 0);
      } 
     break;
    case 3:
     if(clockwise)
      {
     Joystick.setButton(10, 1); 
     delay(50);
     Joystick.setButton(10, 0);
      }
      else
      {
     Joystick.setButton(9, 1); 
     delay(50);
     Joystick.setButton(9, 0);
      } 
     break;
    case 4:
     if(clockwise)
      {
     Joystick.setButton(12, 1); 
     delay(50);
     Joystick.setButton(12, 0);
      }
      else
      {
     Joystick.setButton(11, 1); 
     delay(50);
     Joystick.setButton(11, 0);
      } 
     break;
    case 5:
     if(clockwise)
      {
     Joystick.setButton(14, 1); 
     delay(50);
     Joystick.setButton(14, 0);
      }
      else
      {
     Joystick.setButton(13, 1); 
     delay(50);
     Joystick.setButton(13, 0);
      } 
     break;
    }
    
}

void setup(){
    Serial.begin(115200);
    Serial.println("MCP23007 Polling Test");

    mcp.begin(0x27);      // use default address 0

    //Initialize input encoders (pin mode, interrupt)
    for(int i=0; i < numEncoders; i++) {
        rotaryEncoders[i].init();
        butPinsSetup(ButPins[i]);
    }

    keypad.addEventListener(keypadEvent); // Add an event listener for this keypad

    //setup switch pins
   for(int i = 0; i < 3; i++)
   {
    pinMode(switchpins[i], INPUT);
    digitalWrite(switchpins[i], HIGH);
   }

   for(int i = 0; i < 2; i++)
   {
     pinMode(gearswitchpins[i], INPUT);
     digitalWrite(gearswitchpins[i], HIGH);
     prevstategear[i] = digitalRead(gearswitchpins[i]);
    
   }

   Joystick.begin();
}


void pollAll() {
    //We could also call ".poll()" on each encoder,
    //however that will cause an I2C transfer
    //for every encoder.
    //It's faster to only go through each MCP object,
    //read it, and then feed it into the encoder as input.
    for(int j = 0; j < numMCPs; j++) {
        uint16_t gpioAB = allMCPs[j]->readGPIOAB();
        for (int i=0; i < numEncoders; i++) {
            //only feed this in the encoder if this
            //is coming from the correct MCP
            if(rotaryEncoders[i].getMCP() == allMCPs[j])
                rotaryEncoders[i].feedInput(gpioAB);
        }
    }
}

void ReadEncButton(){
int state;
  for(int i = 0; i < numEncoders; i++)
  {
    state = mcp.digitalRead(ButPins[i]);
    if(state != PrevEncButtonState[i])
    {
      //Serial.println(i);
      Joystick.setButton(i, !state); 
      PrevEncButtonState[i] = state;
    }
    
    
  }
  
}

void loop() {
    pollAll();
    ReadEncButton();
    readswitchs();
    readgearswitchs();

    char key = keypad.getKey();

    if (key) {
        Serial.println(key);
    }
}

void keypadEvent(KeypadEvent key){

 if (key) {
        Serial.println(key);
    }
    switch (keypad.getState()){
    case PRESSED:
        switch(key){
          case '1':
          Joystick.setButton(15, 1); 
          break;
          case '2':
          Joystick.setButton(16, 1); 
          break;
          case '3':
          Joystick.setButton(17, 1); 
          break;
          case '4':
          Joystick.setButton(18, 1); 
          break;
          case '5':
          Joystick.setButton(19, 1); 
          break;
          case '6':
          Joystick.setButton(20, 1); 
          break;
          case '7':
          Joystick.setButton(21, 1); 
          break;
          case '8':
          Joystick.setButton(22, 1); 
          break;
          case '9':
          Joystick.setButton(23, 1); 
          break;
          case '0':
          Joystick.setButton(24, 1); 
          break;
          case 'a':
          Joystick.setButton(25, 1); 
          break;
          case 'b':
          Joystick.setButton(26, 1); 
          break;
          case 'c':
          Joystick.setButton(27, 1); 
          break;
          case 'd':
          Joystick.setButton(28, 1); 
          break;
          case 'e':
          Joystick.setButton(29, 1); 
          break;
          }
        
        break;

    case RELEASED:
         switch(key){
          case '1':
          Joystick.setButton(15, 0); 
          break;
          case '2':
          Joystick.setButton(16, 0); 
          break;
          case '3':
          Joystick.setButton(17, 0); 
          break;
          case '4':
          Joystick.setButton(18, 0); 
          break;
          case '5':
          Joystick.setButton(19, 0); 
          break;
          case '6':
          Joystick.setButton(20, 0); 
          break;
          case '7':
          Joystick.setButton(21, 0); 
          break;
          case '8':
          Joystick.setButton(22, 0); 
          break;
          case '9':
          Joystick.setButton(23, 0); 
          break;
          case '0':
          Joystick.setButton(24, 0); 
          break;
          case 'a':
          Joystick.setButton(25, 0); 
          break;
          case 'b':
          Joystick.setButton(26, 0); 
          break;
          case 'c':
          Joystick.setButton(27, 0); 
          break;
          case 'd':
          Joystick.setButton(28, 0); 
          break;
          case 'e':
          Joystick.setButton(29, 0); 
          break;
          }
        break;

    case HOLD:
    Serial.print("held ");
         Serial.println(key);
        break;
    }
}

void readswitchs(){

int state;
  for(int i = 0;i < 3;i++)
  {
    state = digitalRead(switchpins[i]);
    if(state != prevstate[i])
    {
      Serial.print("switch");
      Serial.println(i);
      Joystick.setButton(30 + i, 1);
      delay(50);
      Joystick.setButton(30 + i, 0); 
      prevstate[i] = state;
    }
  }
}

void readgearswitchs()
{
  int state;
  for(int i = 0;i < 2;i++)
  {
    state = digitalRead(gearswitchpins[i]);
    if(state != prevstategear[i])
    {
      Serial.print("gearswitch");
      Serial.println(i);
      Joystick.setButton(33 + i, state);
      prevstategear[i] = state;
    }
  }
}
