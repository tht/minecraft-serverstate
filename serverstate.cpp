#include "serverstate.h"

void ServerState::set_server(char* _name, uint16_t _port) {
  if (server_name) free(server_name);

  server_name = (char*) malloc(strlen(_name)+1);
  strcpy(server_name, _name);

  server_port = _port;
}

int ServerState::read_length(TCPClient *client) {
  // decode packet length
  int res = 0;
  int i = 0;
  while(true) {
    while(!client->available()) ;
    int c = client->read();
    if (!c) return 0;
    res |= (c & 0x7F) << i++ * 7;
    if(i > 5) return 0;
    if(! (c & 0x80)) break;
  }
  return res;
}

int ServerState::read_int(char *buffer, TCPClient *client, int *ri) {
  int bi = 0;
  char c;
  do {
    while(!client->available()) ;
    c = client->read(); (*ri)++;
    buffer[bi++] = c;
  } while (c >= '0' && c <= '9');
  buffer[bi] = 0;
  return atoi(buffer);
}

void ServerState::query_server() {
  if (!server_name) {
    Serial.println("No server configured, aborting query.");
    return;
  }
  TCPClient client;

  Serial.print("Connecting to "); Serial.print(server_name); Serial.println("...");
  if(client.connect(server_name, server_port)) {
    // Prepare IP as string as we need it for the handshake
    IPAddress remote = client.remoteIP();
    String remoteIP = String::format("%d.%d.%d.%d", remote[0], remote[1], remote[2], remote[3]);

    // First byte of IP should not be 0, abort if it is
    if (! remote[0]) {
      Serial.print("Remote IP reported as: "); Serial.println(remoteIP);
      Serial.println("Aborting");
      client.stop();
      server_state = STAT_UNKNOWN;
      return;
    }
    Serial.print("Connected to: "); Serial.println(remoteIP);

    // Send length in strange hexdec format, packet ID, length and IP as string
    byte mod10 = remoteIP.length() % 10;
    byte hexLength = ((remoteIP.length()-mod10) / 10) << 4 | mod10;
    client.write(hexLength); // length
    client.write((byte)0x00); client.write(0x04);
    client.write(remoteIP.length());
    client.print(remoteIP);

    // Now send port number and end of handshake
    client.write((byte) (server_port >> 8)); client.write((byte) server_port & 0xFF); // server port
    client.write(0x01); client.write(0x01); client.write((byte)0x00);

    // discard one byte
    while(!client.available()) ;
    client.read(); // dropping 0xF6

    int l1 = read_length(&client);
    if (l1 < 10) {
      Serial.print("Length of first result packet is only ");
      Serial.print(l1); Serial.println(" Bytes - aborting");

    } else {
      // looks good, continue parsing result
      while(!client.available()) ;
      client.read(); // dropping 0xF6

      int l2 = read_length(&client);
      Serial.print("Length of JSON response: "); Serial.println(l2);

      char buffer[32];
      bool got_data = false;
      int ri = 0; // ri=reader_index, bi=buffer_index
      for (int bi=0; ri<l2; ri++) {
        while(!client.available()) ;
        char c = client.read();

        if (c == '"') {
          // check if we found something which looks like a string
          if (bi) {
            buffer[bi] = 0;

            if (strcmp(buffer, "online") == 0) {
              while(!client.available()) ;
              client.read(); ri++; // discard ':'

              players_onl = read_int(buffer, &client, &ri);
              Serial.print("Players online: "); Serial.println(players_onl);
              got_data = true;

            } else if (strcmp(buffer, "max") == 0) {
              while(!client.available()) ;
              client.read(); ri++; // discard ':'

              players_max = read_int(buffer, &client, &ri);
              Serial.print("Max players: "); Serial.println(players_max);

            } else if (strcmp(buffer, "description") == 0) {
              while(!client.available()) ;
              client.read(); ri++; // discard ':'

              while(!client.available()) ;
              client.read(); ri++; // discard '"'

              // Get value
              bi = 0;
              do {
                while(!client.available()) ;
                c = client.read(); ri++;
                buffer[bi++] = c;
              } while (c && c != '"');
              buffer[bi-1] = 0;

              // Copy description to class
              if (description) free(description);
              description = (char*) malloc(strlen(buffer)+1);
              strcpy(description, buffer);

              Serial.print("Server Description: "); Serial.println(description);
            }
          }
          bi = 0;
        } else {
          if (bi > 31) bi = 0;
          else buffer[bi++] = c;
        }
      }

      // handle result if there is something
      if (! got_data) {
        server_state = STAT_UNKNOWN;
      }
    }

    Serial.println("Disconnecting...");
    client.stop();

  } else {
    Serial.println("Connection failed :(");
    server_state = STAT_OFFLINE;
  }
}
