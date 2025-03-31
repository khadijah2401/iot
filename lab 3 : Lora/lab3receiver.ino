//  OLED Display Integration
// Display the following information on the OLED for both LoRa Receiver & Sender:
// "Setup Successful"
// "Setup Failed"
// "Sending Message"
// "Waiting for Reply"
// “Message Received”
// Refer to ssd1306_i2c.ino for OLED display implementation. Install the “Adafruit SSD1306” library first.
// Enhance Peer-to-Peer Reliability

// Implement features to prevent crosstalk traffic interference, e.g., ACK and retransmit.
// Improve Message Protocol

// Develop ID-based messaging with a header, payload, and checksum supporting at least three devices, i.e. simple message filtering, only accepting messages that are directed to the node's agreed ID.


// RECEIVER 
#include <SPI.h>              // Include SPI library for communication with the LoRa module
#include <RH_RF95.h>          // Include RH_RF95 library for LoRa communication
#include <Wire.h>             // Include Wire library for I2C communication with the OLED display
#include <Adafruit_GFX.h>     // Include Adafruit_GFX library for drawing on the OLED display
#include <Adafruit_SSD1306.h> // Include SSD1306 library for controlling the OLED display

#define SCREEN_WIDTH 128      // OLED display width
#define SCREEN_HEIGHT 16      // OLED display height
#define OLED_RESET -1         // Reset pin for the OLED (not used in this case)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);  // OLED display object

#define RFM95_CS 10           // LoRa chip select pin
#define RFM95_RST 9           // LoRa reset pin
#define RFM95_INT 2           // LoRa interrupt pin
#define RF95_FREQ 900.0       // LoRa frequency (900 MHz)

RH_RF95 rf95(RFM95_CS, RFM95_INT);  // LoRa object using the defined pins

// Header
// ID-based Messaging
const uint8_t myID = 2;      // Unique ID of this node (Receiver)
const uint8_t senderID = 1;  // Expected transmitter's ID (Sender)

// Message Types
const uint8_t MSG_TYPE_DATA = 0x01; // Data message type
const uint8_t MSG_TYPE_ACK  = 0x02; // Acknowledgment message type

// Function to calculate checksum for message validation
uint8_t calculateChecksum(uint8_t *data, uint8_t length) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < length - 1; i++) { // Iterate over all bytes except the last one
    sum += data[i];  // Sum all the bytes
  }
  return sum;  // Return the calculated checksum
}

void setup() {
  Serial.begin(9600);      // Start serial communication for debugging
  delay(100);              // Short delay

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));  // Check for OLED initialization failure
    while (1);  // Loop here if OLED initialization fails
  }
  
  display.setTextSize(1);         // Set text size for display
  display.setTextColor(WHITE);    // Set text color to white
  display.clearDisplay();         // Clear the screen
  displayMessage("Initializing...");  // Display message on screen

  // Reset the LoRa module
  pinMode(RFM95_RST, OUTPUT);     
  digitalWrite(RFM95_RST, LOW);   // Set RST pin low
  delay(10);                      // Short delay
  digitalWrite(RFM95_RST, HIGH);  // Set RST pin high
  delay(10);                      // Short delay

  // Initialize the LoRa module
  if (!rf95.init()) {
    Serial.println("LoRa init failed");    // If LoRa initialization fails
    displayMessage("Setup Failed");        // Display "Setup Failed" message on the screen
    while (1);  // Loop here if initialization fails
  }

  // Set LoRa frequency
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");  // If setting frequency fails
    displayMessage("Setup Failed");         // Display "Setup Failed" message on the screen
    while (1);  // Loop here if frequency setting fails
  }

  rf95.setTxPower(13, false);   // Set transmission power to 13 dBm
  displayMessage("Setup Successful");  // Display "Setup Successful" message on the screen
  delay(2000);  // Wait for 2 seconds
}

void loop() {
  if (rf95.available()) {  // Check if there is an incoming message
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];  // Buffer to store the received message
    uint8_t len = sizeof(buf);             // Length of the buffer
    if (rf95.recv(buf, &len)) {            // If a message is received
      // Check that we have at least a header (3 bytes) and checksum (1 byte)
      if (len >= 4) {  // Ensure there is enough data (header + checksum)
        uint8_t sender = buf[0];    // Extract sender ID
        uint8_t receiver = buf[1];  // Extract receiver ID
        uint8_t msgType = buf[2];   // Extract message type

        // Filtering Messages
        // Process only if the message is intended for this node and is a data message
        if (receiver == myID && msgType == MSG_TYPE_DATA) {
          uint8_t checksumReceived = buf[len - 1];  // Extract the received checksum
          if (checksumReceived == calculateChecksum(buf, len)) {  // Validate checksum
            Serial.println("Message Received");  // Log message reception
            displayMessage("Message Received");  // Display "Message Received" on screen
            delay(1000);  // Brief pause before retrying

            // Send acknowledgment (ACK) back to the sender
            sendAck(sender);  // Send ACK message to the sender
          } else {
            Serial.println("Invalid Checksum");  // Log checksum error
          }
        }
      }
    }
  }
}

// Function to send acknowledgment (ACK) to the sender
void sendAck(uint8_t receiverID) {
  uint8_t ackMsg[4] = { myID, receiverID, MSG_TYPE_ACK, 0 };  // Create ACK message
  ackMsg[3] = calculateChecksum(ackMsg, sizeof(ackMsg));  // Calculate checksum for ACK message
  
  Serial.println("Sending ACK...");   // Log sending ACK
  displayMessage("Sending ACK");      // Display "Sending ACK" on screen
  
  rf95.send(ackMsg, sizeof(ackMsg));  // Send the ACK message
  rf95.waitPacketSent();              // Wait until the packet is sent
}

// Function to display messages on the OLED display
void displayMessage(const char* msg) {
  display.clearDisplay();  // Clear the screen
  display.setCursor(0, 0);  // Set cursor to top-left corner
  display.println(msg);      // Print the message on the screen
  display.display();         // Update the display
}