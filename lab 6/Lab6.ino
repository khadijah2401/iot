// Basic Mesh Network Setup
// Task 1: Follow the guide to initialize your M5StickC devices into a mesh network. Ensure each device can join the network and communicate with others.

// Task 2: Modify the example code to send a simple "Hello from M5StickC [ID]" message from each device, where [ID] is the ID to uniquely identify each M5StickC in the network. Make sure the MESH_SSID and MESH_PASSWORD are as follows so that everyone joins the 'same' mesh network:

// #define   MESH_SSID       "csc2106meshy"
// #define   MESH_PASSWORD   "meshpotatoes"
// #define   MESH_PORT       5555

// Task 3: Implement functionality to display received messages on the M5StickC's scre

// Advanced Tasks (Not For Submission)
// Once you have the basic mesh network up and running, challenge yourself with the following advanced exercise (hint are available here):

// Custom Message Routing Protocol: Modify your code to implement a custom message routing protocol. Your protocol should include message priority handling, where messages tagged as "High Priority" are processed before others in the network queue. You can implement this in user code and do not need to modify the library directly.

// Step 1: Understand the library's message handling mechanism by reviewing the Painless Mesh library source code.
// Step 2: Design a simple priority scheme for messages (e.g., High, Medium, Low).
// Step 3: Implement the priority handling in the message routing function.
// Step 4: Test your custom protocol by sending messages with different priorities and observing the order in which they are processed. You might want to try this in your project grouping.

#include <painlessMesh.h>
#include <M5StickCPlus.h>

#ifdef LED_BUILTIN
#define LED LED_BUILTIN
#else
#define LED G10  // Define the LED pin if not using the built-in LED
#endif

#define BLINK_PERIOD    3000 // Time period (ms) for the LED blink cycle
#define BLINK_DURATION  100  // Duration (ms) that the LED stays ON

#define MESH_SSID       "kukubird"  // Mesh network SSID (Network name)
#define MESH_PASSWORD   "meshpotatoes"  // Mesh network password
#define MESH_PORT       5555  // Port for mesh communication

// Enum for message priority levels (High, Medium, Low)
enum MessagePriority {
  PRIORITY_HIGH = 0,
  PRIORITY_MEDIUM,
  PRIORITY_LOW
};

// Function prototypes for handling various mesh network events
void sendMessage();
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback(); 
void nodeTimeAdjustedCallback(int32_t offset); 
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler     userScheduler;  // A task scheduler to manage periodic tasks
painlessMesh  mesh;  // Instantiate the mesh network object

bool calc_delay = false;  // Flag to calculate delay (not used in this code)
SimpleList<uint32_t> nodes;  // List to store connected nodes

// Task to send messages periodically
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

// Task to blink the LED when no nodes are connected
Task blinkNoNodes;
bool onFlag = false;  // LED state flag

// Structure to represent a message with content, priority, and sender node ID
struct Message {
  String msg;  // Message content
  MessagePriority priority;  // Message priority
  uint32_t nodeId;  // Sender node ID
};

// Queue to store messages with priorities
SimpleList<Message> messageQueue;

void setup() {
  // Initialize M5StickC and Serial communication
  M5.begin();  
  M5.Lcd.setRotation(3);  // Set the rotation of the LCD screen
  M5.Lcd.fillScreen(BLACK);  // Set the background color of the screen to black
  M5.Lcd.setTextColor(WHITE);  // Set text color to white
  M5.Lcd.setTextSize(2);  // Set the text size on the screen

  Serial.begin(115200);  // Start serial communication for debugging
  pinMode(LED, OUTPUT);  // Set the LED pin as an output pin

  // Mesh network setup
  mesh.setDebugMsgTypes(ERROR | DEBUG);  // Enable debug messages for troubleshooting
  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);  // Initialize the mesh network with SSID, password, scheduler, and port
  mesh.onReceive(&receivedCallback);  // Register callback for receiving messages
  mesh.onNewConnection(&newConnectionCallback);  // Register callback for new node connections
  mesh.onChangedConnections(&changedConnectionCallback);  // Register callback for changes in node connections
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);  // Register callback for time adjustments
  mesh.onNodeDelayReceived(&delayReceivedCallback);  // Register callback for delay information from other nodes

  // Scheduler setup to manage periodic tasks
  userScheduler.addTask(taskSendMessage);  // Add the send message task to the scheduler
  taskSendMessage.enable();  // Enable the task to run periodically

  // Blink LED task setup: blinks the LED when no nodes are connected to the network
  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
    onFlag = !onFlag;  // Toggle the LED state (on or off)
    blinkNoNodes.delay(BLINK_DURATION);  // Wait for the blink duration

    // If it's the last iteration, adjust blink intervals
    if (blinkNoNodes.isLastIteration()) {
      blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);  // Update blink count
      blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000)) / 1000);  // Adjust blink delay based on node time
    }
  });
  userScheduler.addTask(blinkNoNodes);  // Add LED blink task to the scheduler
  blinkNoNodes.enable();  // Enable LED blinking task
}

