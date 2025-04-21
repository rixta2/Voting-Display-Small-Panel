#include <SPI.h>
#include <Ethernet_Generic.h>  // Make sure you've installed this library

// Update MAC address and optionally the IP
byte mac[] = { 0xF8, 0xB3, 0xB7, 0x2B, 0x1E, 0x2C };
IPAddress ip(192, 168, 1, 178);  // Optional static IP


EthernetClient client;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 Ethernet_Generic Hello World");

  // Start SPI manually with correct pins if needed
  // SPI.begin(SCK, MISO, MOSI, SS);  // Only needed if not using default SPI

  // Initialize Ethernet with static IP
  Ethernet.init(5); // CS pin - set to match your wiring!
  Ethernet.begin(mac, ip, dns);

  delay(1000);

  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

  // Optional: ping a site
  if (client.connect("example.com", 80)) {
    Serial.println("Connected to server!");
    client.println("GET / HTTP/1.1");
    client.println("Host: example.com");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      while (client.available()) {
        char c = client.read();
        Serial.print(c);
      }
    }
    client.stop();
    Serial.println("\nDisconnected.");
  } else {
    Serial.println("Connection failed.");
  }
}

void loop() {
  // Nothing in loop
}
