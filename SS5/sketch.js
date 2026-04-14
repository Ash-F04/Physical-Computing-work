/* Fixed P5.js Call and Response */
let serial; 
let portName = 'COM8'; // Ensure this matches p5.serialcontrol
let options = { baudRate: 9600 }; 
let rxFlag = false; 
let firstContact = false; 
let sensors = [0, 0, 0, 0]; 

function setup() {
  createCanvas(500, 500);
  textAlign(CENTER, CENTER);
  textSize(24);
  
  serial = new p5.SerialPort();
  serial.on('data', serialEvent);
  serial.on('error', (err) => console.log('Error:', err));
  serial.open(portName, options);
}

function draw() {
  background(0);
  if (!firstContact) {
    background(0, 0, 127);
    fill(255);
    text("Waiting for First Contact", width/2, height/4);
    circle(width/2, height/2, 140);
    fill(0);
    text("START RX", width/2, height/2);
  } else {
    // Button Indicators based on sensor data
    fill(sensors[0] == 1 ? 'red' : 255); // Button 1
    rect(width/2 - 100, 100, 50, 50);
    fill(sensors[1] == 1 ? 'blue' : 255); // Button 2
    rect(width/2 + 50, 100, 50, 50);

    // Main Control Button
    fill(200);
    circle(width/2, height/2, 140);
    fill(0);
    text(rxFlag ? "PAUSE RX" : "RESUME", width/2, height/2);
  }
}

function mousePressed() {
  if (dist(mouseX, mouseY, width/2, height/2) < 70) {
    rxFlag = !rxFlag;
    serial.write(rxFlag ? 'A' : 'B'); // Call out to Arduino
  }
}

function serialEvent() {
  // Read until newline - this is critical for the CSV format
  let inString = serial.readStringUntil('\n'); 
  
  if (inString && inString.length > 0) {
    if (!firstContact) {
      firstContact = true;
      console.log("Contact Made!");
    }

    if (rxFlag) {
      // Clean and split the string
      let data = split(trim(inString), ',');
      if (data.length >= 2) {
        sensors = data;
        // The Handshake "Repeat Call"
        serial.write('A'); 
      }
    }
  }
}