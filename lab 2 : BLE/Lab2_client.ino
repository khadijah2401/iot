// Extend your lab to incorporate the following functionality:

// Designate M5Stick A as Server and M5Stick B as Client.
// Press the Server Button A to change the Server LED state and notify status updates (temperature, voltage) to the Client.
// Press the Client Button A to read characteristics (led) from the Server & display.
// Press the Client Button B to write characteristics (led) to the Server, changing the LED state on the Server.
// Hints
// You'll need to use BLE characteristics to represent the LED state, temperature, and voltage. Remember to update these characteristics whenever Button A is pressed on the Server.
// Use the M5.Lcd.print() function to display the received LED state on the Client's screen; helps with debugging.
// Ensure that the Client correctly identifies the "LED State" characteristic on the Server to control the LED.

// CLIENT
#include <BLEDevice.h>          // Include the BLEDevice library for Bluetooth Low Energy (BLE) functionality
#include <M5StickCPlus.h>       // Include the M5StickCPlus library for controlling the M5StickCPlus device

// Define BLE UUIDs for the service and characteristics
#define SERVICE_UUID "01234679-0343-4566-89ab-0123456789ab"           // UUID for the BLE service
#define LED_STATE_CHARACTERISTIC_UUID "01234567-89ab-4567-89ab-0123456789ac"  // UUID for LED state characteristic
#define TEMPERATURE_CHARACTERISTIC_UUID "01234567-0123-4567-89ab-0123456789cd" // UUID for temperature characteristic
#define VOLTAGE_CHARACTERISTIC_UUID "01234567-0123-4567-89ab-0123456789ef"    // UUID for voltage characteristic

// BLE client state variables
bool doConnect = false;   // Flag to indicate whether we should attempt a connection
bool connected = false;   // Flag to indicate if the client is connected to the server

BLEAddress* serverAddress;  // Address of the BLE server
BLERemoteCharacteristic* ledStateCharacteristic = nullptr;  // LED state characteristic
BLERemoteCharacteristic* temperatureCharacteristic = nullptr;  // Temperature characteristic
BLERemoteCharacteristic* voltageCharacteristic = nullptr;  // Voltage characteristic

// Function to display a message on the M5StickCPlus LCD screen
void displayMessage(const char* message) {
  M5.Lcd.fillScreen(BLACK);              // Clear the screen
  M5.Lcd.setCursor(0, 0, 2);            // Set the cursor position
  M5.Lcd.printf(message);                // Display the message
}

// Function to connect to a BLE server using its address
bool connectToServer(BLEAddress pAddress) {
  BLEClient* pClient = BLEDevice::createClient();  // Create a BLE client
  Serial.println("Connecting to server...");

  if (pClient->connect(pAddress)) {   // Try to connect to the server
    Serial.println("Connected to server");
    BLERemoteService* pService = pClient->getService(SERVICE_UUID); // Get the service

    if (pService != nullptr) {  // Check if the service is available
      // Get the characteristics from the service
      ledStateCharacteristic = pService->getCharacteristic(LED_STATE_CHARACTERISTIC_UUID);
      temperatureCharacteristic = pService->getCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID);
      voltageCharacteristic = pService->getCharacteristic(VOLTAGE_CHARACTERISTIC_UUID);

      // Check if all the characteristics were found
      if (ledStateCharacteristic && temperatureCharacteristic && voltageCharacteristic) {
        connected = true;   // Mark as connected
        Serial.println("Characteristics found.");
        return true;        // Successfully connected and found characteristics
      }
    }
  }

  Serial.println("Failed to connect or find characteristics.");
  return false;  // Failed to connect or find characteristics
}

// BLE callback class to handle advertised devices
class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.println("Found advertised device");
    Serial.print("Device Name: ");
    Serial.println(advertisedDevice.getName().c_str());

    // If the device name matches "THANOS", stop scanning and connect
    if (advertisedDevice.getName() == "THANOS") {
      advertisedDevice.getScan()->stop();  // Stop scanning for devices
      serverAddress = new BLEAddress(advertisedDevice.getAddress());  // Store the server address
      doConnect = true;    // Set flag to start connection
      Serial.println("Found server. Connecting...");
    }
  }
};

