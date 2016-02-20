
#ifndef server_state_h
#define server_state_h

#include "application.h"

enum server_state_t {
  STAT_STARTING,
  STAT_UNKNOWN,
  STAT_OFFLINE,
  STAT_NOPLAYERS,
  STAT_SOMEPLAYERS
};

extern enum server_state_t server_stat;
extern uint8_t server_players;

class ServerState {

public:
  ServerState(char* _name, uint16_t _port) { set_server(_name, _port); }
  void set_server(char* _name, uint16_t _port);
  void query_server();

private:
  char *server_name;
  uint16_t server_port;

  uint16_t players_max = 0;
  uint16_t players_onl = 0;

  enum server_state_t server_stat;
  uint8_t server_players;

  int read_length(TCPClient *client);
  int read_int(char *buffer, TCPClient *client, int *ri);
};



#endif
