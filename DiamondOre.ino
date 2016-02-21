#include "neopixel.h"
#include "serverstate.h"

#define UPDATE_INTERVAL 30 * 1000 // all 30s
#define EEPROM_BASE 0x00
#define EEPROM_HEAD 0x63
#define MAX_SERVER_NAME 32

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D6
#define PIXEL_COUNT 10
#define PIXEL_TYPE WS2812B

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


ServerState server;

struct {
  char server_name[MAX_SERVER_NAME];
  int32_t server_port;
} config = { "", 25565 };
bool config_changed = false;

// Workaround as it seems impossible to use struct member as cloud varibale
int32_t cloud_port = 0;

void apply_config() {
  config_changed = true;
  cloud_port = config.server_port;
  server.set_server(config.server_name, config.server_port);
}


bool read_config() {
  if (EEPROM.read(EEPROM_BASE) != EEPROM_HEAD) {
    Serial.println("No valid configuration in EEPROM.");
    Serial.println("Use exposed functions to set configuration.");
    return false;
  }

  Serial.println("EEPROM has a valid header. Reading configuration.");
  EEPROM.get(EEPROM_BASE+1, config);
  apply_config();
  return true;
}

void store_config() {
  EEPROM.put(EEPROM_BASE+1, config);
  EEPROM.write(EEPROM_BASE, EEPROM_HEAD);
}

int cloud_set_server(String _server) {
  Serial.print("Received new server from Cloud: "); Serial.println(_server);

  int name_length = _server.indexOf(':');
  int tmp_port = 0;

  if (name_length != -1) {
    String port = _server.substring(name_length+1, _server.length());
    Serial.print("Trying to parse port number: "); Serial.println(port);
    tmp_port = port.toInt();
    if (!tmp_port) {
      Serial.println("Parsing port number failed.");
      return -1;
    }
    Serial.print("New port: "); Serial.println(tmp_port, DEC);
  } else {
    Serial.println("No port number supplied, using 25565 (default).");
    tmp_port = 25565;
    name_length = _server.length();
  }

  // Check length of servername
  if (_server.length() > MAX_SERVER_NAME-1) {
    Serial.print("Servername to long. Max is: "); Serial.println(MAX_SERVER_NAME-1);
    return -1;
  }

  // Looks valid - store new port and server name
  config.server_port = tmp_port;
  for (int i=0; i<name_length; i++) {
    config.server_name[i] = _server.charAt(i);
  }
  config.server_name[name_length] = 0;
  print_config();
  store_config();
  apply_config();
}

void print_config() {
  Serial.print("Configured for: "); Serial.print(config.server_name);
  Serial.print(" ("); Serial.print(config.server_port); Serial.println(')');
}

void setup() {
  Serial.begin(57600);
  Serial.println("Starting up...");

  // Init NeoPixel strip
  strip.begin();
  strip.show();

  // Register configuration function and varibales to Particle Cloud
  Particle.function("setServer", cloud_set_server);
  Particle.variable("serverName", config.server_name);
  Particle.variable("serverPort", config.server_port);
  // additional variables are exported inside the ServerState class

  // Try to read configuration from EEPROM
  if (read_config()) {
    Serial.println("Got configuration from EEPROM.");
    print_config();
  }
}

void loop() {
  static unsigned long lastChecked = 0;
  static unsigned int numRepeats = 0;


  // Rainbow animation for testing
  static uint8_t j;
  static unsigned long lastAnimation = 0;
  static uint8_t brightness = 75;

  if (millis() - lastAnimation > 20) {
    lastAnimation = millis(); j++;

    if (brightness < min(server.get_players_online()*255, 255)) {
      brightness += 2;
    } else if (brightness > 77) {
      brightness -= 2;
    }
    strip.setBrightness(brightness);

    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
  }

  if (config_changed || millis() - lastChecked > (unsigned long) UPDATE_INTERVAL) {
    config_changed = false;
    lastChecked = millis();

    if (server.configured()) {
      Serial.print("Starting check #"); Serial.println(++numRepeats);
      server.query_server();
    }
  }
}
