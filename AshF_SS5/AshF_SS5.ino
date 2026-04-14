/* arduino_serial_handshaking.ino */
const int led1 = 9;           // Pin for the first LED
const int led2 = 10;          // Pin for the second LED
const int btn1 = 2;           // Pin for Button 1 (uses internal pull-up)
const int btn2 = 4;           // Pin for Button 2 (uses internal pull-up)

void setup() {
  Serial.begin(9600);         // Initialize serial communication at 9600 bits per second
  pinMode(led1, OUTPUT);      // Set LED 1 pin as an output
  pinMode(led2, OUTPUT);      // Set LED 2 pin as an output
  pinMode(btn1, INPUT_PULLUP); // Set Pin 2 as input; high when open, low when pressed
  pinMode(btn2, INPUT_PULLUP); // Set Pin 4 as input; high when open, low when pressed
  establishContact();         // Run the handshake function to wake up P5.js
}

void loop() {
  // Check if P5.js has sent a character to the Arduino
  if (Serial.available() > 0) {
    char inByte = Serial.read(); // Read the character ('A' for Start/Resume, 'B' for Pause)

    if (inByte == 'A') {         // P5 is requesting data
      // Read sensor states; '!' inverts the signal so 1 means "pressed"
      int s1 = !digitalRead(btn1); 
      int s2 = !digitalRead(btn2); 

      // Send the data as a comma-separated string back to P5
      Serial.print(s1);          // Send Button 1 state
      Serial.print(",");         // Add separator
      Serial.print(s2);          // Send Button 2 state
      Serial.print(",");         // Add separator
      Serial.print(0);           // Send placeholder for Potentiometer 1
      Serial.print(",");         // Add separator
      Serial.println(0);         // Send placeholder for Pot 2 and a newline (\n)

      // Visual feedback: Turn LED on while data is flowing
      digitalWrite(led1, HIGH); 
    } 
    else if (inByte == 'B') {    // P5 has requested a pause
      digitalWrite(led1, LOW);   // Turn off LED to indicate stopped state
    }
  }
}

void establishContact() {
  // Stay in this loop until P5 sends a character back to the Arduino
  while (Serial.available() <= 0) {
    // Send a "heartbeat" string to let P5 know the Arduino is ready
    Serial.println("0,0,0,0"); 
    delay(300);                // Wait 300ms between attempts to avoid flooding
  }
}