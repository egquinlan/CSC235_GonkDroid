//gonk code cleaned up version

#include <Keypad.h>
#include <Adafruit_NeoPixel.h>

// ========== CONFIGURATION ==========
const int numPixels = 6;
const int neoPixelPin = 13;
int brightness = 150;

const int ledWhitePin = 10;
const int ledGreenPin = 11;

const char sequence[] = "456BCD";
int index;

int state; // 0 = input, 1 = refill, 2 = timer
bool isRefilling = false;
bool timerExpired = false;

// ========== TIMING ==========
unsigned long pixalDecreaseInterval = 150000; // 2.5 minutes-- to test I recoment = 500
unsigned long timerOverInterval = 2000;
unsigned long inputRedInterval = 1000;
int refillInterval = 1500;
int refillFlashInterval = 200;

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




void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.clear();
  setPinModes();
  setupColors();
  strip.setBrightness(brightness);
  strip.show();

  state = 0;
  index = 0;
  timerPixelIndex = numPixels - 1;
}

void loop() {
  char key = keypad.getKey();
  if (key) delay(100); // simple debounce

  keyPressedLight(key);
  finiteStateMachine(key);
  resetRefillState();
}


// ========== FSM ==========

void finiteStateMachine(char key) {
  static int lastState = -1;
  if (state != lastState) {
    Serial.print("State changed to: ");
    Serial.println(state);
    lastState = state;
  }

  switch (state) {
    case 0:
      resetTimerState();
      handleKeypad(key);
      if (timerExpired) handleTimerEndLighting(inputRedInterval, red);
      break;

    case 1:
      if (updateRefillPixels(bluishWhite)) {
        if (handleRefillEndLights(bluishWhite)) {
          Serial.println("Refill complete. Moving to timer...");
          state = 2;
        }
      }
      break;

    case 2:
      isRefilling = false;
      digitalWrite(ledGreenPin, LOW);
      handleTimer(pixalDecreaseInterval, timerOverInterval, green, yellow, orange, red);
      handleStopTimer(key);
      break;
  }
}


// ========== LIGHTING HELPERS ==========

void setupColors() {
  red         = strip.Color(255, 0, 0);
  orange      = strip.Color(210, 20, 0);
  yellow      = strip.Color(210, 80, 0);
  green       = strip.Color(20, 220, 0);
  white       = strip.Color(210, 210, 210);
  bluishWhite = strip.Color(150, 200, 255);
}

void setPinModes() {
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledWhitePin, OUTPUT);
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(ledWhitePin, LOW);
}

// ========== KEYPAD ==========

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


// ========== REFILL ==========

bool updateRefillPixels(uint32_t refillColor) {
  static unsigned long lastUpdate = 0;
  static int litCount = 0;

  if (refillColor == 0) {
    lastUpdate = 0;
    litCount = 0;
    return false;
  }

  unsigned long now = millis();

  if (litCount < numPixels && now - lastUpdate >= refillInterval) {
    lastUpdate = now;
    strip.setPixelColor(litCount, refillColor);
    strip.show();
    litCount++;
  }
  return (litCount >= numPixels);  // true if done
}


bool handleRefillEndLights(uint32_t c1) {
  static bool flashStarted = false;
  static unsigned long lastFlash = 0;
  static int flashStep = 0;
  static bool isOn = false;

  if (c1 == 0) {
    flashStarted = false;
    lastFlash = 0;
    flashStep = 0;
    isOn = false;
    return false;
  }

  unsigned long now = millis();

  if (!flashStarted) {
    bool allLit = true;
    for (int i = 0; i < numPixels; i++) {
      if (strip.getPixelColor(i) == 0) {
        allLit = false;
        break;
      }
    }
    if (allLit) {
      flashStarted = true;
      lastFlash = now;
    }
  }

  if (flashStarted && flashStep < 4 && now - lastFlash >= refillFlashInterval) {
    lastFlash = now;
    isOn = !isOn;

    for (int i = 0; i < numPixels; i++) {
      strip.setPixelColor(i, isOn ? c1 : 0);
    }
    strip.show();
    flashStep++;
  }

  if (flashStep >= 4) {
    flashStarted = false;
    flashStep = 0;
    return true;
  }

  return false;
}

void resetRefillState() {
  static bool firstCall = true;
  static int lastState = -1;

  if (state == 1 && lastState != 1) {
    // Reset static variables from refill functions
    Serial.println("Resetting refill state variables.");
    updateRefillPixels(0);  // Use 0 to trigger static reset
    handleRefillEndLights(0); // Use 0 to trigger static reset
  }
  lastState = state;
}


// ========== TIMER ==========

void handleTimer(unsigned long interval, unsigned long flashDelay, uint32_t c1, uint32_t c2, uint32_t c3, uint32_t flashColor) {
  unsigned long now = millis();

  if (timerPixelIndex >= 0 && now - lastPixelTime >= interval) {
    if (timerStage == 1 && timerPixelIndex > 3) {
      Serial.println("Stage 1 (Green)");
      strip.setPixelColor(timerPixelIndex, c1);
    } else if (timerPixelIndex <= 3 && timerPixelIndex > 1) {
      timerStage = 2;
      Serial.println("Stage 2 (Yellow)");
      strip.setPixelColor(timerPixelIndex, c2);
    } else if (timerPixelIndex <= 1) {
      timerStage = 3;
      Serial.println("Stage 3 (Red)");
      strip.setPixelColor(timerPixelIndex, c3);
    }

    strip.show();
    timerPixelIndex--;
    lastPixelTime = now;
  }

  if (timerPixelIndex < 0) {
    timerExpired = true;
    handleTimerEndLighting(flashDelay, flashColor);
  }
}

void handleTimerEndLighting(int flashInterval, uint32_t c1) {
  static unsigned long lastFlash = 0;
  static bool ledOn = false;
  unsigned long currentTime = millis();

  if (currentTime - lastFlash >= flashInterval) {
    lastFlash = currentTime;
    ledOn = !ledOn;

    for (int i = 0; i < numPixels; i++) {
      strip.setPixelColor(i, ledOn ? c1 : 0);
    }
    strip.show();
  }
}

void handleStopTimer(char key) {
  if (key == '#') {
    Serial.println("Timer interrupted. Returning to input.");
    strip.clear();
    strip.show();

    resetTimerState();
    isRefilling = false;  // ✅ ADD THIS LINE
    timerExpired = false;
    state = 0;
  }
}


void resetTimerState() {
  lastPixelTime = 0;
  timerPixelIndex = numPixels - 1;
  timerStage = 1;
}


// ========== FEEDBACK LIGHT ==========

void keyPressedLight(char key) {
  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);
    digitalWrite(ledWhitePin, HIGH);
    ledIsOn = true;
    ledOnStart = millis();
  }

  if (ledIsOn && millis() - ledOnStart >= 400) {
    digitalWrite(ledWhitePin, LOW);
    ledIsOn = false;
  }
}



