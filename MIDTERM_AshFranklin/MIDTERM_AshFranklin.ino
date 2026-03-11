#include <ESP32Servo.h> // Library for controlling servos with ESP32-compatible PWM timers

// --- PIN DEFINITIONS ---
const int POT_PIN = 1;      // Potentiometer for time selection (Warning: Pin 1 is usually Serial TX)
const int LDR_PIN = 2;      // Light Dependent Resistor to sense ambient brightness
const int BUTTON_PIN = 4;   // Input button for starting/pausing/stopping the timer
const int STATUS_LED = 5;   // LED indicating if the system is running or paused
const int PWM_LED = 6;      // LED that fades as the timer counts down
const int BUZZER_PIN = 7;   // Piezo buzzer for audio alerts
const int SERVO_PIN = 18;   // Pin connected to the servo signal wire

// --- LOGIC VARIABLES ---
Servo gaugeServo;           // Create a servo object to control the motor
int timerOptions[] = {10, 30, 60, 120}; // Preset countdown durations in seconds
bool isRunning = false;     // State flag: is the countdown active?
bool isPaused = false;      // State flag: is the countdown currently halted?
bool autoPaused = false;    // State flag: was the pause triggered by the LDR (darkness)?
unsigned long startTime = 0;    // Tracks the exact moment the timer began
unsigned long intervalMs = 0;   // The total selected time converted to milliseconds
unsigned long pausedAt = 0;     // Timestamp for when a pause starts
unsigned long totalPausedTime = 0; // Total time spent in pause to subtract from elapsed time
int lightThreshold = 0;         // Calibration point to distinguish "light" from "dark"

unsigned long buttonPressedTime = 0; // Records when the button was first pushed
const int LONG_PRESS_MS = 1000;      // Hold time required to register a long press (1 second)

float filteredPot = 0;      // Smoothed reading from the potentiometer
const float emaAlpha = 0.08; // Filter strength (0.01-1.0); smaller is smoother but slower

void setup() {
  Serial.begin(115200);     // Initialize serial monitor for debugging output
  analogReadResolution(12); // Set ADC resolution to 12-bit (0 to 4095)
  ESP32PWM::allocateTimer(0); // Assign a hardware timer for PWM/Servo control
  gaugeServo.setPeriodHertz(50); // Set standard servo frequency (50Hz)
  gaugeServo.attach(SERVO_PIN, 500, 2400); // Attach servo and define pulse range (ms)
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal resistor to keep pin HIGH when not pressed
  pinMode(STATUS_LED, OUTPUT);
  pinMode(PWM_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Calibration: Average 20 light readings to set the dark threshold
  long sum = 0;
  for(int i = 0; i < 20; i++) {
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED)); // Flash LED during calibration
    sum += analogRead(LDR_PIN);
    delay(100);
  }
  lightThreshold = (sum / 20) * 0.8; // Set threshold at 80% of current brightness
  digitalWrite(STATUS_LED, LOW);     // Turn off LED when calibration is done
  filteredPot = analogRead(POT_PIN); // Seed the filter with an initial reading
}

