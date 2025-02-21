#include <WiFi.h>
#include <WebSocketsClient.h>
#include <FastLED.h>

#ifndef WIFI_SSID
  #define WIFI_SSID "Unknown_SSID"
#endif

#ifndef WIFI_PASSWORD
  #define WIFI_PASSWORD "Unknown_Password"
#endif

#ifndef FACTION_NAME
  #define FACTION_NAME "Unknown"
#endif

#ifndef SERVER
  #define SERVER "Unknown"
#endif

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

String websocket_server = SERVER;
String teamName = FACTION_NAME;
String wsPath = "/score_ws/" + teamName;
const uint16_t websocket_port = 80;

// WebSocket Client
WebSocketsClient webSocket;
String receivedScore = "";

// LED Matrix Configuration
#define LED_PIN     5 
#define NUM_LEDS    29
#define BRIGHTNESS  100

CRGB leds[NUM_LEDS];

// 7-segment LED mapping (excluding filler LEDs 0 and 13)
const int segmentMap[7][4] = {
  { 1, 2, 3, 4 },   // Top left
  { 5, 6, 7, 8 },   // Top
  { 9, 10, 11, 12 }, // Top Right
  { 14, 15, 16, 17 }, // Bottom Right
  { 18, 19, 20, 21 }, // Bottom
  { 22, 23, 24, 25 }, // Bottom Left
  { 26, 27, 28, 29 }  // Middle
};

// Digit-to-segment mapping
const byte numbers[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, // 0
  {0, 0, 1, 1, 0, 0, 0}, // 1
  {0, 1, 1, 0, 1, 1, 1}, // 2
  {0, 1, 1, 1, 1, 0, 1}, // 3
  {1, 0, 1, 1, 0, 0, 1}, // 4
  {1, 1, 0, 1, 1, 0, 1}, // 5
  {1, 1, 0, 1, 1, 1, 1}, // 6
  {0, 1, 1, 1, 0, 0, 0}, // 7
  {1, 1, 1, 1, 1, 1, 1}, // 8
  {1, 1, 1, 1, 1, 0, 1}  // 9
};

// Function to display a single digit on the LED matrix
void displayDigit(int digit) {
  FastLED.clear();
  
  if (digit >= 0 && digit <= 9) {
    for (int segment = 0; segment < 7; segment++) {
      if (numbers[digit][segment]) {
        for (int i = 0; i < 4; i++) {
          leds[segmentMap[segment][i]] = CRGB::Green;  // Set LED to Green
        }
      }
    }
  }

  FastLED.show();
}

// WebSocket Event Handler
// WebSocket Event Handler
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
      case WStype_CONNECTED:
          Serial.println("[INFO] Connected to WebSocket server");
          break;

      case WStype_TEXT: {  // Added curly braces
          receivedScore = String((char*)payload);
          Serial.print("[INFO] Received Score: ");
          Serial.println(receivedScore);

          // Convert score to an integer and update LED display
          int score = receivedScore.toInt();
          if (score >= 0 && score <= 9) {  
              displayDigit(score);  
          }
          break;
      } // Closing brace for the case block

      case WStype_DISCONNECTED:
          Serial.println("[ERROR] Disconnected from WebSocket server!");
          break;
  }
}


void setup() {
    Serial.begin(115200);

    // Initialize LED Matrix
    FastLED.addLeds<WS2812B, LED_PIN, RGB>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    // Connect to WiFi
    Serial.print("[INFO] Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[INFO] WiFi Connected!");

    // Connect to WebSocket Server
    Serial.println("[INFO] Connecting to WebSocket server...");
    webSocket.begin(websocket_server, websocket_port, wsPath);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000); // Try reconnecting every 5 seconds
}

void loop() {
    webSocket.loop();
    delay(100);
}
