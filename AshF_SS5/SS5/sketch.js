/* Fixed P5.js Call and Response */
let serial; // Variable to hold the serial port object
let portName = 'COM8'; // The specific port your Arduino is plugged into
let options = { baudRate: 9600 }; // Connection speed (must match Serial.begin in Arduino)
let rxFlag = false; // Toggle to start/stop data flow
let firstContact = false; // Tracks if the "handshake" with Arduino has happened
let sensors = [0, 0, 0, 0]; // Array to store incoming sensor values

function setup() {
  createCanvas(500, 500); // Create the visual stage
  textAlign(CENTER, CENTER); // Center text alignment for UI
  textSize(24); // Set readable font size
  serial = new p5.SerialPort(); // Initialize the serial library
  serial.on('data', serialEvent); // Link incoming data to the serialEvent function
  serial.on('error', (err) => console.log('Error:', err)); // Log errors to the console
  serial.open(portName, options); // Open the connection to the Arduino
}

function draw() {
  background(0); // Set background to black
  
  if (!firstContact) { 
    // UI shown before the first message is received from Arduino
    background(0, 0, 127); // Dark blue background for "waiting" state
    fill(255); // White text
    text("Waiting for First Contact", width/2, height/4);
    circle(width/2, height/2, 140); // Draw the "Start" button area
    fill(0); // Black text for button label
    text("START RX", width/2, height/2);
  } else { 
    // UI shown once communication is established
    // Button 1 Indicator: turns red if sensors[0] is 1, otherwise stays white
    fill(sensors[0] == 1 ? 'red' : 255); 
    rect(width/2 - 100, 100, 50, 50);
    
    // Button 2 Indicator: turns blue if sensors[1] is 1, otherwise stays white
    fill(sensors[1] == 1 ? 'blue' : 255); 
    rect(width/2 + 50, 100, 50, 50);

    // Main central button to control data flow
    fill(200); 
    circle(width/2, height/2, 140);
    fill(0);
    // Change button text dynamically based on the rxFlag state
    text(rxFlag ? "PAUSE RX" : "RESUME", width/2, height/2);
  }
}

function mousePressed() {
  // Check if the user clicked inside the center circle (70px radius)
  if (dist(mouseX, mouseY, width/2, height/2) < 70) {
    rxFlag = !rxFlag; // Flip the data flow switch (on to off, or off to on)
    serial.write(rxFlag ? 'A' : 'B'); // Send 'A' to Arduino to start, 'B' to stop
  }
}

function serialEvent() {
  // Read incoming data until it hits a newline character (\n)
  let inString = serial.readStringUntil('\n'); 
  
  if (inString && inString.length > 0) {
    if (!firstContact) {
      firstContact = true; // Success! We received the first message
      console.log("Contact Made!");
    }
    
    if (rxFlag) {
      // Remove whitespace and split the comma-separated string into an array
      let data = split(trim(inString), ','); 
      
      if (data.length >= 2) {
        sensors = data; // Update our sensor array with the new values
        // The Handshake: Send 'A' back to Arduino to ask for the next packet
        serial.write('A'); 
      }
    }
  }
}
