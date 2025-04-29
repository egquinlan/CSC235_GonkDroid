//final project rough draft
#include <Adafruit_NeoPixel.h>
#include <Keypad.h>

//my global vars

int state;
const int numPixels = 12;
const int neoPixelPin = 11;
int brightness = 150;

Adafruit_NeoPixel strip(numPixels, neoPixelPin);

// keypad vars
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'} // # is at row 4, column 3
};

int rowPins[ROWS] = {9, 8, 7, 6}; 
int colPins[COLS] = {5, 4, 3, 2}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


const int ledPinRed = 10;
const int ledPinWhite = 11; 
const int ledGreen = 13;
const int rRbgLedPin
const int ledPinRGB = 12; 

const char sequence[] = "123ABC"; // correct password

bool codeCorrect; // to track if the password is entered correctly

unsigned long startTime; // Track when LED sequence starts
int refillInteral = 5000;
unsigned long  pixalDecreaseInterval= 150000; // pixels will decrease every 2.5 min
int timerOverInterval = 2000;
int inputRedInterval = 3000;

//bool isState0;
//bool TimerOn = false; // Track if LEDs are on
bool isRefilling;

uint32_t red = strip.Color(255,0,0);
uint32_t orange = strip.Color(210,20,0);
uint32_t yellow = strip.Color(210,80,0);
uint32_t green = strip.Color(20,220,0);
uint32_t white = strip.Color(210,210,210);





void setup() {
  Serial.begin(9600);
  strip.begin();
  setPins();
  strip.setBrightness(brightness);

}





void loop() {
 handleTimer();
 handleKeypad();
}



//////////////////////////////////////////////////////////
//////////////// FUNCTION DEFINITIONS ////////////////////
//////////////////////////////////////////////////////////




/////////////////////helpful/////////////////////////
void setPins(){
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinWhite, OUTPUT);
  pinMode(ledPinRGB, OUTPUT);

  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinWhite, LOW);
  digitalWrite(ledPinRGB, LOW);
}

/////////////////////////////////////////


void finiteStateMachine(){
 Switch(state){

  case 0:
    //inputCode
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, LOW);
      Serial.println("state 0: LED1 ON");
  break;

  case 1:
    //refilling
      digitalWrite(ledPin1, HIGH);
      digitalWrite(ledPin2, HIGH);
      Serial.println("state 1: LED1 and LED2 ON");
  break;

  case 2:
    runningTimer(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t endColor);
  break;

  default:
  // you shouldn't be here
  Serial.println("you shouldn't be here");
  break;
 }
}

//handle keypad enter
void handleKeypad(){
  int index = 0; // Track current input position
  char key = keypad.getKey();
  //codeCorrect = false;

  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    digitalWrite (ledPinWhite, HIGH);

    if (!isRefilling) {
      // Check if the key matches the sequence
      if (key == sequence[index]) {
        index++;
        if (index == sizeof(sequence) - 1) { // Full sequence matched
          // digitalWrite(ledPin1, HIGH); // Turn LED1 ON
          state = 1; // go to refill state
          isRefilling = true;
          //ledsOn = true;
          startTime = millis(); // Record start time of LED sequence
          Serial.println("Correct sequence entered! refilling attention");
        }
      } else {
        index = 0; // Reset sequence if wrong key is pressed
      }

    } else {
    digitalWrite (ledPinWhite, LOW);
      // If LEDs are on, check if '#' is pressed to reset
      if (key == '#') {
       digitalWrite (ledPinWhite, LOW);
       reset(index);
      }
    }
  }
  
}


//handle reset
void reset(int i){
  strip.clear();
  // digitalWrite(led8Pin1, LOW);
  // digitalWrite(ledPin2, LOW);

  Serial.println("Pound key pressed! Both LEDs OFF.");
  isRefilling = false; // Reset the system
  i = 0; // Reset the sequence
  strip.show();
}





////////STATE 0: Input Code

//neopixel- 3sec on 3sec off- red
void handleInputCodeLight(uint32_t color1){
  strip.clear();
  int i;
  if (millis() - startTime >= inputRedInterval && i <= numPixels) {
      i++;
      strip.setPixelColor(numPixels, color1);
      startTime = millis();
  }
  else{
    strip.clear();
  }
  strip.show();
}

//read code

//if statements to move states




////////STATE 1: refill battery
void refillColorPattern(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t fullChargeColor){
  strip.clear();
  static int index = 0;
  strip.setPixelColor(numPixels, c1);

  if (millis() - startTime >= refillInteral && index <= numPixels) {
    index++;
    startTime = millis();

    for(int i = 0; i <=3; i++ ){
      strip.setPixelColor(i,c1);
      Serial.println("16% charged");
    }
    for(int i = 3; i <= 5; i++){
      strip.setPixelColor(i,c2);
      Serial.println("30-50% charged");
    }
    for(int i = 5; i <= 10; i++){
      strip.setPixelColor(i,c3);
      Serial.println("50-80 % charged");
    }
    for(int i = 10; i <= numpixels; i++){
      strip.setPixelColfor(i,c4);
      Serial.println("80-100% charged!!");
    }
  }
  else{
    refillEndPattern(index, fullChargeColor);
  }
  strip.show();
}

void refillEndPattern(int ix, uint32_t whiteColor){
    // Check if 15 seconds have passed, and cycle LEDs
  if(millis()- startTime >= timerOverInterval){

     for (ix = 0; ix <= numpixels; ix++) { 
      strip.setPixelColor(ix, whiteColor);
      strip.show();
  }
  else {
    strip.clear();
    strip.show();
  }
}



////////STATE 2: Timer
//--maybe done?

//timer for state changes
void runningTimer(uint32_t c1, uint32_t c2, uint32_t c3, uint32_t c4, uint32_t endColor){
  strip.clear();
  static int index = 0;
  strip.setPixelColor(numPixels, c1);

  if (millis() - startTime >= pixalDecreaseInterval && index >= 0) {
    index = index- 1;
    startTime = millis();

    for(int i = numPixels; i >= 5; i = i-1 ){
      strip.setPixelColor(i,c1);
      Serial.println("more than 50% battery");
    }

    for(int i = 5; i >= 3; i = i-1 ){
      strip.setPixelColor(i,c2);
      Serial.println("50-30% remaining");

    }

    for(int i = 3; i >= 1; i = i-1 ){
      strip.setPixelColor(i,c3);
      Serial.println("30- 16% remaining");
    }

    for(int i = 1; i >= 0; i = i-1){
      strip.setPixelColfor(i,c4);
      Serial.println("less than 15% remaining!!");
    }
  }
  else{
    handleTimerEnd(index, endColor);
  }
  strip.show();
}



void handleTimerEnd(int ix, uint32_t redColor){
    // Check if 15 seconds have passed, and cycle LEDs
  if(millis()- startTime >= timerOverInterval){

     for (ix = 0; ix <= numpixels; ix++) { 
      strip.setPixelColor(ix, redColor);
      strip.show();  
  }
  else{
    strip.clear();
    strip.show();
  }
}
