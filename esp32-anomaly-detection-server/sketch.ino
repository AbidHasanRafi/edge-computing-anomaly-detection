/*
  This project is part of an ongoing research initiative.
  To extend, use, or adapt the resources provided here, please contact the author.
  © 2025 - Abid Hasan Rafi. All rights reserved.
  Email: ahr16.abidhasanrafi@gmail.com
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_SSD1306.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

WebServer server(80);

const int BLUE_LED = 26;
const int GREEN_LED = 27;
const int RED_LED = 14;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool blueLedState = false;
bool redLedState = false;

// Anomaly Data
int anomalyCount = 0;
unsigned long lastAnomalyTime = 0;
const String serverUrl = "your_api"; // Add Api for POST Request

// Simulate normal vs anomaly behavior
bool isAnomaly(int powerReading) {
  // Anomalies happen randomly (30% of the time for demo)
  return random(0, 100) < 30; 
}

void sendHtml() {
  String response = R"(
    <!DOCTYPE html><html>
      <head>
        <title>ESP32 Web Server Demo</title>
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <style>
          html { font-family: sans-serif; text-align: center; }
          body { display: inline-flex; flex-direction: column; }
          h1 { margin-bottom: 1.2em; } 
          h2 { margin: 0; }
          div { display: grid; grid-template-columns: 1fr 1fr; grid-template-rows: auto auto; grid-auto-flow: column; grid-gap: 1em; }
          .btn { background-color: #5B5; border: none; color: #fff; padding: 0.5em 1em;
                 font-size: 2em; text-decoration: none }
          .btn.OFF { background-color: #333; }
        </style>
      </head>
      <body>
        <h1>ESP32 Web Server</h1>
        <div>
          <h2>LED 1</h2>
          <a href="/toggle/1" class="btn LED1_TEXT">LED1_TEXT</a>
          <h2>LED 2</h2>
          <a href="/toggle/2" class="btn LED2_TEXT">LED2_TEXT</a>
        </div>
      </body>
    </html>
  )";
  response.replace("LED1_TEXT", blueLedState ? "ON" : "OFF");
  response.replace("LED2_TEXT", redLedState ? "ON" : "OFF");
  server.send(200, "text/html", response);
}

void displayReading(int powerReading, bool anomalyDetected) {
  display.clearDisplay();
  display.setTextSize(1);      
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(0,0);     
  display.print("Power Reading: ");
  display.print(powerReading);

  display.setCursor(0,10);  
  display.print(anomalyDetected ? "Anomaly Detected!" : "Normal Reading");

  display.display();
}

void sendAnomalyData(int powerReading, bool anomalyDetected) {
  HTTPClient http;
  http.begin(serverUrl);

  // Prepare JSON data to send
  DynamicJsonDocument doc(1024);
  doc["meterId"] = "MT-0001";
  doc["time"] = millis();
  doc["anomalousPowerReading"] = powerReading;
  doc["isAnomaly"] = anomalyDetected ? "true" : "false";
  
  String jsonData;
  serializeJson(doc, jsonData);

  // Send POST request
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonData);

  if (httpResponseCode > 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
  }

  http.end();
}

void setup(void) {
  Serial.begin(115200);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);  // Pause for 2 seconds for the display to initialize

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);
  Serial.print("Connecting to WiFi ");
  Serial.print(WIFI_SSID);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", sendHtml);

  server.on(UriBraces("/toggle/{}"), []() {
    String led = server.pathArg(0);
    Serial.print("Toggle LED #");
    Serial.println(led);

    switch (led.toInt()) {
      case 1:
        blueLedState = !blueLedState;
        digitalWrite(BLUE_LED, blueLedState);
        break;
      case 2:
        redLedState = !redLedState;
        digitalWrite(RED_LED, redLedState);
        break;
    }

    sendHtml();
  });

  server.begin();
  Serial.println("HTTP server started");

  // Start streaming power readings and detecting anomalies
  unsigned long startTime = millis();
  while (millis() - startTime < 120000 && anomalyCount < 10) { // 2 minutes or 10 anomalies
    int powerReading = random(500, 1000); // Random power reading
    bool anomalyDetected = isAnomaly(powerReading);
    anomalyCount += anomalyDetected ? 1 : 0;

    // Turn on the respective LED
    if (anomalyDetected) {
      digitalWrite(BLUE_LED, HIGH); // Blue LED on for anomaly
      digitalWrite(GREEN_LED, LOW); // Green LED off during anomaly
    } else {
      digitalWrite(GREEN_LED, HIGH); // Green LED on for normal reading
      digitalWrite(BLUE_LED, LOW); // Blue LED off during normal reading
    }

    sendAnomalyData(powerReading, anomalyDetected); // Send anomaly data to server
    displayReading(powerReading, anomalyDetected);  // Display power readings on OLED
    
    delay(10000); // Wait for 10 seconds before sending the next data point
  }

  // After 2 minutes or 10 anomalies, turn off both LEDs
  digitalWrite(BLUE_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
}

void loop(void) {
  server.handleClient();
  delay(2);
}
