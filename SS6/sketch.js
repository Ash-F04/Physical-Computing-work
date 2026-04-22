let serial;
let sensors = [0, 0];
let mode = 'kaleidoscope'; 

function setup() {
  createCanvas(800, 600); // Meets display window requirements
  serial = new p5.SerialPort();
  serial.open("COM8"); // UPDATE TO YOUR S3 PORT
  serial.on('data', serialEvent);
}

function draw() {
  background(20, 20, 40, 40);
  
  // Mapping the 12-bit ESP32-S3 sensor data
  let size = map(sensors[0], 0, 4095, 20, 350);
  let speed = map(sensors[1], 0, 4095, 0.01, 0.2);

  push();
  translate(width/2, height/2);
  
  if (mode === 'kaleidoscope') {
    rotate(frameCount * speed);
    stroke(0, 255, 255);
    line(-size, 0, size, 0); // Primitive 1: Line
    
    noFill();
    ellipse(0, 0, size, size/2); // Primitive 2: Ellipse
    
    fill(255, 100, 0, 150);
    rect(-15, -15, 30, 30); // Primitive 3: Rect
  } else {
    // Mode 2: Dynamic Pulse
    for(let i=0; i<8; i++) {
      let r = size + sin(frameCount * speed + i) * 60;
      stroke(255, 255, i*30);
      ellipse(0, 0, r, r);
    }
  }
  pop();
}

function serialEvent() {
  let inString = serial.readLine();
  if (inString.length > 0) {
    sensors = split(inString, ',');
    
    // Actuator Feedback: Control LEDs via P5 mouse interaction
    let led1 = floor(map(mouseX, 0, width, 0, 255));
    let led2 = floor(map(mouseY, 0, height, 0, 255));
    
    serial.write(led1);
    serial.write(led2);
  }
}

function keyPressed() {
  if (key === ' ') mode = (mode === 'kaleidoscope') ? 'pulse' : 'kaleidoscope';
}
