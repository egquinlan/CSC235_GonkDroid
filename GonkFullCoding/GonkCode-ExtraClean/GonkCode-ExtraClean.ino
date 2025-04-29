//   ---used chatGPT and just pasted my code from "GonkCode-cleanedUp" and said
//   ---"rewrite this code to be neater and more consise" 
//   ---and I changed a couple initalizations to test them out

#include <Keypad.h>
#include <Adafruit_NeoPixel.h>

// ========== CONFIGURATION ==========
const int numPixels = 6;
const int neoPixelPin = 13;
const int brightness = 150;

const int ledWhitePin = 10;
const int ledGreenPin = 11;

const char sequence[] = "456BCD";
int index = 0;

int state = 0; // 0 = input, 1 = refill, 2 = timer
bool isRefilling = false;
bool timerExpired = false;

// ========== TIMING ==========
unsigned long pixelDecreaseInterval = 150000; // 2.5 minutes = 150,000 ms (for test you can use 500)
unsigned long timerOverInterval = 2000;
unsigned long inputRedInterval = 850;
unsigned long refillInterval = 1500;
unsigned long refillFlashInterval = 200;

unsigned long ledOnStart = 0;
bool ledIsOn = false;

unsigned long lastPixelTime = 0;
int timerPixelIndex = numPixels - 1;
int timerStage = 1;

// ========== COLORS ==========
Adafruit_NeoPixel strip(numPixels, neoPixelPin, NEO_GRB + NEO_KHZ800);
uint32_t red, orange, yellow, green, white, bluishWhite;

// ========== KEYPAD SETUP ==========
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ============================
// SETUP
// ============================
void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.clear();
  strip.setBrightness(brightness);
  strip.show();

  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledWhitePin, OUTPUT);
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(ledWhitePin, LOW);

  setupColors();
}

// ============================
// MAIN LOOP
// ============================
void loop() {
  char key = keypad.getKey();
  if (key) delay(30); // Debounce

  keyPressedLight(key);
  finiteStateMachine(key);
}

// ============================
// FSM
// ============================
void finiteStateMachine(char key) {
  static int lastState = -1;

  if (state != lastState) {
    Serial.print("State changed to: ");
    Serial.println(state);
    lastState = state;

    if (state == 0) {
      resetTimerState();
      resetRefillState();
      timerExpired = false;
    }
  }

  switch (state) {
    case 0:
      handleKeypad(key);
      break;

    case 1:
      if (updateRefillPixels(bluishWhite)) {
        if (handleRefillEndLights(bluishWhite)) {
          Serial.println("Refill complete. Moving to timer...");
          digitalWrite(ledGreenPin, LOW); // Turn off green after refill
          state = 2;
        }
      }
      break;

    case 2:
      handleTimer(pixelDecreaseInterval, timerOverInterval, green, yellow, red, red);
      handleStopTimer(key);
      break;
  }
}

// ============================
// KEYPAD HANDLER
// ============================
void handleKeypad(char key) {
  if (!key || isRefilling) return;

  if (key == sequence[index]) {
    index++;
    if (index == sizeof(sequence) - 1) {
      Serial.println("Correct sequence entered!");
      digitalWrite(ledWhitePin, HIGH);
      digitalWrite(ledGreenPin, HIGH);
      isRefilling = true;
      state = 1;
      index = 0;
    }
  } else {
    Serial.println("Incorrect key, resetting...");
    index = 0;
  }
}

// ============================
// REFILL STAGE
// ============================
bool updateRefillPixels(uint32_t color) {
  static unsigned long lastUpdate = 0;
  static int litCount = 0;

  if (color == 0) {
    lastUpdate = 0;
    litCount = 0;
    return false;
  }

  if (millis() - lastUpdate >= refillInterval) {
    lastUpdate = millis();
    if (litCount < numPixels) {
      strip.setPixelColor(litCount, color);
      strip.show();
      litCount++;
    }
  }
  return (litCount >= numPixels);
}

bool handleRefillEndLights(uint32_t color) {
  static bool flashing = false;
  static unsigned long lastFlash = 0;
  static int flashCount = 0;
  static bool isOn = false;

  if (color == 0) {
    flashing = false;
    flashCount = 0;
    isOn = false;
    return false;
  }

  if (!flashing) {
    flashing = true;
    lastFlash = millis();
    flashCount = 0;
  }

  if (millis() - lastFlash >= refillFlashInterval) {
    lastFlash = millis();
    isOn = !isOn;
    for (int i = 0; i < numPixels; i++) {
      strip.setPixelColor(i, isOn ? color : 0);
    }
    strip.show();
    flashCount++;
  }

  if (flashCount >= 4) {
    flashing = false;
    flashCount = 0;
    return true;
  }
  return false;
}

void resetRefillState() {
  updateRefillPixels(0); // Reset internal variables
  handleRefillEndLights(0);
}

// ============================
// TIMER STAGE
// ============================
void handleTimer(unsigned long interval, unsigned long flashDelay, uint32_t c1, uint32_t c2, uint32_t flashColor, uint32_t finalColor) {
  static unsigned long lastFlash = 0;
  static bool flashing = false;
  unsigned long now = millis();

  if (timerPixelIndex >= 0 && now - lastPixelTime >= interval) {
    if (timerPixelIndex >= 4) strip.setPixelColor(timerPixelIndex, c1); // Green
    else if (timerPixelIndex >= 2) strip.setPixelColor(timerPixelIndex, c2); // Yellow
    else strip.setPixelColor(timerPixelIndex, flashColor); // Red

    strip.show();
    timerPixelIndex--;
    lastPixelTime = now;
  }

  if (timerPixelIndex < 0) {
    timerExpired = true;
    if (now - lastFlash >= flashDelay) {
      lastFlash = now;
      flashing = !flashing;
      for (int i = 0; i < numPixels; i++) {
        strip.setPixelColor(i, flashing ? finalColor : 0);
      }
      strip.show();
    }
  }
}

void handleStopTimer(char key) {
  if (key == '#') {
    Serial.println("Reset triggered.");
    strip.clear();
    strip.show();
    resetTimerState();
    resetRefillState();
    isRefilling = false;
    timerExpired = false;
    state = 0;
    index = 0;
  }
}

void resetTimerState() {
  lastPixelTime = 0;
  timerPixelIndex = numPixels - 1;
  timerStage = 1;
}

// ============================
// LED FEEDBACK
// ============================
void keyPressedLight(char key) {
  if (key) {
    digitalWrite(ledWhitePin, HIGH);
    ledIsOn = true;
    ledOnStart = millis();
  }

  if (ledIsOn && millis() - ledOnStart >= 400) {
    digitalWrite(ledWhitePin, LOW);
    ledIsOn = false;
  }
}

// ============================
// COLOR DEFINITIONS
// ============================
void setupColors() {
  red         = strip.Color(255, 0, 0);
  orange      = strip.Color(255, 80, 0);
  yellow      = strip.Color(255, 150, 0);
  green       = strip.Color(0, 255, 0);
  white       = strip.Color(255, 255, 255);
  bluishWhite = strip.Color(180, 220, 255);
}
