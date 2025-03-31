// Extend your lab to incorporate both the functionality listed below:
// Extend URL Endpoints: You can extend the RESTful APIs into different URL endpoints to interact with the device's sensors and obtain data. The available endpoints are:

// a. / - Root endpoint
// b. /gyro/ - Gyroscope data endpoint
// c. /accel/ - Accelerometer data endpoint
// d. /temp/ - Temperature data endpoint

// Actuator Control: In addition to obtaining data from the device, the RESTful API can also trigger actuators on the device. You can extend the functionality to control various actuators, such as LED, buzzer, and LCD. The available actuator control endpoints are:
// a. /led/0 - Turn off the LED
// b. /led/1 - Turn on the LED
// c. /buzzer/0 - Turn off the buzzer
// d. /buzzer/1 - Turn on the buzzer

#include <WiFi.h>               // Include the WiFi library for networking
#include <WebServer.h>           // Include the WebServer library for creating an HTTP server
#include <M5StickCPlus.h>        // Include the M5StickCPlus library for handling the device's hardware

/* Put your SSID & Password */
const char* ssid = "Tj";         // Define your WiFi SSID
const char* password =  "00000000";  // Define your WiFi Password

// Sensor values to hold pitch, roll, yaw, and temperature data
float pitch, roll, yaw, temperature;

WebServer server(80);            // Create a web server object that listens on port 80 (HTTP)


// Pin definitions for LED and buzzer
#define M5_LED 10                // Define the GPIO pin number for the LED (change as necessary)
#define BUZZER_PIN 9             // Define the GPIO pin number for the buzzer (change as necessary)

void setup() {
  Serial.begin(115200);             // Initialize serial communication at 115200 baud rate

  // Initialize M5StickCPlus device
  M5.begin();  

  // Check if the IMU (Inertial Measurement Unit) initializes correctly
  if (M5.IMU.Init() != 0)
    Serial.println("IMU initialization failed!");

  // Set up the LCD screen on M5StickCPlus and display a message
  M5.Lcd.setRotation(3);            // Rotate the screen for correct orientation
  M5.Lcd.fillScreen(BLACK);         // Fill the screen with a black background
  M5.Lcd.setCursor(0, 0, 2);        // Set the cursor position
  M5.Lcd.printf("RESTful API", 0);  // Display text on the LCD screen

  // Configure pins for LED and buzzer as output
  pinMode(M5_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Ensure the LED and buzzer are turned off initially
  digitalWrite(M5_LED, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  WiFi.setHostname("group01-stick"); // Set a custom hostname for the device

  // Wait for WiFi connection
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {  // Loop until WiFi is connected
    delay(500);                             // Wait for 500ms before retrying
    Serial.print(".");                      // Display a dot for each retry
  }

  // Display the device's IP address on the LCD once connected to WiFi
  M5.Lcd.setCursor(0, 20, 2);
  M5.Lcd.print("IP: ");
  M5.Lcd.println(WiFi.localIP());  // Display the assigned IP address

  // Configure HTTP server endpoints
  server.on("/", handle_Root);               // Root endpoint
  server.on("/gyro/", handle_Gyro);         // Gyroscope data endpoint
  server.on("/accel/", handle_Accel);       // Accelerometer data endpoint
  server.on("/temp/", handle_Temp);         // Temperature data endpoint
  server.on("/led/0", handle_LED_Off);     // Turn off the LED endpoint
  server.on("/led/1", handle_LED_On);      // Turn on the LED endpoint
  server.on("/buzzer/1", handle_Buzzer_On); // Turn on the buzzer endpoint
  server.on("/buzzer/0", handle_Buzzer_Off); // Turn off the buzzer endpoint
  server.onNotFound(handle_NotFound);       // Handle requests to undefined endpoints

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");   // Notify in the serial monitor
}

// Function to handle the root endpoint ("/")
void handle_Root() {
  server.send(200, "text/plain", "Welcome to M5StickCPlus RESTful API");  // Send a welcome message
}

// Function to handle the gyroscope data endpoint ("/gyro/")
void handle_Gyro() {
  M5.IMU.getGyroData(&pitch, &roll, &yaw);  // Get gyroscope data from the IMU
  String message = "Gyroscope Data:\nPitch: " + String(pitch) + "\nRoll: " + String(roll) + "\nYaw: " + String(yaw);
  server.send(200, "text/plain", message);  // Send gyroscope data as text response
}

// Function to handle the accelerometer data endpoint ("/accel/")
void handle_Accel() {
  M5.IMU.getAccelData(&pitch, &roll, &yaw);  // Get accelerometer data from the IMU
  String message = "Accelerometer Data:\nPitch: " + String(pitch) + "\nRoll: " + String(roll) + "\nYaw: " + String(yaw);
  server.send(200, "text/plain", message);  // Send accelerometer data as text response
}

// Function to handle the temperature data endpoint ("/temp/")
void handle_Temp() {
  M5.IMU.getTempData(&temperature);  // Get temperature data from the IMU
  String message = "Temperature: " + String(temperature) + " °C";  // Create the temperature message
  server.send(200, "text/plain", message);  // Send the temperature data as a response
}

// Function to handle the LED off endpoint ("/led/0")
void handle_LED_Off() {
  digitalWrite(M5_LED, HIGH);     // Turn off the LED by setting the pin to HIGH
  server.send(200, "text/plain", "LED turned off");  // Send confirmation message
}

// Function to handle the LED on endpoint ("/led/1")
void handle_LED_On() {
  digitalWrite(M5_LED, LOW);      // Turn on the LED by setting the pin to LOW
  server.send(200, "text/plain", "LED turned on");   // Send confirmation message
}

// Function to handle the buzzer off endpoint ("/buzzer/0")
void handle_Buzzer_Off() {
  digitalWrite(BUZZER_PIN, LOW);  // Turn off the buzzer by setting the pin to LOW
  noTone(BUZZER_PIN);             // Stop any sound from the buzzer
  server.send(200, "text/plain", "Buzzer turned off");  // Send confirmation message
}

// Function to handle the buzzer on endpoint ("/buzzer/1")
void handle_Buzzer_On() {
  digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer by setting the pin to HIGH
  tone(BUZZER_PIN, 1000);         // Play a 1000 Hz tone
  server.send(200, "text/plain", "Buzzer turned on");  // Send confirmation message
}

// Function to handle undefined endpoints (404 error)
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");  // Send a 404 response for invalid URLs
}

void loop() {
  server.handleClient();   // Keep handling incoming HTTP requests
}

