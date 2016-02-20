#include "serverstate.h"

#define UPDATE_INTERVAL 30 * 1000 // all 30s

char *server = "example.org";
uint16_t port = 25565;





void setup() {
    Serial.begin(57600);
    Serial.println("Starting up...");

    // Register Variables to Particle Cloud
    Particle.variable("serverName", server);
    Particle.variable("serverPort", port);
}

void loop() {
    static unsigned long lastChecked = 2*UPDATE_INTERVAL;
    static unsigned int numRepeats = 0;

    if (millis() - lastChecked > (unsigned long) UPDATE_INTERVAL) {
        lastChecked = millis();
        Serial.print("Starting check #"); Serial.println(++numRepeats);
        query_server(server, port);
    }
}
