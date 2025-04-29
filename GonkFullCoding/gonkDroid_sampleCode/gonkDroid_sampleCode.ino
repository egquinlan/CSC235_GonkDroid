//gonk droid- sample code?
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>


int state;
const int numPixels = 5;
const int neoPixelPin = 13;
int brightness = 150;

Adafruit_NeoPixel strip(numPixels, neoPixelPin);

const int ledWhitePin = 10;
const int ledRedPin = 11;

// keypad vars
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'} // # is at row 4, column 3
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

const char sequence[] = "123ABC"; // correct password

int ledStartTime;
bool isRefilling;

unsigned long startTime; // Track when LED sequence starts
int refillInteral = 5000;
unsigned long  pixalDecreaseInterval= 150000; // pixels will decrease every 2.5 min
int timerOverInterval = 2000;
int inputRedInterval = 3000;

uint32_t red = strip.Color(255,0,0);
uint32_t orange = strip.Color(210,20,0);
uint32_t yellow = strip.Color(210,80,0);
uint32_t green = strip.Color(20,220,0);
uint32_t white = strip.Color(210,210,210);



void setup(){
  Serial.begin(9600);
  setPins();
  ledStartTime = millis();
}

void loop() {
  keyPressedLight();
  finiteStateMachine();
}

//////////////////////////////////////////////////////////
//////////////// FUNCTION DEFINITIONS ////////////////////
//////////////////////////////////////////////////////////

void finiteStateMachine(){
 switch(state){

  case 0:
    //inputCode
     handleKeypad();
  break;


  case 1:
    //refilling 
  break;


  case 2:
    //running timer
  break;


  default:
  Serial.println("you shouldn't be here");
  break;
 }
}


/////////////////////helpful/////////////////////////
void setPins(){
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledWhitePin, OUTPUT);

  digitalWrite(ledRedPin, LOW);
  digitalWrite(ledWhitePin, LOW);
}


/////////////////////functions/////////////////////////


//white led goes on when a key is pressed
void keyPressedLight(){
    char key = keypad.getKey();
    bool ledIsOn;

    if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    ledIsOn = true;
    digitalWrite(ledWhitePin, HIGH);
    ledStartTime = millis();
    }

    if (millis() - ledStartTime >= 400 && ledIsOn == true) {
      digitalWrite(ledWhitePin, LOW);
      ledStartTime = millis(); // Reset the timer for next cycle
      ledIsOn = false;
    }
}



//handle keypad enter
void handleKeypad(){
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    if (!isRefilling) {
      // Check if the key matches the sequence
      readSequenceInput(index);
    }
  }
}

void readSequenceInput() {
  char key = keypad.getKey();
  if (!key) return;

  if (key == sequence[index]) {
    index++;
    if (index == sizeof(sequence) - 1) {
      isRefilling = true;
      state = 1; // go to refill state
      Serial.println("Correct sequence entered! Refilling...");
      index = 0;
    }
  } else {
    index = 0;
    Serial.println("Wrong key, resetting input");
  }
}






//handle reset
void reset(int i){
  if(key == '#'){
  digitalWrite(ledRedPin, LOW);
  digitalWrite(ledWhitePin, LOW);
  Serial.println("Resetting sequence");
  isRefilling = false; // Reset the system
  i = 0; // Reset the sequence
  }
}



