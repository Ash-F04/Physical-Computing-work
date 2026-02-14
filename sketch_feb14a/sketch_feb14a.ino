// --- Variable Declarations ---
int ledPins[] = {2, 3, 4, 5}; // Array to hold pin numbers
int patternCounter = 0;       // Keeps track of which pattern to show
int waitTime = 200;           // Base delay time in milliseconds

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
}

void loop() {
  // Pattern 1: The "Chaser" (One by one)
  if (patternCounter == 0) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(waitTime);
      digitalWrite(ledPins[i], LOW);
    }
  } 
  
  // Pattern 2: The "Fill Up" (Stay on)
  else if (patternCounter == 1) {
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(waitTime);
    }
    allOff();
  }

  // Pattern 3: Outside-In (Symmetry)
  else if (patternCounter == 2) {
    digitalWrite(ledPins[0], HIGH); digitalWrite(ledPins[3], HIGH);
    delay(waitTime * 2);
    digitalWrite(ledPins[0], LOW);  digitalWrite(ledPins[3], LOW);
    digitalWrite(ledPins[1], HIGH); digitalWrite(ledPins[2], HIGH);
    delay(waitTime * 2);
    allOff();
  }

  // Pattern 4: Alternating Pairs
  else if (patternCounter == 3) {
    for(int j=0; j<3; j++) {
      digitalWrite(ledPins[0], HIGH); digitalWrite(ledPins[2], HIGH);
      digitalWrite(ledPins[1], LOW);  digitalWrite(ledPins[3], LOW);
      delay(waitTime);
      digitalWrite(ledPins[0], LOW);  digitalWrite(ledPins[2], LOW);
      digitalWrite(ledPins[1], HIGH); digitalWrite(ledPins[3], HIGH);
      delay(waitTime);
    }
    allOff();
  }

  // Pattern 5: Rapid Strobe (All together)
  else if (patternCounter == 4) {
    for(int j=0; j<5; j++) {
      allOn();
      delay(50);
      allOff();
      delay(50);
    }
  }

  // Pattern 6: The "Ping-Pong" (Back and forth)
  else if (patternCounter >= 5) { // Logical comparator used here
    for (int i = 0; i < 4; i++) {
      digitalWrite(ledPins[i], HIGH);
      delay(100);
      digitalWrite(ledPins[i], LOW);
    }
    for (int i = 2; i >= 0; i--) {
      digitalWrite(ledPins[i], HIGH);
      delay(100);
      digitalWrite(ledPins[i], LOW);
    }
  }

  // Increment pattern and reset if we exceed the count
  patternCounter++;
  
  if (patternCounter > 5) {
    patternCounter = 0; // Reset loop
    delay(1000);        // Pause before restarting the sequence
  }
}

// Helper functions to keep code clean
void allOff() {
  for(int i=0; i<4; i++) digitalWrite(ledPins[i], LOW);
}

void allOn() {
  for(int i=0; i<4; i++) digitalWrite(ledPins[i], HIGH);
}