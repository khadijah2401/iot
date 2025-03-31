// SERVER
#include <BLEDevice.h>          // Include BLEDevice library for Bluetooth Low Energy (BLE) functionality
#include <BLEServer.h>          // Include BLEServer library to handle BLE server operations
#include <M5StickCPlus.h>       // Include the M5StickCPlus library for controlling the M5StickCPlus device

// Define BLE UUIDs for the service and characteristics
#define SERVICE_UUID "01234679-0343-4566-89ab-0123456789ab"           // UUID for the BLE service
#define LED_STATE_CHARACTERISTIC_UUID "01234567-89ab-4567-89ab-0123456789ac"  // UUID for LED state characteristic
#define TEMPERATURE_CHARACTERISTIC_UUID "01234567-0123-4567-89ab-0123456789cd" // UUID for temperature characteristic
#define VOLTAGE_CHARACTERISTIC_UUID "01234567-0123-4567-89ab-0123456789ef"    // UUID for voltage characteristic

// BLE server characteristic objects
BLECharacteristic* ledStateCharacteristic;  // LED state characteristic
BLECharacteristic* temperatureCharacteristic; // Temperature characteristic
BLECharacteristic* voltageCharacteristic;  // Voltage characteristic

// LED state (initially off)
bool ledState = false;
#define LED_PIN 10  // Pin for the LED

// Function to display messages on the M5StickCPlus LCD
void displayMessage(const char* message) {
  M5.Lcd.fillScreen(BLACK);              // Clear the screen
  M5.Lcd.setCursor(0, 0, 2);            // Set the cursor position
  M5.Lcd.printf(message);                // Display the message
}

// BLEServerCallbacks class to handle client connection and disconnection
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected!");       // Log connection
    displayMessage("Client Connected!");       // Show on display
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected!");    // Log disconnection
    displayMessage("Client Disconnected!");    // Show on display
  }
};

// Custom callback class for LED state changes (via BLE)
class LEDStateCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) override {
    std::string value = pCharacteristic->getValue(); // Get value written to the characteristic

    if (value == "ON") {
      ledState = true;
      digitalWrite(LED_PIN, LOW);  // Turn LED ON (active low)
      Serial.println("LED turned ON by client");
    } else if (value == "OFF") {
      ledState = false;
      digitalWrite(LED_PIN, HIGH); // Turn LED OFF
      Serial.println("LED turned OFF by client");
    } else {
      Serial.println("Invalid LED state received from client");
    }

    // Update the display to show LED status
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0, 2);
    M5.Lcd.printf("LED: %s\n", ledState ? "ON" : "OFF");
  }
};

// BLE setup function
void setupBLE() {
  BLEDevice::init("THANOS");  // Initialize BLE device with name "THANOS"

  // Create the BLE server and service
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());  // Set the connection/disconnection callbacks
  BLEService* pService = pServer->createService(SERVICE_UUID);  // Create the service with the given UUID

  // Create characteristics for LED state, temperature, and voltage
  ledStateCharacteristic = pService->createCharacteristic(
    LED_STATE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  ledStateCharacteristic->setCallbacks(new LEDStateCallback());  // Set the callback for writing LED state

  // Characteristics for temperature and voltage, just read and notify
  temperatureCharacteristic = pService->createCharacteristic(
    TEMPERATURE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  voltageCharacteristic = pService->createCharacteristic(
    VOLTAGE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  // Set initial values for the characteristics
  ledStateCharacteristic->setValue(ledState ? "ON" : "OFF");
  temperatureCharacteristic->setValue("25.0");  // Initial temperature
  voltageCharacteristic->setValue("3.7");       // Initial voltage

  // Start the service
  pService->start();

  // Start advertising the BLE service
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);  // Add service UUID to advertisement
  pAdvertising->start();                      // Start advertising the service

  Serial.println("BLE Server is running...");
  displayMessage("Server Running...");
}

void setup() {
  Serial.begin(115200);         // Initialize serial communication
  M5.begin();                   // Initialize M5StickCPlus
  M5.Lcd.setRotation(3);        // Set screen rotation
  M5.Lcd.fillScreen(BLACK);     // Clear screen
  M5.Lcd.setCursor(0, 0, 2);    // Set cursor position
  M5.Lcd.println("BLE Server"); // Display message

  // Set up LED pin (G10) as output and turn LED off initially
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Start with LED off (active low)

  setupBLE();                   // Initialize the BLE service and server
}

// Function to read temperature from onboard sensor
float readTemperature() {
  return M5.Axp.GetTempData() / 100.0;  // Return temperature in Celsius (converted from raw data)
}

// Function to read battery voltage
float readVoltage() {
  return M5.Axp.GetVBusVoltage() / 1000.0;  // Return voltage in Volts (converted from raw data)
}

void loop() {
  M5.update();  // Update the M5StickCPlus (check for button presses)

  // Toggle LED state and notify connected clients when Button A is pressed
  if (M5.BtnA.wasPressed()) {
    ledState = !ledState;  // Toggle LED state
    ledStateCharacteristic->setValue(ledState ? "ON" : "OFF");  // Update BLE characteristic with new value
    ledStateCharacteristic->notify();  // Notify connected clients

    // Control the LED based on the state
    if (ledState) {
      digitalWrite(LED_PIN, LOW);  // Turn LED ON (LOW because the LED is active low)
    } else {
      digitalWrite(LED_PIN, HIGH); // Turn LED OFF
    }

    // Get real-time temperature and voltage
    float temperature = readTemperature();  // Actual temperature reading
    float voltage = readVoltage();         // Actual voltage reading

    // Convert readings to strings for BLE notifications
    String temperatureStr = String(temperature, 1);  // Temperature with 1 decimal precision
    String voltageStr = String(voltage, 2);          // Voltage with 2 decimal precision

    // Set characteristic values for temperature and voltage and notify clients
    temperatureCharacteristic->setValue(temperatureStr.c_str());
    temperatureCharacteristic->notify();
    voltageCharacteristic->setValue(voltageStr.c_str());
    voltageCharacteristic->notify();

    // Update display with LED, temperature, and voltage information
    M5.Lcd.fillScreen(BLACK);               // Clear the screen
    M5.Lcd.setCursor(0, 0, 2);             // Set cursor position
    M5.Lcd.printf("LED: %s\nTemp: %.1f C\nVoltage: %.2f V", 
                  ledState ? "ON" : "OFF", temperature, voltage);  // Display the information

    Serial.printf("LED: %s, Temp: %.1f, Voltage: %.2f\n", 
                  ledState ? "ON" : "OFF", temperature, voltage);  // Log the information to serial
  }
}

