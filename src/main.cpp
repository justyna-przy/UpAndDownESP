// Starter code for ESP32

#include <Arduino.h>

#define LED_PIN 2  

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("ESP32 LED blink test starting...");

    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);  // 500 ms
    digitalWrite(LED_PIN, LOW);
    delay(500);
}
