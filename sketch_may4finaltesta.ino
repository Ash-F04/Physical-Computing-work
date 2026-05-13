// Shifted completely away from the Octal memory and SPI channel blocks
const int touchPins[] = {1, 2, 3, 8, 9, 10, 14}; 

void setup() {
  Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < 7; i++) {
    int val = touchRead(touchPins[i]);
    
    // Safety check stays active to protect the p5.js text parsing layout
    if (val >= 4194303) val = 0; 

    Serial.print(val);
    if (i < 6) Serial.print(",");
  }
  
  Serial.println(); 
  delay(100);
}