void loop() {
  mesh.update();  // Update the mesh network (handle network operations)

  digitalWrite(LED, !onFlag);  // Control the LED state based on the onFlag (LED on/off)

  // Process messages in the queue based on their priority
  processMessages();
}

// Function to send a message at random intervals
void sendMessage() {
  String msg = "Hello from M5StickC ";  // Create a message
  msg += String(mesh.getNodeId());  // Append the node ID to the message
  
  // Assign a random priority (HIGH, MEDIUM, or LOW) to the message
  MessagePriority priority = static_cast<MessagePriority>(random(0, 3));  // Randomly select a priority
  Message message = {msg, priority, mesh.getNodeId()};  // Create a message object with content, priority, and sender ID
  
  // Add the message to the message queue
  messageQueue.push_back(message);  // Add to the queue

  Serial.printf("Sending message: %s with priority %d\n", msg.c_str(), priority);  // Print the message details to the Serial Monitor

  // Set the interval for the next message to be sent randomly
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));  // Randomize the interval between 1 and 5 seconds
}

// Function to process and send messages based on priority (high -> medium -> low)
void processMessages() {
  // Sort the queue by message priority (higher priority messages will be sent first)
  messageQueue.sort([](const Message& a, const Message& b) {
    return a.priority < b.priority;  // Sort messages by priority (ascending order)
  });

  // If there are messages in the queue, send the highest priority message
  if (messageQueue.size() > 0) {
    Message message = messageQueue.front();  // Get the highest priority message
    mesh.sendBroadcast(message.msg);  // Broadcast the message to all nodes in the network
    messageQueue.pop_front();  // Remove the message from the front of the queue
  }
}

// Callback function when a message is received from another node
void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("Received from %u: %s\n", from, msg.c_str());  // Print the sender and message to the Serial Monitor

  // Clear previous message on the LCD and display the new message
  M5.Lcd.fillRect(0, 10, 160, 70, BLACK);  // Clear previous message from the screen

  M5.Lcd.setCursor(10, 20);  // Set the cursor for the message on the screen
  M5.Lcd.setTextColor(WHITE);  // Set the text color to white
  M5.Lcd.setTextSize(2);  // Set the text size to 2
  M5.Lcd.print("From: ");  // Display the sender's ID
  M5.Lcd.println(from);

  M5.Lcd.setCursor(10, 50);  // Set the cursor for the message text
  M5.Lcd.setTextSize(1);  // Set the text size to 1
  M5.Lcd.print("Msg: ");  // Display the message content
  M5.Lcd.println(msg);
}

// Callback function for new node connections
void newConnectionCallback(uint32_t nodeId) {
  onFlag = false;  // Stop LED blinking
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);  // Adjust blink count
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000)) / 1000);  // Adjust blink timing

  // Print the details of the new connection
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
  Serial.printf("Connection Details: %s\n", mesh.subConnectionJson(true).c_str());
}

// Callback function for changes in node connections
void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  onFlag = false;  // Stop LED blinking
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);  // Adjust blink count
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD*1000)) / 1000);  // Adjust blink timing

  // Update and print the list of nodes in the mesh
  nodes = mesh.getNodeList();  // Get the list of connected nodes
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);  // Print each node ID
    node++;
  }
  Serial.println();
}

// Callback function for time adjustment
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);  // Print time adjustment details
}

// Callback function for delay received from a node
void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);  // Print the delay to a specific node
}
