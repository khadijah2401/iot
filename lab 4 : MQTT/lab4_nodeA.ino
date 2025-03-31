// Node Interaction Using MQTT
// Configure two M5StickC devices (Node A and Node B).
// Node A should toggle Node B's LED (and vice-versa) upon button press.
// Implement and propose appropriate topic names for this interaction.

// NODE A 
#include <WiFi.h>
#include <PubSubClient.h>
#include <M5StickCPlus.h>

// WiFi and MQTT configurations
WiFiClient espClient;  // WiFi client instance
PubSubClient client(espClient);  // MQTT client instance

const char* ssid = "kukubird";  // WiFi SSID
const char* password = "kukubird";  // WiFi password
const char* mqtt_server = "172.20.10.14";  // MQTT broker IP

#define LED_PIN 10  // Pin where the LED is connected
#define BUTTON_PIN M5.BtnA  // Built-in button on the M5StickC

// Function prototypes
void setupWifi();  // Function to connect to WiFi
void callback(char* topic, byte* payload, unsigned int length);  // MQTT message handler
void reConnect();  // Function to reconnect MQTT if disconnected

void setup() {
    M5.begin();  // Initialize M5StickC
    M5.Lcd.setRotation(3);  // Set the LCD rotation

    pinMode(LED_PIN, OUTPUT);  // Set LED pin as output
    digitalWrite(LED_PIN, LOW);  // Initially turn off the LED

    setupWifi();  // Connect to WiFi
    client.setServer(mqtt_server, 1883);  // Set up the MQTT server (IP and port)
    client.setCallback(callback);  // Set the callback function for MQTT messages
}

void loop() {
    M5.update();  // Update M5StickC button state

    if (!client.connected()) {  // Check if the client is not connected
        reConnect();  // Try reconnecting to MQTT broker
    }
    client.loop();  // Maintain MQTT connection and handle incoming messages

    if (M5.BtnA.wasPressed()) {  // Check if the button is pressed
        M5.Lcd.println("Button Pressed!");  // Display message on the screen
        client.publish("nodeA_to_nodeB_led", "TOGGLE");  // Publish message to toggle the LED on nodeB
        delay(500);  // Debounce delay to avoid multiple presses being registered
    }
}

// Function to connect to WiFi
void setupWifi() {
    delay(10);
    M5.Lcd.printf("Connecting to %s", ssid);  // Display the WiFi SSID on the screen
    WiFi.mode(WIFI_STA);  // Set WiFi mode to station (client)
    WiFi.begin(ssid, password);  // Start WiFi connection

    // Wait until connected to WiFi
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);  // Wait for 500 ms before checking status again
        M5.Lcd.print(".");  // Display a dot on the screen while connecting
    }
    M5.Lcd.println("\nWiFi Connected!");  // Display message when WiFi is connected
}

// Function to handle incoming MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
    M5.Lcd.print("Message arrived: [");  // Display incoming topic on the screen
    M5.Lcd.print(topic);
    M5.Lcd.print("] ");

    String receivedMessage = "";  // String to hold the message content
    for (int i = 0; i < length; i++) {  // Convert payload byte array to string
        receivedMessage += (char)payload[i];
    }
    M5.Lcd.println(receivedMessage);  // Display the message on the screen

    // If the topic is "led" and the message is "TOGGLE", toggle the LED state
    if (String(topic) == "led") {
        if (receivedMessage == "TOGGLE") {
            M5.Lcd.println("Toggling LED!");  // Display toggle message on screen
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle the LED state
        }
    }
}

// Function to reconnect to the MQTT broker
void reConnect() {
    while (!client.connected()) {  // Keep trying until connected
        M5.Lcd.print("Attempting MQTT connection...");  // Display connection attempt message
        String clientId = "NodeA-";
        clientId += String(random(0xffff), HEX);  // Generate a random client ID for the connection

        // Try to connect to the MQTT broker
        if (client.connect(clientId.c_str())) {
            M5.Lcd.println("Connected!");  // Display connected message
            client.subscribe("nodeB_to_nodeA_led");  // Subscribe to the topic to receive LED toggle messages
        } else {
            M5.Lcd.print("Failed, rc=");
            M5.Lcd.print(client.state());  // Display connection error code
            M5.Lcd.println(" retrying in 5s...");  // Retry after 5 seconds
            delay(5000);  // Wait for 5 seconds before retrying
        }
    }
}