void loop() {
  int rawPot = analogRead(POT_PIN);     // Get the raw 0-4095 value from the pot
  int lightLevel = analogRead(LDR_PIN); // Get current brightness from the LDR
  int btnState = digitalRead(BUTTON_PIN); // Check if button is pressed (LOW) or not (HIGH)
  static int lastBtnState = HIGH;      // Track previous button state for edge detection

  // Smooth out potentiometer noise using an Exponential Moving Average (EMA)
  filteredPot = (rawPot * emaAlpha) + (filteredPot * (1.0 - emaAlpha));
  // Map the smooth 0-4095 value to select index 0, 1, 2, or 3 from timerOptions
  int selection = map((int)filteredPot, 0, 4095, 0, 3);
  selection = constrain(selection, 0, 3); // Ensure value never goes out of array bounds

  // --- FIXED AUTO-PAUSE/RESUME LOGIC ---
  if (isRunning) {
    // TRIGGER AUTO-PAUSE: If not paused and light falls below threshold
    if (!isPaused && lightLevel < lightThreshold) {
      isPaused = true;
      autoPaused = true;
      pausedAt = millis(); // Store current time to calculate pause duration later
      tone(BUZZER_PIN, 300, 200); // Play a low "pause" beep
    } 
    // TRIGGER AUTO-RESUME: Light must be 10% brighter than threshold to prevent flickering
    else if (isPaused && autoPaused && lightLevel >= (lightThreshold * 1.1)) {
      isPaused = false;
      autoPaused = false;
      totalPausedTime += (millis() - pausedAt); // Calculate how long we were paused
      
      noTone(BUZZER_PIN); // Stop any hanging buzzer tones
      delay(10);          // Short delay for buzzer stability
      tone(BUZZER_PIN, 900, 100); // Play a high "resume" beep
    }
  }

  // --- BUTTON LOGIC ---
  if (btnState == LOW && lastBtnState == HIGH) buttonPressedTime = millis(); // Start timing press

  if (btnState == HIGH && lastBtnState == LOW) { // Button just released
    unsigned long holdDuration = millis() - buttonPressedTime; // How long was it held?
    
    if (holdDuration > LONG_PRESS_MS) { // LONG PRESS: Reset/End session
      if (isRunning) { 
        noTone(BUZZER_PIN); // Force stop any alert tones
        tone(BUZZER_PIN, 200, 500); // Low reset sound
        endSession(); 
      }
    } else { // SHORT PRESS: Start or Manual Pause
      if (!isRunning && lightLevel > lightThreshold) { // Start timer if it's bright enough
        isRunning = true; isPaused = false; autoPaused = false;
        startTime = millis(); totalPausedTime = 0;
        intervalMs = (unsigned long)timerOptions[selection] * 1000; // Set timer length
        tone(BUZZER_PIN, 1000, 100); // Start beep
      } else if (isRunning) { // Toggle manual pause while running
        isPaused = !isPaused;
        autoPaused = false; // Manual toggle clears the "auto-pause" status
        noTone(BUZZER_PIN); // Clear previous tone
        if (isPaused) { 
            pausedAt = millis(); 
            tone(BUZZER_PIN, 400, 100); // Pause beep
        } else { 
            totalPausedTime += (millis() - pausedAt); 
            tone(BUZZER_PIN, 800, 100); // Resume beep
        }
      }
    }
  }
  lastBtnState = btnState; // Update last button state for next loop

  // --- EXECUTION ---
  if (isRunning) {
    if (isPaused) {
      // Blink LED fast for auto-pause (200ms), slow for manual pause (1000ms)
      int blinkRate = autoPaused ? 200 : 1000;
      digitalWrite(STATUS_LED, (millis() % (blinkRate * 2) < blinkRate) ? HIGH : LOW);
    } else {
      // Calculate true elapsed time: current time minus start time minus any time spent paused
      unsigned long elapsed = millis() - startTime - totalPausedTime;
      // Map progress to LED brightness (fades out as time passes)
      analogWrite(PWM_LED, map(elapsed, 0, intervalMs, 255, 0));
      digitalWrite(STATUS_LED, HIGH); // Steady light when actively running
      // Map progress to servo position (0 to 180 degrees)
      gaugeServo.write(map(elapsed, 0, intervalMs, 0, 180));
      
      if (elapsed >= intervalMs) endSession(); // Timer reached zero
    }
  } else {
    // Idle Mode: PWM LED brightness shows currently selected timer option
    analogWrite(PWM_LED, map((int)filteredPot, 0, 4095, 10, 255));
    gaugeServo.write(0); // Reset servo gauge to zero
    // Safety check to ensure the buzzer stays off in idle mode
    static unsigned long lastSilence = 0;
    if (millis() - lastSilence > 500) { noTone(BUZZER_PIN); lastSilence = millis(); }
  }
}

// Resets all state variables and plays a finishing melody
void endSession() {
  isRunning = false; isPaused = false; autoPaused = false;
  digitalWrite(STATUS_LED, LOW);
  analogWrite(PWM_LED, 0);
  // Play a 4-note rising "success" melody
  for (int i = 0; i < 4; i++) { 
    tone(BUZZER_PIN, 600 + (i * 150), 100); 
    delay(150); 
  }
  noTone(BUZZER_PIN); // Final silence to prevent sticking
  gaugeServo.write(0); // Move pointer back to start
}
