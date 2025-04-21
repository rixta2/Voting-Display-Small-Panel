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

const char* ssid = "Gusmanor";
const char* password = "Bigboygus";

String websocket_server = SERVER;
String factionName = FACTION_NAME;
String wsPath = "/score_ws/" + factionName;
const uint16_t websocket_port = 80;


WebSocketsClient webSocket;
String receivedScore = "0000";

#define BRIGHTNESS  100
#define NUM_LEDS    29  

#define PANEL1_PIN 25  // Thousands
#define PANEL2_PIN 26  // Hundreds
#define PANEL3_PIN 27  // Tens
#define PANEL4_PIN 5   // Units

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

// 7-segment LED mapping (LEDs 0 and 13 do not make up segments)
const int segmentMap[7][4] = {
  { 1, 2, 3, 4 },   // Top left
  { 5, 6, 7, 8 },   // Top
  { 9, 10, 11, 12 }, // Top Right
  { 14, 15, 16, 17 }, // Bottom Right
  { 18, 19, 20, 21 }, // Bottom
  { 22, 23, 24, 25 }, // Bottom Left
  { 26, 27, 28, 29 }  // Middle
};

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

// Currently shows leading zeros, may change.
void displayDigit(int digit, CRGB leds[]) {

  if (digit >= 0 && digit <= 9) {
      for (int segment = 0; segment < 7; segment++) {
          if (numbers[digit][segment]) {
              for (int i = 0; i < 4; i++) {
                  leds[segmentMap[segment][i]] = CRGB::Green;
              }
          } else {
              for (int i = 0; i < 4; i++) {
                  leds[segmentMap[segment][i]] = CRGB::Black;
              }
          }
      }
  }

  FastLED.show();
}

void updateDisplay(String score) {
  while (score.length() < 4) score = "0" + score; 

  displayDigit(score[0] - '0', leds1);  // Thousands
  displayDigit(score[1] - '0', leds2);  // Hundreds
  displayDigit(score[2] - '0', leds3);  // Tens
  displayDigit(score[3] - '0', leds4);  // Units
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("[INFO] Connected to WebSocket server");
            break;

        case WStype_TEXT: {
            receivedScore = String((char*)payload);
            Serial.print("[INFO] Received Score: ");
            Serial.println(receivedScore);

            // Ensure valid 4-digit number
            int score = receivedScore.toInt();
            if (score >= 0 && score <= 9999) {  
                updateDisplay(String(score));  
            }
            break;
        }

        case WStype_DISCONNECTED:
            Serial.println("[ERROR] Disconnected from WebSocket server!");
            break;
    }
}

void setup() {
    Serial.begin(115200);

    FastLED.addLeds<WS2812B, PANEL1_PIN, GRB>(leds1, NUM_LEDS);
    FastLED.addLeds<WS2812B, PANEL2_PIN, GRB>(leds2, NUM_LEDS);
    FastLED.addLeds<WS2812B, PANEL3_PIN, GRB>(leds3, NUM_LEDS);
    FastLED.addLeds<WS2812B, PANEL4_PIN, GRB>(leds4, NUM_LEDS);
    
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.clear();
    FastLED.show();

    Serial.print("[INFO] Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[INFO] WiFi Connected!");

    Serial.println("[INFO] Connecting to WebSocket server...");
    webSocket.begin(websocket_server, websocket_port, wsPath);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void loop() {
    webSocket.loop();
    delay(100);
}