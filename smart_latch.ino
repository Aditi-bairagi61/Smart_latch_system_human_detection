#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "Galaxyy";
const char *password = "123123123";

ESP8266WebServer server(80);

// IR sensor pins
const int sensorPin1 = D1; // First sensor further to the entrance
const int sensorPin2 = D2; // Second sensor closer from the entrance
const int ledPin = D5;     // LED pin
const int fanPin = D6;     // Fan pin

// TTP223B Touch Sensor and Relay (Electronic Latch) pins
const int touchSensorPin = D3; // TTP223B Touch Sensor pin
const int relayPin = D7;       // Relay pin for electronic latch

bool sensor1State = HIGH;
bool sensor2State = HIGH;
int enteringCount = 0;
int exitingCount = 0;
int inRoomCount = 0;

// Function declarations
void openLatch();
void closeLatch();

void setup() {
    Serial.begin(115200);
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.println(WiFi.localIP());

    // Initialize IR sensor and output pins
    pinMode(sensorPin1, INPUT);
    pinMode(sensorPin2, INPUT);
    pinMode(ledPin, OUTPUT);
    pinMode(fanPin, OUTPUT);

    // Initialize TTP223B Touch Sensor pin
    pinMode(touchSensorPin, INPUT_PULLUP);

    // Initialize relay pin
    pinMode(relayPin, OUTPUT);

    // Setup routes
    server.on("/", handleRoot);
    
    server.begin();

    Serial.println("Ready");
}

void loop() {
    server.handleClient();
    // Check people movement based on sensor inputs
    checkPeopleMovement();
}

void handleRoot() {
    // Serve HTML page with count information and controls
    server.send(200, "text/html", getPage());
}

String getPage() {
    // Generate HTML page with count information and controls
    String page = "<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Smart People Counter</title>";
    page += "<style>body { font-family: 'Arial', sans-serif; margin: 0; padding: 0; background-image: url('https://sanjivani.org.in/wp-content/uploads/2023/05/000.jpg'); background-repeat: no-repeat; background-size: cover; background-position: center center; display: flex; justify-content: center; align-items: center;}";
    page += ".container { max-width: 600px; margin: 50px auto; background-color: rgba(255, 255, 255, 0.9); backdrop-filter: blur(5px); padding: 20px; border-radius: 15px; box-shadow: 0 0 20px rgba(0,0,0,0.1); }";
    page += "h1 { color: #333366; border-bottom: 3px solid #007bff; padding-bottom: 20px; margin-bottom: 30px; text-align: center; }";
    page += "h2 { color: #555555; margin-top: 30px; text-align: center; }";
    page += ".status { font-size: 28px; text-align: center; display: block; margin-bottom: 30px; }";
    page += ".btn { display: inline-block; padding: 12px 25px; margin: 15px; font-size: 20px; ";
    page += "border: none; border-radius: 10px; cursor: pointer; transition: background-color 0.3s; background-color: #007bff; color: white; }";
    page += ".btn:hover { background-color: #0056b3; }";
    page += ".member { display: inline-block; padding: 10px 20px; margin: 15px; border-radius: 10px; background-color: #e9e9e9; text-align: center; box-shadow: 0 0 10px rgba(0,0,0,0.05); }";
    page += ".logo { text-align: center; margin-bottom: 30px; }";
    page += ".counters { display: grid; grid-template-columns: repeat(3, 1fr); gap: 20px; text-align: center; }";
    page += ".heading { font-size: 32px; color: #333366; text-align: center; margin-bottom: 30px; }";
    page += ".on { color: red; }";
    page += ".off { color: black; }";
    page += "</style></head><body>";
    page += "<div class='container'>";
    page += "<h1>Smart Lock System:Human Detection</h1>";
    
    // Heading for Counters section
    page += "<h2>People Counter</h2>";
    
    // Counters section
    page += "<div class='counters'>";
    page += "<div><h2>In Room Count</h2><span class='status'>" + String(inRoomCount) + "</span></div>";
    page += "<div><h2>Entering Count</h2><span class='status'>" + String(enteringCount) + "</span></div>";
    page += "<div><h2>Exiting Count</h2><span class='status'>" + String(exitingCount) + "</span></div>";
    page += "</div>";

    // LED and Fan status
    page += "<h2>Device Status</h2>";
    page += "<div class='status'><strong>LED:</strong> ";
    if(digitalRead(ledPin) == HIGH) {
        page += "<span class='on'>On</span>";
    } else {
        page += "<span class='off'>Off</span>";
    }
    page += "</div>";
    page += "<div class='status'><strong>FAN:</strong> ";
    if(digitalRead(fanPin) == HIGH) {
        page += "<span class='on'>On</span>";
    } else {
        page += "<span class='off'>Off</span>";
    }
    page += "</div>";

    // Group members
    page += "<h2>Group Members</h2>";
    page += "<div class='member'><strong> 1:</strong> Rutuja mali </div>";
    page += "<div class='member'><strong> 2:</strong> Ishwari Kape </div>";
    page += "<div class='member'><strong> 3:</strong> Aditi Bairagi </div>";
    page += "<div class='member'><strong> 4:</strong> Sanket Tambe </div>";
    
    page += "<script>function refreshPage() { location.reload(); }</script>";
    page += "<script>setInterval(refreshPage, 1000);</script>";  // Auto-refresh every 1 second
    page += "</div></body></html>";
    return page;
}

void checkPeopleMovement() {
    // Read current sensor states
    bool newSensor1State = digitalRead(sensorPin1);
    bool newSensor2State = digitalRead(sensorPin2);

    // Check for entering
    if (sensor2State == LOW && sensor1State == HIGH && newSensor1State == LOW) {
        enteringCount++;
    }

    // Check for exiting
    if (sensor1State == LOW && sensor2State == HIGH && newSensor2State == LOW) {
        if (enteringCount && inRoomCount > 0) {
            exitingCount++;
        }
    }

    // Update sensor states
    sensor1State = newSensor1State;
    sensor2State = newSensor2State;

    // Update inRoomCount
    inRoomCount = max(enteringCount - exitingCount, 0);

    // Control LED and Fan based on inRoomCount
    if (inRoomCount > 0) {
        digitalWrite(ledPin, HIGH);  // Turn on LED
        digitalWrite(fanPin, HIGH);  // Turn on Fan
    } else {
        digitalWrite(ledPin, LOW);   // Turn off LED
        digitalWrite(fanPin, LOW);   // Turn off Fan
    }

    // Read touch sensor state
    bool touchState = digitalRead(touchSensorPin);

    // Debugging: Print touch sensor state
    //Serial.print("Touch sensor state: ");
    //Serial.println(touchState);

    // If touch sensor is triggered (HIGH), open the latch
    if (touchState == HIGH) {
        openLatch();
        delay(2000); // Keep the latch open for 5 seconds
        closeLatch();
    }
}

void openLatch() {
    digitalWrite(relayPin, LOW); // Activate relay to open latch
}

void closeLatch() {
    digitalWrite(relayPin, HIGH); // Deactivate relay to close latch
}
