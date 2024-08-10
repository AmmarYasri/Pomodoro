#include "TickTwo.h"

// RGB LED setup
const int RGB_RED_PIN = 25;
const int RGB_GREEN_PIN = 26;
const int RGB_BLUE_PIN = 27;

//Shift register setup
#define SHIFT_CLK 23
#define SHIFT_DATA 21
#define SHIFT_LATCH 22

// Function declarations
void pomoloop();
void breakloop();
void checkButtonState();

// Time elements of Pomodoro and Break are independent
int Pminute = 24;
int Psecond = 59;
int Bminute = 4;
int Bsecond = 59;
int Psum = (60 * Pminute) + Psecond;
int Bsum = (60 * Bminute) + Bsecond;

// Buttons
int buttonPin = 12;
int buzzerPin = 5;
int forwardPin = 13;

//byte variables for conversion of decimal to BCD
byte AddressArr[158];
byte address1;
byte address2;
byte address3;
byte address4;
byte result1;
byte result2;

// Timers
TickTwo timerP(pomoloop, 1000, Psum);
TickTwo timerB(breakloop, 1000, Bsum);

// State variables
bool PomoStart = true;
bool BreakStart = false;
bool PomoEnd = false;
bool BreakEnd = false;
bool isPaused = false;
bool pauseButtonPressed = false;
bool pomodoroRunning = false;
bool breakRunning = false;
bool forward = false;

// LED RGB
bool ledPink = false;
bool ledBlue = false;

byte dec_to_bcd(byte dec)
{
  byte result = 0;
  
  result |= (dec / 10) << 4;
  result |= (dec % 10) << 0;
  
  return result;
}

void pomoloop() {
  // Infinite Pomodoro Loop
  if (!isPaused && pomodoroRunning) {
    
    address1 = dec_to_bcd(Psecond);
    address2 = dec_to_bcd(Pminute);
    
    shiftOut(SHIFT_DATA, SHIFT_CLK, LSBFIRST, address1);
    shiftOut(SHIFT_DATA, SHIFT_CLK, LSBFIRST, address2);
    
    
    digitalWrite(SHIFT_LATCH, LOW);
    
    
    Psecond--;
    if (Psecond == 0 && Pminute != 0) {
      Pminute--;
      Psecond = 59;
    } else if (Psecond == 0 && Pminute == 0) {
      Serial.println("Time for a break");
      buzzBuzzer();  // Call the buzzer function
      PomoEnd = true;
      BreakStart = true;
      pomodoroRunning = false;
      breakRunning = true;
    }
    digitalWrite(SHIFT_LATCH, HIGH);
  }
}

void breakloop() {
  // Infinite Break Loop
  if (!isPaused && breakRunning) {
    
    address3 = dec_to_bcd(Bsecond);
    address4 = dec_to_bcd(Bminute);
    
    shiftOut(SHIFT_DATA, SHIFT_CLK, LSBFIRST, address3);
    shiftOut(SHIFT_DATA, SHIFT_CLK, LSBFIRST, address4);

    
    digitalWrite(SHIFT_LATCH, LOW);
    
    
    Bsecond--;
    if (Bsecond == 0 && Bminute != 0) {
      Bminute--;
      Bsecond = 59;
    } else if (Bsecond == 0 && Bminute == 0) {
      Serial.println("Time to get to work");
      buzzBuzzer();  // Call the buzzer function
      BreakEnd = true;
      PomoStart = true;
      breakRunning = false;
      pomodoroRunning = true;
    }

    digitalWrite(SHIFT_LATCH, HIGH);  
    
  }
}

void checkButtonState() {
  // Check the state of the pause button
  if (digitalRead(buttonPin) == LOW) {
    if (!pauseButtonPressed) {
      pauseButtonPressed = true;

      if (isPaused) {
        // Button pressed, resume timer for the current phase
        if (breakRunning) {
          timerB.resume();
        } else if (pomodoroRunning) {
          timerP.resume();
        }
        isPaused = false;
        Serial.println("Resumed");
      } else {
        // Button pressed, pause timers
        timerP.pause();
        timerB.pause();
        isPaused = true;
        Serial.println("Paused");
      }
    }
    delay(200); // Add a small delay to debounce the button
  } else {
    pauseButtonPressed = false;
  }

  // Check the state of the forward button
  if (digitalRead(forwardPin) == LOW) {
    if (!forward) {
      forward = true;
      if (pomodoroRunning) {
        Serial.println("Skipping to break");
        delay(1000);
        PomoEnd = true;
        BreakStart = true;
        pomodoroRunning = false;
        breakRunning = true;
      } else if (breakRunning) {
        Serial.println("Skipping to Pomodoro");
        delay(1000);
        BreakEnd = true;
        PomoStart = true;
        breakRunning = false;
        pomodoroRunning = true;
      }
    }
    delay(200); // Add a small delay to debounce the button
  } else {
    forward = false;
  }
}

void setRgbLedColor(int red, int green, int blue)
{
  // If a common anode LED, invert values
  
  red = !red;
  green = !green;
  blue = !blue;
  
  digitalWrite(RGB_RED_PIN, red);
  digitalWrite(RGB_GREEN_PIN, green);
  digitalWrite(RGB_BLUE_PIN, blue);  
}

void setup() {
  //peripheral_setup();
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(forwardPin, INPUT_PULLUP);
  pinMode(18, OUTPUT);

  //RGB LED Setup
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);


  //Shift Register Setup
  pinMode(SHIFT_DATA, OUTPUT);
  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  
}

void loop() {
  //peripheral_loop();
  checkButtonState();

  if (PomoStart) {
    timerP.start();
    PomoStart = false;
    pomodoroRunning = true;
    ledPink = true;
    setRgbLedColor(HIGH, LOW, HIGH);    
  }

  timerP.update();

  if (PomoEnd) {
    timerP.stop();
    Pminute = 24;
    Psecond = 59;
    PomoEnd = false;
    ledPink = false;
    setRgbLedColor(LOW, LOW, LOW);    
  }

  if (BreakStart) {
    timerB.start();
    BreakStart = false;
    breakRunning = true;
    ledBlue = true;
    setRgbLedColor(LOW, LOW, HIGH);
  }

  timerB.update();

  if (BreakEnd) {
    timerB.stop();
    Bminute = 4;
    Bsecond = 59;
    BreakEnd = false;
    ledBlue = false;
    setRgbLedColor(LOW, LOW, LOW);
  }
}

void buzzBuzzer() {
  tone(buzzerPin, 10000, 500);
  delay(3000);
  noTone(buzzerPin);
  delay(1000);
}
