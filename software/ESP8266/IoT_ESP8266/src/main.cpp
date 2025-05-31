#include <WiFiClient.h>

void setup() {
  Serial.begin(115200);
  Serial.println("start project!");
}

void loop() {
  Serial.println("Im alive MF");
  delay(500);
}