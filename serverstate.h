
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

  ServerState() { }
  ServerState(char* _name, uint16_t _port) { set_server(_name, _port); }
  void set_server(char* _name, uint16_t _port);
  void query_server();
  bool configured() {return server_name != 0;}

  int get_state() {return server_state;}
  int get_players_online() {return players_onl;}
  int get_max_players() {return players_max;}

private:
  // Configuration
  char *server_name;
  uint16_t server_port;

  // State
  uint16_t players_max = 0;
  uint16_t players_onl = 0;
  char *description;
  enum server_state_t server_state;

  int read_length(TCPClient *client);
  int read_int(char *buffer, TCPClient *client, int *ri);
};



#endif
