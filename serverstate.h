
#ifndef server_state_h
#define server_state_h

#include "application.h"

class ServerState {

public:

  enum server_state_t {
    STAT_STARTING,
    STAT_UNKNOWN,
    STAT_OFFLINE,
    STAT_ONLINE
  };

  ServerState(char* _name, uint16_t _port) { set_server(_name, _port); }
  void set_server(char* _name, uint16_t _port);
  void query_server();

private:
  char *server_name;
  uint16_t server_port;

  uint16_t players_max = 0;
  uint16_t players_onl = 0;
  char *description;

  enum server_state_t server_state;
  uint8_t server_players;

  int read_length(TCPClient *client);
  int read_int(char *buffer, TCPClient *client, int *ri);
};



#endif
