// TRANSMITTER
#include <SPI.h>
#include <RH_RF95.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED screen setup parameters
#define SCREEN_WIDTH 128  
#define SCREEN_HEIGHT 16  
#define OLED_RESET -1  
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 8);

// LoRa module pin definitions
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2
#define RF95_FREQ 900.0  // Frequency for LoRa communication

RH_RF95 rf95(RFM95_CS, RFM95_INT);  // Instantiate LoRa object with the given pins

// ID-based Messaging
const uint8_t myID = 1;       // Unique ID of this node (Transmitter)
const uint8_t targetID = 2;   // Intended receiver's ID

// Message Types
const uint8_t MSG_TYPE_DATA = 0x01;  // Data message type
const uint8_t MSG_TYPE_ACK  = 0x02;  // Acknowledgment message type

// Function to calculate a simple checksum over all message bytes except the checksum byte
uint8_t calculateChecksum(uint8_t *data, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length - 1; i++) {  // Sum all bytes except the last one (checksum)
    sum += data[i];
  }
  return sum;  // Return the checksum
}

void setup() {
  Serial.begin(9600);  // Initialize serial communication for debugging
  delay(200);
  
  // Explicitly initialize I2C communication for the OLED display
  Wire.begin();

  // Initialize OLED display and check if successful
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));  // If initialization fails, show error on Serial Monitor
    while (1);  // Halt the program
  }

  display.setTextSize(1);  // Set text size for display
  display.setTextColor(WHITE);  // Set text color to white
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);  // Clear screen
  displayMessage("Initializing...");

  // Initialize LoRa module by resetting the RFM95 module
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, LOW);  // Reset the module
  delay(10);
  digitalWrite(RFM95_RST, HIGH);  // Set the reset pin high to finish the reset
  delay(10);

  // Initialize the LoRa module
  if (!rf95.init()) {
    Serial.println("LoRa init failed");  // Print error if LoRa fails to initialize
    displayMessage("Setup Failed");
    while (1);  // Halt program
  }

  // Set the frequency for LoRa communication
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");  // Print error if frequency setup fails
    displayMessage("Setup Failed");
    while (1);  // Halt program
  }

  // Set LoRa transmission power to 13 dBm
  rf95.setTxPower(13, false);
  displayMessage("Setup Successful");
  delay(2000);  // Wait 2 seconds before continuing
}

void loop() {
  // Main loop: Send a message every 5 seconds with retransmissions if no ACK is received
  sendMessageWithRetransmission(targetID, MSG_TYPE_DATA, "Hello from Transmitter!");
  delay(5000);  // Wait for 5 seconds before sending the next message
}

// Function to send a message with retransmission and acknowledgment
void sendMessageWithRetransmission(uint8_t receiverID, uint8_t msgType, const char* payload) {
  uint8_t payloadLen = strlen(payload);  // Get the length of the payload
  // Prepare the message: [SenderID, ReceiverID, MsgType, Payload..., Checksum]
  uint8_t message[payloadLen + 4];
  message[0] = myID;  // Sender ID
  message[1] = receiverID;  // Receiver ID
  message[2] = msgType;  // Message type
  memcpy(&message[3], payload, payloadLen);  // Copy the payload into the message
  message[3 + payloadLen] = calculateChecksum(message, payloadLen + 4);  // Calculate and append the checksum

  const uint8_t maxRetries = 3;  // Maximum number of retries in case of failure
  bool ackReceived = false;  // Flag to check if acknowledgment is received
  uint8_t attempts = 0;  // Counter for number of attempts

  // Retransmission loop
  while (!ackReceived && attempts < maxRetries) {
    Serial.println("Sending Message...");
    displayMessage("Sending Message");
    delay(1000);  // Delay before sending

    // Send the message using LoRa
    rf95.send(message, sizeof(message));
    rf95.waitPacketSent();  // Wait until the message is sent

    // Wait for ACK response
    displayMessage("Waiting for Reply");
    unsigned long startTime = millis();  // Start a timer for timeout
    while (millis() - startTime < 3000) {  // 3-second timeout
      if (rf95.available()) {  // If a message is available
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];  // Buffer to store received message
        uint8_t len = sizeof(buf);
        if (rf95.recv(buf, &len)) {  // Receive the message
          // Check if this is an ACK response from the receiver
          if (len >= 4 && buf[0] == receiverID && buf[1] == myID && buf[2] == MSG_TYPE_ACK) {
            uint8_t checksumReceived = buf[len - 1];  // Get received checksum
            if (checksumReceived == calculateChecksum(buf, len)) {  // Validate checksum
              Serial.println("ACK Received");  // Print ACK received message
              ackReceived = true;  // Set flag to true
              displayMessage("Message Received");
              break;  // Exit loop as ACK is received
            }
          }
        }
      }
    }
    
    // If ACK is not received, retry the transmission
    if (!ackReceived) {
      attempts++;  // Increment attempt counter
      Serial.println("No ACK Received, retrying...");
      displayMessage("No ACK, Retry");
      delay(1000);  // Brief pause before retrying
    }
  }

  // If no ACK is received after retries, show failure message
  if (!ackReceived) {
    Serial.println("Message failed after retries.");
    displayMessage("Send Failed");
  }
}

// Function to display a message on the OLED screen
void displayMessage(const char* msg) {
  display.clearDisplay();  // Clear the display
  display.setCursor(0, 0);  // Set the cursor position at the top-left corner
  display.println(msg);  // Display the message
  display.display();  // Update the screen
}

