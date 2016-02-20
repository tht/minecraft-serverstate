#include "serverstate.h"

enum server_state_t server_stat;
uint8_t server_players = 0;

int read_length(TCPClient *client) {
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

void query_server(char *server, uint16_t port) {
    TCPClient client;

    Serial.print("Connecting to "); Serial.print(server); Serial.println("...");
    if(client.connect(server, port)) {
        // Prepare IP as string as we need it for the handshake
        IPAddress remote = client.remoteIP();
        String remoteIP = String::format("%d.%d.%d.%d", remote[0], remote[1], remote[2], remote[3]);

        // First byte of IP should not be 0, abort if it is
        if (! remote[0]) {
            Serial.print("Remote IP reported as: "); Serial.println(remoteIP);
            Serial.println("Aborting");
            client.stop();
            server_stat = STAT_UNKNOWN;
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
        client.write((byte) (port >> 8)); client.write((byte) port & 0xFF); // server port
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

            /*
            // TODO: Should add a small buffer here as we're only interested in online players
            char * buffer = (char*)malloc(l2);
            for (int i=0; i<l2 && client.connected(); i++) {
                while(!client.available()) ;
                buffer[i] = client.read();
            }
            buffer[l2] = 0x00;
            //Serial.println(buffer);


            char *online = strstr(buffer, "\"online\":") + strlen("\"online\":");
            if (online != NULL) {
                server_players = atoi(online);
                if (server_players) {
                    server_stat = STAT_SOMEPLAYERS;
                } else {
                    server_stat = STAT_NOPLAYERS;
                }
                Serial.print("Number of players online: "); Serial.println(server_players);
            } else {
                Serial.println("Unable to locate player count in response from server.");
            }

            */

            char buffer[32];
            int tmp_players = -1;
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

                            // Get value
                            bi = 0;
                            do {
                                while(!client.available()) ;
                                c = client.read(); ri++;
                                buffer[bi++] = c;
                            } while (c >= '0' && c <= '9');
                            buffer[bi] = 0;
                            tmp_players = atoi(buffer);
                            Serial.print("Players online: "); Serial.println(tmp_players);

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
                            Serial.print("Server Description: "); Serial.println(buffer);
                        }
                    }
                    bi = 0;

                } else { // continue reading data, reset buffer_index if needed
                    if (bi > 31) bi = 0;
                    else buffer[bi++] = c;
                }
            }

            // handle result if there is something
            if (tmp_players != -1) {
                server_players = tmp_players;
                if (server_players) server_stat = STAT_SOMEPLAYERS;
                else server_stat = STAT_NOPLAYERS;
            }
        }

        Serial.println("Disconnecting...");
        client.stop();

    } else {
        Serial.println("Connection failed :(");
        server_stat = STAT_OFFLINE;
    }
}
