/* arduino_serial_handshaking.ino */
const int led1 = 9;   
const int led2 = 10;  
const int btn1 = 2;   
const int btn2 = 4;   

void setup() {
  Serial.begin(9600); // Must match P5 options
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(btn1, INPUT_PULLUP); // Button 1 on Pin 2
  pinMode(btn2, INPUT_PULLUP); // Button 2 on Pin 4
  establishContact();
}

void loop() {
  if (Serial.available() > 0) {
    char inByte = Serial.read(); // Read the 'A' or 'B' from P5
    
    if (inByte == 'A') { // Start/Resume command
      // Read sensors and send as a single CSV line
      int s1 = !digitalRead(btn1); // Invert so 1 = Pressed
      int s2 = !digitalRead(btn2);
      
      // Format: btn1,btn2,pot1,pot2 followed by a newline
      Serial.print(s1);
      Serial.print(",");
      Serial.print(s2);
      Serial.print(",");
      Serial.print(0); // Dummy Pot 1
      Serial.print(",");
      Serial.println(0); // Dummy Pot 2 + Newline
      
      // Update Actuators based on current state
      digitalWrite(led1, HIGH);
    } else if (inByte == 'B') {
      digitalWrite(led1, LOW); // Turn off on Pause
    }
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("0,0,0,0"); // Send dummy CSV to trigger P5 firstContact
    delay(300);
  }
}