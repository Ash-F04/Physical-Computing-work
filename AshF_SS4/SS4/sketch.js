/*
  ESP32-S3 SERIAL READ (9200 Baud)
  Hardware: Pot (GPIO 4), Photocell (GPIO 5), Button 1 (GPIO 11), Button 2 (GPIO 12)
*/

let serial; // declare variable for an instance of the serialport library
// UPDATE: Change this string to match the port in your p5.serialcontrol app
let portName = 'COM8';  
let options = { baudRate: 9200 }; // baud rate set to 9200 to match Arduino

// use of declared variables
let sensors = [0, 0, 1, 1]; // array for: [pot, photo, btn1, btn2]
let posX, bgBrightness, shapeSize; 

function setup() {
  // P5 SerialPort Setup
  serial = new p5.SerialPort();             
  serial.on('data', serialEvent);           
  serial.on('error', serialError);          
  serial.open(portName, options);           

  // display window width/height (max 1080x720, min 400x400)
  createCanvas(900, 600); 
}

function draw() {
  // use of data type conversion (Number() used in serialEvent)
  let valPot = sensors[0];    // Potentiometer (0-4095)
  let valPhoto = sensors[1];  // Photocell (0-4095)
  let valBtn1 = sensors[2];   // Button 1 (0 or 1)
  let valBtn2 = sensors[3];   // Button 2 (0 or 1)

  // use of map() function
  posX = map(valPot, 0, 4095, 0, width);
  bgBrightness = map(valPhoto, 0, 1000, 0, 255);
  shapeSize = map(valPhoto, 0, 1000, 20, 300);

  // use of background()
  background(bgBrightness); 

  // use of fill() and graphics primitives
  
  // Primitive 1: circle (Follows Potentiometer)
  fill(0, 200, 255); 
  noStroke();
  circle(posX, height * 0.2, 60);

  // Primitive 2: rect (Size follows Light, Color follows Button 1)
  if (valBtn1 === 0) fill(255, 0, 0); // Red if Button 1 pressed
  else fill(255);
  rectMode(CENTER);
  rect(posX, height / 2, shapeSize, 100);

  // Primitive 3: line (Thickness follows Button 2)
  if (valBtn2 === 0) strokeWeight(15); // Thick if Button 2 pressed
  else strokeWeight(2);
  stroke(255 - bgBrightness); // Inverted color for contrast
  line(0, height * 0.8, width, height * 0.8);
  
  // Text display for debugging
  fill(255);
  noStroke();
  textSize(24);
  text("Sensors: " + sensors, 20, height - 20);
}

// use of sensor data (transmitted via UART)
function serialEvent() {
  let inString = serial.readStringUntil('\n'); 
  if (inString && inString.length > 0) {
    let data = split(trim(inString), ','); 
    if (data.length >= 4) {
      for (let i = 0; i < data.length; i++) {
        // use of data type conversion
        sensors[i] = Number(data[i]); 
      }
    }
  }
}

function serialError(err) {
  print('ERROR: ' + err);
}
