// --- Global Variables (Hardware Setup) ---
const int potPin = 4;   // Pin for Potentiometer (ADC1 Channel 3)
const int lightPin = 5; // Pin for Photocell (ADC1 Channel 4)
const int btn1 = 11;    // Pin for Button 1 (Digital)
const int btn2 = 12;    // Pin for Button 2 (Digital)

void setup() {
  // Initialize Serial at exactly 9200 baud for UART communication
  Serial.begin(9200); 
  
  // Set pins 11 and 12 to input mode using the internal pull-up resistors
  pinMode(btn1, INPUT_PULLUP); 
  pinMode(btn2, INPUT_PULLUP);
}

void loop() {
  // Read raw 12-bit analog values (0-4095) from ESP32-S3 pins
  int val0 = analogRead(potPin);   
  int val1 = analogRead(lightPin); 
  
  // Read digital states (0 = Pressed/GND, 1 = Open/3.3V)
  int val2 = digitalRead(btn1);   
  int val3 = digitalRead(btn2);   

  // Transmit values via UART as a single comma-separated string
  Serial.print(val0);      // First element: Potentiometer
  Serial.print(",");       // Data separator
  Serial.print(val1);      // Second element: Photocell
  Serial.print(",");       // Data separator
  Serial.print(val2);      // Third element: Button 1
  Serial.print(",");       // Data separator
  Serial.println(val3);    // Fourth element: Button 2 + Newline character
  
  // Delay 50ms to prevent flooding the serial buffer at 9200 baud
  delay(50); 
}
