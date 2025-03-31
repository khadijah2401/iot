**IoT Lab Quiz – Coding Focused**

### **Multiple Choice Questions (MCQ)**
1. In an MQTT-based IoT system, which function is used in Python to publish a message to a topic?
   a `mqtt.publish(topic, message)`  
   b `client.send(topic, message)`  
   c `client.publish(topic, message)`  
   d `mqtt.sendMessage(topic, message)`  
   **Answer:** c `client.publish(topic, message)`  

2. What library is commonly used in Python to implement MQTT communication?
   a `pymqtt`  
   b `mqttlib`  
   c `paho-mqtt`  
   d `iot-mqtt`  
   **Answer:** c `paho-mqtt`  

3. In an Arduino sketch, which function runs continuously after setup?
   a `initialize()`  
   b `loop()`  
   c `main()`  
   d `execute()`  
   **Answer:** b `loop()`  

4. What is the purpose of `client.loop_start()` in an MQTT client code?
   a To start a separate thread for handling network traffic  
   b To continuously send messages  
   c To stop the MQTT connection  
   d To reset the client buffer  
   **Answer:** a To start a separate thread for handling network traffic  

5. In Arduino, what function is used to read analog sensor data?
   a `digitalRead(pin)`  
   b `analogRead(pin)`  
   c `readSensor(pin)`  
   d `sensorRead(pin)`  
   **Answer:** b `analogRead(pin)`  

### **Multi-Select Questions**
6. Which of the following are required to set up an MQTT client in Python? (Select all that apply)
   - a) Define a broker address  
   - b) Initialize a `client` object  
   - c) Manually open network sockets  
   - d) Use `client.connect()` to establish connection  
   **Answer:** a) Define a broker address, b) Initialize a `client` object, d) Use `client.connect()` to establish connection  

7. What components are necessary for an Arduino to publish sensor data over MQTT? (Select all that apply)
   - a) WiFi module (or Ethernet shield)  
   - b) MQTT library  
   - c) Database connection  
   - d) Sensor  
   **Answer:** a) WiFi module (or Ethernet shield), b) MQTT library, d) Sensor  

### **Short Answer Questions**
8. What is the purpose of the `setup()` function in Arduino sketches?
   **Answer:** The `setup()` function initializes variables, pin modes, and starts communication protocols such as Serial or WiFi. It runs once at the beginning of the program.

9. In an MQTT system, what does QoS (Quality of Service) determine?
   **Answer:** QoS determines message delivery guarantees, with levels 0 (at most once), 1 (at least once), and 2 (exactly once).

10. Why do we use `client.loop_forever()` in MQTT Python scripts?
   **Answer:** `loop_forever()` keeps the MQTT client running, handling network traffic and incoming messages until the script is terminated.

11. How does a LoRa module transmit data in an IoT network?
   **Answer:** LoRa modules send data using long-range, low-power radio communication through gateways that connect to the internet or local servers.

12. What is the significance of using `millis()` instead of `delay()` in Arduino code?
   **Answer:** `millis()` allows non-blocking delays, enabling the microcontroller to handle other tasks while waiting, unlike `delay()`, which stops execution.

### **Fill in the Blanks**
13. The function used to subscribe to a topic in MQTT using Python is `________`.
   **Answer:** `client.subscribe(topic)`  

14. In Arduino, digital pins can be configured as input or output using `________`.
   **Answer:** `pinMode(pin, mode)`  

15. The function used to send data over a LoRa module in Arduino is `________`.
   **Answer:** `LoRa.beginPacket(); LoRa.print(data); LoRa.endPacket();`  

16. To connect an MQTT client to a broker, we use the function `________`.
   **Answer:** `client.connect(broker_address)`  

17. The command `Serial.begin(9600);` in Arduino is used to `________`.
   **Answer:** Initialize serial communication at a baud rate of 9600  

### **Code Interpretation Questions**
18. Given the following Arduino MQTT code snippet:
   ```cpp
   void loop() {
      int sensorValue = analogRead(A0);
      String payload = String(sensorValue);
      client.publish("sensor/data", payload.c_str());
      delay(1000);
   }
   ```
   What does this code do?
   **Answer:** It reads an analog sensor value from pin A0, converts it to a string, and publishes it to the MQTT topic `sensor/data` every second.

19. Identify the error in the following Python MQTT script:
   ```python
   import paho.mqtt.client as mqtt
   client = mqtt.Client()
   client.subscribe("home/sensor")
   client.connect("mqtt.example.com", 1883)
   client.loop_start()
   ```
   **Answer:** The `connect()` function should be called before `subscribe()`, otherwise, the client is subscribing before the connection is established.

20. What will be printed by the following Arduino Serial code?
   ```cpp
   void setup() {
      Serial.begin(9600);
   }
   void loop() {
      for (int i = 0; i < 5; i++) {
         Serial.println(i);
      }
      delay(5000);
   }
   ```
   **Answer:** The numbers 0, 1, 2, 3, 4 will be printed repeatedly every 5 seconds.