void setup() {
  Serial.begin(115200);         // Start serial communication
  M5.begin();                   // Initialize M5StickCPlus
  M5.Lcd.setRotation(3);        // Set the screen rotation
  M5.Lcd.fillScreen(BLACK);     // Clear the screen
  M5.Lcd.setCursor(0, 0, 2);    // Set the cursor position
  M5.Lcd.println("BLE Client"); // Display "BLE Client" on the screen

  BLEDevice::init("");          // Initialize BLE device without a name

  // Start scanning for BLE devices
  BLEScan* pScan = BLEDevice::getScan();
  pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks()); // Set the callback to handle advertised devices
  pScan->setActiveScan(true);     // Enable active scanning (for better performance)
  pScan->start(30, false);       // Start scanning for 30 seconds
}

void loop() {
  M5.update();  // Update the M5StickCPlus (check for button presses)

  // If we need to connect to the server and aren't already connected
  if (doConnect && !connected) {
    if (connectToServer(*serverAddress)) {  // Try to connect
      displayMessage("Connected!");         // Display success message
    } else {
      displayMessage("Connection failed");  // Display failure message
    }
    doConnect = false;  // Reset the connection flag
  }

  if (connected) {  // If we're connected to the server
    // If button A is pressed, read and display the LED state
    if (M5.BtnA.wasPressed()) {
      if (ledStateCharacteristic->canRead()) {  // Check if the LED state characteristic is readable
        std::string value = ledStateCharacteristic->readValue();  // Read the LED state
        M5.Lcd.fillScreen(BLACK);          // Clear the screen
        M5.Lcd.setCursor(0, 0, 2);        // Set the cursor position
        M5.Lcd.printf("LED State: %s", value.c_str());  // Display LED state
        Serial.printf("Read LED State: %s\n", value.c_str());  // Print LED state to serial
        delay(2000);  // Keep the message displayed for 2 seconds
      }
    }

    // If button B is pressed, toggle the LED state and write it
    if (M5.BtnB.wasPressed()) {
      static bool ledState = false;  // Track the LED state (on/off)
      ledState = !ledState;          // Toggle the LED state

      if (ledStateCharacteristic->canWrite()) {  // Check if the LED state characteristic is writable
        std::string value = ledState ? "ON" : "OFF";  // Set the new LED state value ("ON" or "OFF")
        ledStateCharacteristic->writeValue(value);  // Write the value to the LED state characteristic

        M5.Lcd.fillScreen(BLACK);              // Clear the screen
        M5.Lcd.setCursor(0, 0, 2);            // Set the cursor position
        M5.Lcd.printf("Set LED: %s", value.c_str());  // Display the new LED state
        Serial.printf("Set LED State to: %s\n", value.c_str());  // Print the new LED state to serial
      } else {
        Serial.println("LED State characteristic is not writable.");
      }
    }

    // Read and display temperature and voltage from the BLE server
    if (temperatureCharacteristic->canRead() && voltageCharacteristic->canRead()) {
      std::string temp = temperatureCharacteristic->readValue();  // Read the temperature
      std::string volt = voltageCharacteristic->readValue();  // Read the voltage

      M5.Lcd.fillScreen(BLACK);           // Clear the screen
      M5.Lcd.setCursor(0, 0, 2);         // Set the cursor position
      M5.Lcd.printf("Temp: %s\n", temp.c_str());   // Display the temperature
      M5.Lcd.printf("Voltage: %s\n", volt.c_str());  // Display the voltage

      Serial.printf("Temp: %s, Voltage: %s\n", temp.c_str(), volt.c_str());  // Print temperature and voltage to serial
    }

    delay(500);  // Delay for 500ms before the next loop iteration
  }
}
