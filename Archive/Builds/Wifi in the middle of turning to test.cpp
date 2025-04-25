#include <WiFi.h>
#include <FastLED.h>
#include <string>
#include <iostream>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Arduino.h>

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
const char* serverUrl = "http://questival.bid/get_score/artificers";

int lastScore = -1;  // Track last displayed score

#define BRIGHTNESS  100
#define NUM_LEDS    30

#define PANEL1_PIN 33  // Thousands
#define PANEL2_PIN 32  // Hundreds
#define PANEL3_PIN 22  // Tens
#define PANEL4_PIN 21  // Units

CRGB leds1[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];

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

int getScoreFromServer() {
  HTTPClient http;
  http.begin(serverUrl);
  
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("[HTTP] Response: " + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print("[ERROR] JSON Parsing Error: ");
      Serial.println(error.c_str());  
      http.end();
      return lastScore;  
    }

    if (!doc.containsKey("score") || !doc["score"].is<int>()) {
      Serial.println("[ERROR] Invalid JSON response, keeping last score.");
      http.end();
      return lastScore;
    }

    int score = doc["score"];  
    Serial.print("[INFO] Fetched Score: ");
    Serial.println(score);

    http.end();
    return score;
  } else {
    Serial.print("[ERROR] HTTP Request failed, code: ");
    Serial.println(httpCode);
    http.end();
    return lastScore;  
  }
}

void displayDigit(int digit, CRGB leds[]) {
  if (digit >= 0 && digit <= 9) {
    for (int segment = 0; segment < 7; segment++) {
      for (int i = 0; i < 4; i++) {
        leds[segmentMap[segment][i]] = numbers[digit][segment] ? CRGB::Red : CRGB::Black;
      }
    }
  } else {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
  }
}

void updateDisplay(int score_int) {
  Serial.println("[INFO] Updating display...");
  std::string score = std::to_string(score_int);

  for (size_t i = 0; i < score.length(); i++) {
    int displayDigitValue = score[i] - '0';
    Serial.print("Display digit value: ");
    Serial.println(displayDigitValue);

    CRGB* targetStrip = (i == 0) ? leds1 : (i == 1) ? leds2 : (i == 2) ? leds3 : leds4;
    displayDigit(displayDigitValue, targetStrip);
  }

  FastLED.show();
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
}

void loop() {
  int newScore = getScoreFromServer();
  delay(1000);
  
  Serial.print("[DEBUG] Last Score: ");
  Serial.print(lastScore);
  Serial.print(" | New Score: ");
  Serial.println(newScore);

  if (newScore != lastScore) {
    Serial.println("[INFO] Score changed. Updating display.");
    lastScore = newScore;
    updateDisplay(newScore); // Fixed to pass the correct score
  } else {
    Serial.println("[INFO] No change in score.");
  }

  delay(2000);
}