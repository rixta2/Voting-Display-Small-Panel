#include <WiFi.h>

// Replace these with your actual credentials
const char* ssid = "gusmanor";
const char* password = "Bigboygus";

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries++;
    if (retries > 20) {
      Serial.println("\n[ERROR] Failed to connect to WiFi.");
      return;
    }
  }

  Serial.println("\n[INFO] WiFi connected!");
  Serial.print("[INFO] IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Nothing needed here for this test
}