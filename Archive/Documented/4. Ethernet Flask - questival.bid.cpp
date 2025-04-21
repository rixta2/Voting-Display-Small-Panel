#include <SPI.h>
#include <Ethernet_Generic.h>
#include <ArduinoJson.h>
#include <FastLED.h>

// --- Ethernet Setup ---
#define ETH_CS 5
byte mac[] = { 0xF8, 0xB3, 0xB7, 0x2B, 0x1E, 0x2C };
IPAddress ip(192, 168, 1, 178);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// --- API ---
const char* server = "questival.bid";
const char* apiPath = "/score/nocturne";
//const char* apiKey = "qrIL8SMfv2sKznqyZxRZDjWqvqCiPV9NNTaKnVnm308"; - not included in this API

// --- LED Setup ---
#define NUM_LEDS 30
#define BRIGHTNESS 128
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PANEL_PIN1 4
#define PANEL_PIN2 25
#define PANEL_PIN3 26
#define PANEL_PIN4 27

CRGB panel1[NUM_LEDS];
CRGB panel2[NUM_LEDS];
CRGB panel3[NUM_LEDS];
CRGB panel4[NUM_LEDS];

const int segmentMap[7][4] = {
  {1, 2, 3, 4},  
  {5, 6, 7, 8}, 
  {9, 10, 11, 12},
  {14, 15, 16, 17}, 
  {18, 19, 20, 21}, 
  {22, 23, 24, 25}, 
  {26, 27, 28, 29}
};

const byte numbers[10][7] = {
  {1,1,1,1,1,1,0}, 
  {0,0,1,1,0,0,0}, 
  {0,1,1,0,1,1,1}, 
  {0,1,1,1,1,0,1},
  {1,0,1,1,0,0,1}, 
  {1,1,0,1,1,0,1}, 
  {1,1,0,1,1,1,1}, 
  {0,1,1,1,0,0,0},
  {1,1,1,1,1,1,1}, 
  {1,1,1,1,1,0,1}
};

void clearDisplay() {
  fill_solid(panel1, NUM_LEDS, CRGB::Black);
  fill_solid(panel2, NUM_LEDS, CRGB::Black);
  fill_solid(panel3, NUM_LEDS, CRGB::Black);
  fill_solid(panel4, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void displayDigit(int digit, CRGB* strip) {
  fill_solid(strip, NUM_LEDS, CRGB::Black);
  if (digit >= 0 && digit <= 9) {
    for (int s = 0; s < 7; s++) {
      if (numbers[digit][s]) {
        for (int i = 0; i < 4; i++) {
          int ledIndex = segmentMap[s][i];
          if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
            strip[ledIndex] = CRGB::Green;
          }
        }
      }
    }
  }
  FastLED.show();
}

void updateDisplay(const String& score) {
  String padded = score;
  while (padded.length() < 4) padded = "0" + padded;
  displayDigit(padded[0] - '0', panel1);
  displayDigit(padded[1] - '0', panel2);
  displayDigit(padded[2] - '0', panel3);
  displayDigit(padded[3] - '0', panel4);
}

void fetchScore() {
  EthernetClient client;

  if (client.connect(server, 80)) {
    Serial.println("[HTTP] Connected to server");

    // Send HTTP GET request
    client.println("GET " + String(apiPath) + " HTTP/1.1");
    client.println("Host: " + String(server));
    //client.println("Authorization: " + String(apiKey)); - not included in this API
    client.println("Connection: close");
    client.println();

    // Wait for response
    String response = "";
    while (client.connected() || client.available()) {
      if (client.available()) {
        response += client.readStringUntil('\n');
      }
    }
    client.stop();

    // Parse HTTP response
    int jsonStart = response.indexOf('{');
    int jsonEnd = response.lastIndexOf('}');
    if (jsonStart != -1 && jsonEnd != -1) {
      String jsonString = response.substring(jsonStart, jsonEnd + 1);
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, jsonString);

      if (!error && doc.containsKey("score")) {
        int score = doc["score"];
        updateDisplay(String(score));
      } else {
        Serial.println("[ERROR] Failed to parse JSON or 'score' not found.");
      }
    } else {
      Serial.println("[ERROR] Invalid HTTP response");
    }
  } else {
    Serial.println("[HTTP] Connection failed");
  }
}

void setup() {
  Serial.begin(115200);

  Ethernet.init(ETH_CS);
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  delay(1000);

  Serial.print("[INFO] IP Address: ");
  Serial.println(Ethernet.localIP());

  FastLED.addLeds<LED_TYPE, PANEL_PIN1, COLOR_ORDER>(panel1, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, PANEL_PIN2, COLOR_ORDER>(panel2, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, PANEL_PIN3, COLOR_ORDER>(panel3, NUM_LEDS);
  FastLED.addLeds<LED_TYPE, PANEL_PIN4, COLOR_ORDER>(panel4, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  clearDisplay();

  fetchScore();  // Initial fetch on boot
}

void loop() {
  static unsigned long lastFetch = 0;
  if (millis() - lastFetch > 10000) {  // fetch every 10 seconds
    fetchScore();
    lastFetch = millis();
  }
}