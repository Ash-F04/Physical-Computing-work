let serial;
let latestData = "waiting for data";
let sensorValues = [0, 0, 0, 0, 0, 0, 0];
let portName = 'COM8'; // Change this to your serial port
let options = { baudRate: 9600 };
let images = [];

// --- YOUR SPECIFIC THRESHOLDS ---
let thresholds = [50000, 40000, 35000, 40000, 40000, 60000, 38000];
let names = ["Asia", "Africa", "N. America", "S. America", "Antarctica", "Europe", "Australia"];

function setup() {
  createCanvas(windowWidth, windowHeight);

  // 1. Create 7 virtual canvases for the continents
  for (let i = 0; i < names.length; i++) {
    images[i] = createGraphics(150, 150);
    drawContinentImage(images[i], i);
  }

  // 2. Serial Setup with 9600 Baud
    serial = new p5.SerialPort();             // make a new instance of the serialport library
  serial.on('list', printList);             // set a callback function for the serialport list event
  serial.on('connected', serverConnected);  // set callback for connecting to the server
  serial.on('open', portOpen);              // set callback for the port opening
  serial.on('data', serialEvent);           // set callback for when new data received
  serial.on('error', serialError);          // set callback for errors
  serial.on('close', portClose);            // set callback for closing the port
  serial.list();                            // list the serial ports
  serial.open(portName, options);           // open a serial port
  
  imageMode(CENTER);
  textAlign(CENTER);
}

function serialEvent() //gets called when new data arrives
{
  let inString = serial.readStringUntil('\n'); // read until newline character
  if (inString.length > 0) //if there's data in the string
  {
    sensors = split(inString, ','); // split the string at commas and store in array
    
    for (let i = 0; i < sensors.length; i++) 
    {
      // Number() function converts string to number
      sensors[i] = Number(sensors[i]); // convert every element in array to numbers

      // You can also use parseInt() function, which takes a second argument for the base (radix).
      // A base of 10 is for decimal numbers, base of 16 is for hexadecimal, base of 2 is for binary.
      // sensors[i] = parseInt(sensors[i], 10); // converts every element in array to decimal number
    }
    //print(sensors);
  } 
}
function draw() {
  background(20);
  let spacing = width / 8;

  for (let i = 0; i < sensorValues.length; i++) {
    let x = spacing * (i + 1);
    let y = height / 2;

    // --- CHECK THRESHOLD ---
    if (sensorValues[i] > thresholds[i]) {
      // Draw the generative image
      image(images[i], x, y, 150, 150);
      
      fill(255);
      textSize(16);
      text(names[i], x, y + 100);
    } else {
      // Idle state
      fill(60);
      ellipse(x, y, 15, 15);
    }

    // Debugging Data
    fill(100);
    textSize(10);
    text(sensorValues[i], x, y + 130);
  }
}

// Generative Image Logic
function drawContinentImage(g, index) {
  g.clear();
  g.noStroke();
  switch(index) {
    case 0: g.fill(255, 0, 0); g.ellipse(75, 75, 120, 120); break; // Asia
    case 1: g.fill(0, 255, 0); g.triangle(75, 15, 15, 135, 135, 135); break; // Africa
    case 2: g.fill(0, 0, 255); g.rect(25, 25, 100, 100); break; // N. America
    case 3: g.fill(255, 255, 0); g.ellipse(75, 75, 100, 140); break; // S. America
    case 4: g.fill(255); g.stroke(0, 0, 255); g.strokeWeight(5); g.ellipse(75, 75, 120, 120); break; // Antarctica
    case 5: g.fill(128, 0, 128); drawStar(g, 75, 75, 30, 60, 5); break; // Europe
    case 6: g.fill(255, 165, 0); g.rect(60, 25, 30, 100); g.rect(25, 60, 100, 30); break; // Australia
  }
}

function drawStar(g, x, y, radius1, radius2, npoints) {
  let angle = TWO_PI / npoints;
  let halfAngle = angle / 2.0;
  g.beginShape();
  for (let a = 0; a < TWO_PI; a += angle) {
    let sx = x + cos(a) * radius2;
    let sy = y + sin(a) * radius2;
    g.vertex(sx, sy);
    sx = x + cos(a + halfAngle) * radius1;
    sy = y + sin(a + halfAngle) * radius1;
    g.vertex(sx, sy);
  }
  g.endShape(CLOSE);
}
