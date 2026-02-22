// --- 1. Variable Declarations (Requirement: at least 3) ---
const int leds[] = {4, 5, 6, 7};    // Pins for 4 LEDs
const int btn = 1;                  // Pin for Button
int mode = 0;                       // Current pattern (0-5)
bool lastState = LOW;               // For button edge detection
unsigned long prevTime = 0;         // For non-blocking animations
int blinkState = LOW;               // Toggle for blinking patterns

void setup() {
  for (int i = 0; i < 4; i++) pinMode(leds[i], OUTPUT);
  pinMode(btn, INPUT); // Assumes external 10k pull-down resistor
}

void loop() {
  // --- 2. digitalRead() & Logic (Requirement) ---
  bool reading = digitalRead(btn);
  
  // Detect "Falling Edge" (Button Released)
  // 3. Conditional with Logical Comparator (Requirement)
  if (reading == LOW && lastState == HIGH) {
    mode++;
    if (mode > 5) mode = 0; // Cycle back to first pattern
    clear();                // Reset LEDs when switching
  }
  lastState = reading;

  // --- 4. Run Patterns (Requirement: 6 Unique Patterns) ---
  unsigned long now = millis();
  
  if (mode == 0) { // PATTERN 0: Fast Strobe All
    if (now - prevTime >= 100) { 
      prevTime = now;
      blinkState = !blinkState;
      setAll(blinkState);
    }
  } 
  else if (mode == 1) { // PATTERN 1: The "Chaser" (One by one)
    if (now - prevTime >= 150) {
      prevTime = now;
      static int chase = 0;
      clear();
      digitalWrite(leds[chase], HIGH);
      chase = (chase + 1) % 4;
    }
  }
  else if (mode == 2) { // PATTERN 2: Alternating Pairs
    if (now - prevTime >= 300) {
      prevTime = now;
      blinkState = !blinkState;
      digitalWrite(leds[0], blinkState); digitalWrite(leds[2], blinkState);
      digitalWrite(leds[1], !blinkState); digitalWrite(leds[3], !blinkState);
    }
  }
  else if (mode == 3) { // PATTERN 3: In-Out Bounce
    if (now - prevTime >= 200) {
      prevTime = now;
      blinkState = !blinkState;
      digitalWrite(leds[0], blinkState); digitalWrite(leds[3], blinkState);
      digitalWrite(leds[1], !blinkState); digitalWrite(leds[2], !blinkState);
    }
  }
  else if (mode == 4) { // PATTERN 4: Slow Heartbeat
    if (now - prevTime >= 600) {
      prevTime = now;
      blinkState = !blinkState;
      setAll(blinkState);
    }
  }
  else if (mode == 5) { // PATTERN 5: Random Sparkle
    if (now - prevTime >= 50) {
      prevTime = now;
      digitalWrite(leds[random(0,4)], random(0,2));
    }
  }
}

// Helper Functions (Requirement: digitalWrite)
void setAll(int state) {
  for (int i = 0; i < 4; i++) digitalWrite(leds[i], state);
}

void clear() {
  setAll(LOW);
}
