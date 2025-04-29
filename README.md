# CSC235_GonkDroid
This is a repo to document GonkDroid (3Q-AA-2)

This is code for an original Gonk Droid (from Star Wars) that uses an Arduino Uno, keypad, and a 6px long NeoPixel strip.

The code has three stages:         
    The first is where the correct passcode needs to be entered into the keypad, and as that stage is happening, an LED turns   on for each button pressed, whether the code is correct or not. Once the correct sequence is pressed, the green LED lights up to indicate it was correct, and the code then goes to the second stage.
    The second stage is the "refill" stage, where the "battery" is refilled, and the neopixel strip incrementally lights up and once the strip is "full" (the px are all on) it flashes twice to indicate that it's done, and it then moves onto the third and final stage.
    The last stage is the timer, which in the end lasts 30 min. It starts with the strip completely off, but every 2.5 min, 1px turns on. For the first 2 px, the light is green, the second 2 are yellow, and the last 2 are red. Once the timer is over, it flashes red slowly but continuously until the pound sign is pressed, at which point the strip turns off and the cycle starts over.
