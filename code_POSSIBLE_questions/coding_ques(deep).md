### **Lab 1: REST API Code**  
1. **What does this line do?**  
   ```cpp
   WebServer server(80);
   ```
   **Answer:** Creates a web server object listening on port 80 for HTTP requests.

2. **Why is `M5.IMU.getGyroData(&pitch, &roll, &yaw)` used?**  
   **Answer:** Reads gyroscope data (pitch/roll/yaw) from the IMU sensor into the specified variables.

3. **What happens if `WiFi.status() != WL_CONNECTED` is true indefinitely?**  
   **Answer:** The device stalls in a loop, printing dots to serial until WiFi connects.

4. **Explain this snippet:**  
   ```cpp
   digitalWrite(LED_PIN, !digitalRead(LED_PIN));
   ```
   **Answer:** Toggles the LED state (ON→OFF or OFF→ON) by reading the current state and writing its inverse.

5. **What is the purpose of `server.send(200, "text/plain", message)`?**  
   **Answer:** Sends an HTTP 200 (OK) response with plain text content (e.g., sensor data).

---

### **Lab 2: BLE Code**  
6. **What does this code do?**  
   ```cpp
   pCharacteristic->setValue("OFF");
   pCharacteristic->notify();
   ```
   **Answer:** Updates the BLE characteristic value to "OFF" and notifies subscribed clients of the change.

7. **Why is `BLEDevice::init("THANOS")` called?**  
   **Answer:** Initializes the BLE stack and sets the device name to "THANOS" for advertising.

8. **What happens if `pService->getCharacteristic(LED_STATE_UUID)` returns `nullptr`?**  
   **Answer:** The Client fails to control the LED because the characteristic is not found.

9. **Explain this Client code:**  
   ```cpp
   if (advertisedDevice.getName() == "THANOS") { ... }
   ```
   **Answer:** The Client checks if the advertised BLE device name matches "THANOS" before connecting.

10. **What is the bug in this Server callback?**  
    ```cpp
    void onWrite(BLECharacteristic *pChar) {
      digitalWrite(LED_PIN, HIGH); // Always turns LED ON
    }
    ```
    **Answer:** It ignores the written value (`pChar->getValue()`) and hardcodes the LED state.

---

### **Lab 3: LoRa Code**  
11. **What does this line do?**  
    ```cpp
    rf95.setFrequency(RF95_FREQ);
    ```
    **Answer:** Sets the LoRa radio’s operating frequency (e.g., 900 MHz).

12. **Why is this check needed?**  
    ```cpp
    if (len >= sizeof(checksum)) { ... }
    ```
    **Answer:** Ensures the received LoRa packet has enough data to include a valid checksum.

13. **What is the purpose of `rf95.waitPacketSent()`?**  
    **Answer:** Blocks until the LoRa packet is fully transmitted, preventing buffer overflows.

14. **Explain this transmitter code:**  
    ```cpp
    uint8_t data[] = "Hello";
    rf95.send(data, sizeof(data));
    ```
    **Answer:** Sends the string "Hello" as a LoRa packet (includes null terminator).

15. **What happens if `rf95.init()` fails?**  
    **Answer:** The program halts with `while (1);` and prints "LoRa init failed".

---

### **Lab 4: MQTT Code**  
16. **What does this callback do?**  
    ```cpp
    void callback(char *topic, byte *payload, unsigned int length) {
      String message = String((char*)payload).substring(0, length);
    }
    ```
    **Answer:** Converts the MQTT payload byte array into a String for processing.

17. **Why is `client.loop()` called in `loop()`?**  
    **Answer:** Maintains the MQTT connection and processes incoming messages.

18. **What is the bug here?**  
    ```cpp
    client.publish("topic", "message"); // No check if connected
    ```
    **Answer:** The message may be lost if the client is not connected to the broker.

19. **Explain this reconnection logic:**  
    ```cpp
    if (!client.connected()) { reconnect(); }
    ```
    **Answer:** Attempts to reconnect to the MQTT broker if the connection drops.

20. **What does `WiFi.mode(WIFI_STA)` do?**  
    **Answer:** Configures the ESP32 as a WiFi client (station), not an access point.

---

### **Lab 5: LoRaWAN Code**  
21. **What is the purpose of this line?**  
    ```cpp
    LMIC_setupChannel(0, 923200000, DR_RANGE_MAP(DR_SF12, DR_SF7));
    ```
    **Answer:** Configures LoRaWAN channel 0 with AU915 frequency and SF7–SF12 data rates.

22. **Why is `os_runloop_once()` called in `loop()`?**  
    **Answer:** Processes LoRaWAN stack tasks (e.g., joining, sending/receiving).

23. **What does this code do?**  
    ```cpp
    LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
    ```
    **Answer:** Queues a LoRaWAN uplink message with confirmed delivery (`0 = unconfirmed`).

24. **Explain this OTAA join setup:**  
    ```cpp
    LMIC_startJoining();
    ```
    **Answer:** Initiates the Over-The-Air Activation (OTAA) process with the network server.

25. **What happens if `LMIC_reset()` is omitted?**  
    **Answer:** Previous LoRaWAN session state may cause undefined behavior.

---

### **Lab 6: Mesh Code**  
26. **What does this task do?**  
    ```cpp
    Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);
    ```
    **Answer:** Schedules `sendMessage()` to run every 1 second indefinitely.

27. **Explain this priority comparator:**  
    ```cpp
    return a.priority < b.priority;
    ```
    **Answer:** Sorts messages in ascending priority (lower numbers = higher priority).

28. **What is the purpose of `mesh.onReceive(&receivedCallback)`?**  
    **Answer:** Assigns a callback to handle incoming mesh messages.

29. **Why is `mesh.update()` called in `loop()`?**  
    **Answer:** Handles network maintenance (e.g., routing, node discovery).

30. **What happens if `MESH_PASSWORD` differs between nodes?**  
    **Answer:** Nodes cannot join the same mesh network (authentication fails).

---

### **Debugging Scenarios**  
31. **The REST API returns empty sensor data. Why?**  
    **Answer:** `M5.IMU.getGyroData()` failed (check IMU initialization).

32. **BLE Client crashes after connection. What’s wrong?**  
    ```cpp
    BLERemoteCharacteristic* pChar = nullptr;
    pChar->readValue(); // Null pointer dereference
    ```
    **Answer:** `pChar` is never assigned a valid characteristic.

33. **LoRa receiver gets garbled messages. How to fix?**  
    **Answer:** Ensure matching frequency/SF settings and checksum validation.

34. **MQTT messages are delayed by 10 seconds. Cause?**  
    **Answer:** `client.loop()` is called too infrequently in `loop()`.

35. **Mesh nodes show "Setup Failed". Debug steps?**  
    **Answer:** Verify `MESH_SSID`, `MESH_PASSWORD`, and WiFi antenna connection.

---

### **Advanced Code Questions**  
36. **How would you modify the REST API to serve HTML?**  
    **Answer:** Use `server.send(200, "text/html", "<h1>Hello</h1>")`.

37. **Optimize BLE for lower power usage.**  
    **Answer:** Increase advertisement interval and use `BLEDevice::setPower()`.

38. **Add encryption to LoRa messages.**  
    **Answer:** Implement AES-128 in `rf95.send()`/`rf95.receive()`.

39. **Secure MQTT with TLS.**  
    **Answer:** Use `WiFiClientSecure` and broker’s CA certificate.

40. **Extend the mesh to support OTA updates.**  
    **Answer:** Use `painlessMesh::startDelayNodeOTA()`.

