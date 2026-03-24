let serial;          // Serial port object
let sensorData = 0;  // Variable to store incoming data

function setup() {
  createCanvas(800, 500);
  
  // Serial Setup
  serial = new p5.SerialPort();
  serial.open("COM8");
  serial.on('data', gotData);
}

function gotData() {
  let currentString = serial.readLine(); 
  trim(currentString);                    
  if (!currentString) return;             
  sensorData = Number(currentString);     
}

function draw() {
  // 1. Use map() to control background color (Night to Day)
  let bgColor = map(sensorData, 0, 1023, 20, 200);
  background(bgColor, 100, 250);

  // 2. Map sensor to Sun position and size
  let sunY = map(sensorData, 0, 1023, height, 100);
  let sunSize = map(sensorData, 0, 1023, 40, 120);
  let sunCol = map(sensorData, 0, 1023, 100, 255);

  // --- Graphics Primitives ---
  
  // Primitive 1: Sun (Ellipse)
  noStroke();
  fill(255, sunCol, 0);
  ellipse(width / 2, sunY, sunSize);

  // Primitive 2: Mountain (Triangle)
  fill(50, 100, 50);
  triangle(100, height, 400, 150, 700, height);

  // Primitive 3: Ground (Rect)
  fill(30, 150, 30);
  rect(0, height - 50, width, 50);
  
  // Extra: Cloud movement based on sensor
  let cloudX = map(sensorData, 0, 1023, -100, width + 100);
  fill(255, 255, 255, 200);
  ellipse(cloudX, 80, 80, 40);
}
