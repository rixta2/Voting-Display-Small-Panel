#include <WiFi.h>
#include <WebSocketsClient.h>
#include <FastLED.h>
#include <string>
#include <iostream>


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
String factionName = FACTION_NAME;
String wsPath = "/score_ws/" + factionName; 
//+ "/timed";
const uint16_t websocket_port = 80;
unsigned long lastLoop = 0;
int currentDigits[4] = {-1, -1, -1, -1};  // -1 means nothing shown

WebSocketsClient webSocket;
String receivedScore = "";

#define BRIGHTNESS  100
#define NUM_LEDS    30

#define PANEL1_PIN 33  // Thousands
#define PANEL2_PIN 32  // Hundreds
#define PANEL3_PIN 22  // Tens
#define PANEL4_PIN 21   // Units

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

int previousScore = 0;

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

void displayDigit(int digit, CRGB leds[]) {
    if (digit >= 0 && digit <= 9) {
      for (int segment = 0; segment < 7; segment++) {
        for (int i = 0; i < 4; i++) {
          leds[segmentMap[segment][i]] = numbers[digit][segment] ? LED_COLOR : CRGB::Black;
        }
      }
    } else {
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
    }

  }

  void updateDisplay(int score_int) {
    Serial.print("Inside update display");
    std::string score = std::to_string(score_int);
    // if (score.length() > 4) {
    //     score = score.substring(score.length() - 4);
    // }

    //while (score.length() < 4) {
    //   score = "0" + score;
    //}

    //bool leadingZero = true;

    for (size_t i = 0; i < score.length(); i++) {
        //int digit = score[i] - '0';

        // if (digit != 0 || i == 3) {
        //     leadingZero = false;
        // }

        int displayDigitValue = score[i] - '0';
        Serial.print("Display digit value: ");
        Serial.println(displayDigitValue);
        Serial.print("score[i]: ");
        Serial.println(score[i]);

        currentDigits[i] = displayDigitValue;
        CRGB* targetStrip = (i == 0) ? leds1 : (i == 1) ? leds2 : (i == 2) ? leds3 : leds4;
        displayDigit(displayDigitValue, targetStrip);
        
    }

    FastLED.show();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            Serial.println("[INFO] Connected to WebSocket server");
            break;

        case WStype_TEXT: {
            Serial.println("[DEBUG] Received raw WebSocket payload:");
            for (size_t i = 0; i < length; i++) {
                Serial.print((char)payload[i]);
            }
            Serial.println();

            receivedScore = String((char*)payload);
            Serial.print("[INFO] Received Score: ");
            Serial.println(receivedScore);

            int score = receivedScore.toInt();
            if (score >= 0 && score <= 9999 && score != previousScore) {  
                // int scoreTest = 53;
                updateDisplay(score);  
                previousScore = score;
                Serial.print("[INFO] Updated display to score: ");
                Serial.println(score);
            } else {
                Serial.println("[WARNING] Invalid score received, ignoring...");
            }
            break;
        }
        case WStype_PONG:
        case WStype_PING:
            break;

        case WStype_DISCONNECTED:
            Serial.println("[ERROR] Disconnected from WebSocket server!");
            break;

        default:
            Serial.print("[DEBUG] Unhandled WebSocket event type: ");
            Serial.println(type);
            break;
    }
}

void connectWiFi() {
    Serial.print("[INFO] Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[INFO] WiFi Connected!");
}

void connectWebSocket() {
    Serial.println("[INFO] Connecting to WebSocket server...");
    webSocket.begin(websocket_server, websocket_port, wsPath);
    webSocket.enableHeartbeat(15000, 3000, 2); //is this necessary?
    try {
      webSocket.onEvent(webSocketEvent);
    } catch (const std::exception& e) {
        Serial.print("[ERROR] WebSocket event handler error: ");
        Serial.println(e.what());
    }
    webSocket.setReconnectInterval(5000); //how does this work?
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

    connectWiFi();
    WiFi.setSleep(false);
    connectWebSocket();
}

void loop() {

    webSocket.loop();
    
    if (millis() - lastLoop > 10000) {
        lastLoop = millis();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[ERROR] Lost WiFi connection! Attempting reconnect...");
            connectWiFi();
        }
    
        // Check if WebSocket is still connected
        if (!webSocket.isConnected()) {
            Serial.println("[ERROR] Lost WebSocket connection! Attempting reconnect...");
            connectWebSocket();
        }
    }
}
