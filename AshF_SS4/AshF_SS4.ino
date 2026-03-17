cpp
void setup() {
  Serial.begin(9600); // Match this speed in p5.js
}

void loop() {
  int sensorValue = analogRead(A0); // Read sensor (0-1023)
  Serial.println(sensorValue);      // Send value as a string with a newline
  delay(50);                        // Short delay for stability
}