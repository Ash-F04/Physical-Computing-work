#include <ESP32Servo.h> // Library to handle PWM for Servos on ESP32 hardware

// --- PIN DEFINITIONS ---
const int POT_PIN = 1;       // Pin for the dial (Potentiometer)
const int LDR_PIN = 2;       // Pin for the light sensor (Photoresistor)
const int BUTTON_PIN = 4;    // Pin for the start button
const int STATUS_LED = 5;    // Indicator for session status and warnings
const int PWM_LED = 6;       // LED that dims as time runs out
const int BUZZER_PIN = 7;    // Output for end-of-session alert
const int SERVO_PIN = 18;    // Output for the physical gauge needle

// --- LOGIC VARIABLES ---
Servo gaugeServo;            // Create a servo object to control the motor
int timerOptions[] = {10, 30, 60, 120}; // Array of durations in seconds
bool isRunning = false;      // Flag to track if a session is active
unsigned long startTime = 0; // Stores the timestamp when a session starts
unsigned long intervalMs = 0;// Stores the total duration in milliseconds
int lightThreshold = 0;      // Variable to store calibrated "working light" level

// --- SMOOTHING (Fixes Big Jumps) ---
float filteredPot = 0;       // Buffer for the "cleaned up" dial value
const float emaAlpha = 0.08; // Smoothing factor (lower = smoother/slower)

void setup() {
  Serial.begin(115200);      // Start serial communication for debugging
  
  // FIX: Force S3 to 12-bit (0-4095) so our math stays consistent
  analogReadResolution(12); 

  ESP32PWM::allocateTimer(0); // Reserve a hardware timer for the servo
  gaugeServo.setPeriodHertz(50); // Set standard frequency for SG90 servos
  gaugeServo.attach(SERVO_PIN, 500, 2400); // Connect servo to pin with pulse limits
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal resistor to keep pin HIGH
  pinMode(STATUS_LED, OUTPUT);       // Set status LED as an output
  pinMode(PWM_LED, OUTPUT);          // Set dimming LED as an output
  pinMode(BUZZER_PIN, OUTPUT);       // Set buzzer as an output

  // --- CALIBRATION ---
  Serial.println("CALIBRATING LIGHT...");
  long sum = 0;                      // Temporary sum for averaging
  for(int i = 0; i < 20; i++) {      // Loop 20 times to get an average
    digitalWrite(STATUS_LED, !digitalRead(STATUS_LED)); // Flash LED while calibrating
    sum += analogRead(LDR_PIN);      // Add current light reading to sum
    delay(100);                      // Wait 0.1 seconds between samples
  }
  lightThreshold = (sum / 20) * 0.8; // Set threshold at 80% of current brightness
  digitalWrite(STATUS_LED, LOW);     // Turn off calibration flash
  
  filteredPot = analogRead(POT_PIN); // Initialize filter with current dial position
  Serial.printf("Target Light: %d\n", lightThreshold); // Print final target to monitor
}

void loop() {
  int rawPot = analogRead(POT_PIN);       // Read the raw noisy dial value
  int lightLevel = analogRead(LDR_PIN);     // Read the current room brightness
  int btnState = digitalRead(BUTTON_PIN);   // Check if the button is pressed (LOW)

  // SMOOTHING: Blends 8% new data with 92% old data to stop "jitter"
  filteredPot = (rawPot * emaAlpha) + (filteredPot * (1.0 - emaAlpha));
  
  // Convert 0-4095 dial range into 0-3 index for our timer array
  int selection = map((int)filteredPot, 0, 4095, 0, 3);
  selection = constrain(selection, 0, 3); // Ensure index stays within array bounds

  // DEBUG PRINTING: Send values to computer every 0.5 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.printf("POT: %d | LIGHT: %d | THRESHOLD: %d\n", (int)filteredPot, lightLevel, lightThreshold);
    lastPrint = millis();
  }

  // START TRIGGER: Only if button is pressed, light is bright, and no session is running
  if (btnState == LOW && lightLevel > lightThreshold && !isRunning) {
    isRunning = true;                     // Mark session as active
    startTime = millis();                 // Record the start time
    intervalMs = (unsigned long)timerOptions[selection] * 1000; // Calc duration in ms
    digitalWrite(STATUS_LED, HIGH);       // Turn on status light
  }

  if (isRunning) {
    unsigned long elapsed = millis() - startTime; // Calculate time passed

    // COUNTDOWN DIMMING: Map time passed (0 to end) to brightness (255 to 0)
    int brightness = map(elapsed, 0, intervalMs, 255, 0);
    analogWrite(PWM_LED, constrain(brightness, 0, 255)); // Apply dimming

    // THE BLINK FIX: Active monitoring during work
    if (lightLevel < lightThreshold) {    // If room gets darker than calibrated...
      bool blink = (millis() % 500) < 250; // Calculate blink toggle (0.25s ON/OFF)
      digitalWrite(STATUS_LED, blink ? HIGH : LOW); // Flash the warning
    } else {
      digitalWrite(STATUS_LED, HIGH);     // Keep solid if light is okay
    }

    // SERVO PROGRESS: Move needle from 0 to 180 degrees based on time
    gaugeServo.write(map(elapsed, 0, intervalMs, 0, 180));

    if (elapsed >= intervalMs) endSession(); // Finish if time is up
    
  } else {
    // IDLE MODE: Show dial selection by changing LED brightness
    int previewBrightness = map((int)filteredPot, 0, 4095, 10, 255);
    analogWrite(PWM_LED, previewBrightness);
    gaugeServo.write(0); // Hold needle at zero while waiting
  }
}

void endSession() {
  isRunning = false;                // Mark session as finished
  digitalWrite(STATUS_LED, LOW);    // Turn off work light
  analogWrite(PWM_LED, 0);          // Turn off dimming LED
  
  for (int i = 0; i < 4; i++) {     // Play 4 beeps
    tone(BUZZER_PIN, 600 + (i * 150), 100); // Ascending pitch
    delay(150);                     // Pause between beeps
  }
  noTone(BUZZER_PIN);               // Ensure buzzer is silent
  gaugeServo.write(0);              // Reset physical needle
}