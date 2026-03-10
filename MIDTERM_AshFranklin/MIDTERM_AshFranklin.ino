#include <ESP32Servo.h>

// --- PIN DEFINITIONS ---
const int POT_PIN = 1;
const int LDR_PIN = 2;
const int BUTTON_PIN = 4;
const int STATUS_LED = 5;
const int PWM_LED = 6;
const int BUZZER_PIN = 7;
const int SERVO_PIN = 18;

// --- LOGIC VARIABLES ---
Servo gaugeServo;
int timerOptions[] = {10, 30, 60, 120};
bool isRunning = false;
bool isPaused = false;
bool autoPaused = false;
unsigned long startTime = 0;
unsigned long intervalMs = 0;
unsigned long pausedAt = 0;
unsigned long totalPausedTime = 0;
int lightThreshold = 0;

unsigned long buttonPressedTime = 0;
const int LONG_PRESS_MS = 1000;

float filteredPot = 0;
const float emaAlpha = 0.08;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  ESP32PWM::allocateTimer(0);
  gaugeServo.setPeriodHertz(50);
  gaugeServo.attach(SERVO_PIN, 500, 2400);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);
  pinMode(PWM_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Calibration
  long sum = 0;
  for(int i = 0; i < 20; i++) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
    sum += analogRead(LDR_PIN);
    delay(100);
  }
  lightThreshold = (sum / 20) * 0.8;
  digitalWrite(STATUS_LED, LOW);
  filteredPot = analogRead(POT_PIN);
}

void loop() {
  int rawPot = analogRead(POT_PIN);
  int lightLevel = analogRead(LDR_PIN);
  int btnState = digitalRead(BUTTON_PIN);
  static int lastBtnState = HIGH;

  filteredPot = (rawPot * emaAlpha) + (filteredPot * (1.0 - emaAlpha));
  int selection = map((int)filteredPot, 0, 4095, 0, 3);
  selection = constrain(selection, 0, 3);

  // --- AUTO-PAUSE/RESUME LOGIC (The Fix is here) ---
  if (isRunning) {
    // 1. TRIGGER AUTO-PAUSE
    if (!isPaused && lightLevel < lightThreshold) {
      isPaused = true;
      autoPaused = true;
      pausedAt = millis();
      tone(BUZZER_PIN, 300, 200); // Play ONCE
    } 
    // 2. TRIGGER AUTO-RESUME (Added 'isPaused' and 'autoPaused' check to prevent looping beeps)
    else if (isPaused && autoPaused && lightLevel >= lightThreshold) {
      isPaused = false;
      autoPaused = false;
      totalPausedTime += (millis() - pausedAt);
      tone(BUZZER_PIN, 900, 100); // Play ONCE
    }
  }

  // --- BUTTON LOGIC ---
  if (btnState == LOW && lastBtnState == HIGH) buttonPressedTime = millis();

  if (btnState == HIGH && lastBtnState == LOW) {
    unsigned long holdDuration = millis() - buttonPressedTime;
    if (holdDuration > LONG_PRESS_MS) {
      if (isRunning) { tone(BUZZER_PIN, 200, 500); endSession(); }
    } else {
      if (!isRunning && lightLevel > lightThreshold) {
        isRunning = true; isPaused = false; autoPaused = false;
        startTime = millis(); totalPausedTime = 0;
        intervalMs = (unsigned long)timerOptions[selection] * 1000;
        tone(BUZZER_PIN, 1000, 100);
      } else if (isRunning) {
        isPaused = !isPaused;
        autoPaused = false; 
        if (isPaused) { pausedAt = millis(); tone(BUZZER_PIN, 400, 100); }
        else { totalPausedTime += (millis() - pausedAt); tone(BUZZER_PIN, 800, 100); }
      }
    }
  }
  lastBtnState = btnState;

  // --- EXECUTION ---
  if (isRunning) {
    if (isPaused) {
      int blinkRate = autoPaused ? 200 : 1000;
      digitalWrite(STATUS_LED, (millis() % (blinkRate * 2) < blinkRate) ? HIGH : LOW);
    } else {
      unsigned long elapsed = millis() - startTime - totalPausedTime;
      analogWrite(PWM_LED, map(elapsed, 0, intervalMs, 255, 0));
      digitalWrite(STATUS_LED, HIGH);
      gaugeServo.write(map(elapsed, 0, intervalMs, 0, 180));
      if (elapsed >= intervalMs) endSession();
    }
  } else {
    // Idle Mode
    analogWrite(PWM_LED, map((int)filteredPot, 0, 4095, 10, 255));
    gaugeServo.write(0);
    noTone(BUZZER_PIN); // Ensure silence when idle
  }
}

void endSession() {
  isRunning = false; isPaused = false; autoPaused = false;
  digitalWrite(STATUS_LED, LOW);
  analogWrite(PWM_LED, 0);
  for (int i = 0; i < 4; i++) { tone(BUZZER_PIN, 600 + (i * 150), 100); delay(150); }
  noTone(BUZZER_PIN);
  gaugeServo.write(0);
}
