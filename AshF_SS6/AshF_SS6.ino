// ESP32-S3 Pin definitions
const int pot1Pin = 1;  // ADC1_CH0
const int pot2Pin = 2;  // ADC1_CH1
const int led1Pin = 10; // PWM Output
const int led2Pin = 11; // PWM Output

void setup() {
  // ESP32-S3 often uses 9600 for stable serial over USB
  Serial.begin(9600); 
  
  pinMode(led1Pin, OUTPUT);
  pinMode(led2Pin, OUTPUT);
  
  establishContact();
}

void loop() {
  if (Serial.available() > 0) {
    // Read two bytes from P5 for LED brightness (0-255)
    int led1Val = Serial.read();
    int led2Val = Serial.read();
    
    // Actuators: ESP32-S3 supports analogWrite() in newer Arduino cores
    analogWrite(led1Pin, led1Val);
    analogWrite(led2Pin, led2Val);

    // Sensors: ESP32-S3 ADC is 12-bit (0-4095)
    int val1 = analogRead(pot1Pin);
    int val2 = analogRead(pot2Pin);
    
    // Handshake: Send comma-separated values to P5
    Serial.print(val1);
    Serial.print(",");
    Serial.println(val2);
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("0,0");
    delay(300);
  }
}

