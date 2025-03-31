// NODE B
#include <WiFi.h>
#include <PubSubClient.h>
#include <M5StickCPlus.h>

// Initialize WiFi client and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

// WiFi and MQTT configurations
const char* ssid = "kukubird";  // WiFi SSID
const char* password = "kukubird";  // WiFi password
const char* mqtt_server = "172.20.10.14";  // MQTT Broker IP

#define LED_PIN 10  // Built-in LED pin on M5StickC Plus
#define BUTTON_PIN M5.BtnA  // Built-in button (Button A)

// Function prototypes for key operations
void setupWifi();  // Function to connect to WiFi
void callback(char* topic, byte* payload, unsigned int length);  // MQTT message callback handler
void reConnect();  // Function to reconnect MQTT client if disconnected

void setup() {
    M5.begin();  // Initialize M5StickC Plus hardware
    M5.Lcd.setRotation(3);  // Set the LCD screen rotation
    M5.Lcd.println("Node B Starting...");  // Initial message on LCD

    pinMode(LED_PIN, OUTPUT);  // Set LED pin as an output
    digitalWrite(LED_PIN, LOW);  // Ensure LED is off initially

    setupWifi();  // Connect to WiFi network
    client.setServer(mqtt_server, 1883);  // Set MQTT server and port
    client.setCallback(callback);  // Set the callback function for receiving messages
}

void loop() {
    M5.update();  // Update the state of the M5StickC (check for button press)

    if (!client.connected()) {  // Check if the MQTT client is connected
        reConnect();  // Reconnect to MQTT broker if not connected
    }
    client.loop();  // Keep the MQTT connection alive and process incoming messages

    // If the button (Button A) is pressed
    if (M5.BtnA.wasPressed()) {
        M5.Lcd.println("Button Pressed!");  // Display message on LCD
        client.publish("nodeB_to_nodeA_led", "TOGGLE");  // Publish a message to toggle the LED on Node A
        delay(500);  // Simple debounce to avoid multiple presses being registered
    }
}

// Function to connect to WiFi
void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);  // Display WiFi SSID on the screen
    WiFi.mode(WIFI_STA);  // Set the WiFi mode to station (client mode)
    WiFi.begin(ssid, password);  // Start WiFi connection using the SSID and password

    // Wait for WiFi to connect
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);  // Delay before checking WiFi status again
        M5.Lcd.print(".");  // Display a dot on the screen while connecting
    }
    M5.Lcd.println("\nWiFi Connected!");  // Display success message when connected
}

// Callback function for handling incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
    M5.Lcd.print("Message arrived: [");  // Display topic of received message
    M5.Lcd.print(topic);
    M5.Lcd.print("] ");

    // Convert the payload (byte array) to a string
    String receivedMessage;
    for (int i = 0; i < length; i++) {
        receivedMessage += (char)payload[i];
    }
    M5.Lcd.println(receivedMessage);  // Display the received message

    // If the message topic is "led" and the message is "TOGGLE"
    if (String(topic) == "led") {
        if (receivedMessage == "TOGGLE") {  // Toggle the LED state
            M5.Lcd.println("Toggling LED!");  // Display toggle message on screen
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle the LED state (turn on/off)
        }
    }
}

// Function to reconnect to MQTT broker if the client is disconnected
void reConnect() {
    while (!client.connected()) {  // Keep trying to reconnect until successful
        M5.Lcd.print("Attempting MQTT connection...");  // Display reconnect attempt message
        String clientId = "NodeB-";  // Create a unique client ID
        clientId += String(random(0xffff), HEX);  // Append a random number to the client ID

        // Try connecting to the MQTT broker
        if (client.connect(clientId.c_str())) {
            M5.Lcd.println("Connected!");  // Display connected message
            client.subscribe("led");  // Subscribe to the topic "led" to receive messages from Node A
        } else {
            // If connection fails, display the error code and retry after 5 seconds
            M5.Lcd.print("Failed, rc=");
            M5.Lcd.print(client.state());
            M5.Lcd.println(" retrying in 5s...");
            delay(5000);  // Wait for 5 seconds before retrying
        }
    }
}

