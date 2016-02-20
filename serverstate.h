
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

  ServerState() {
    // Register status variables
    Particle.variable("PlayersOnl", players_onl);
    Particle.variable("PlayersMax", players_max);
    Particle.variable("Desc", "(not known yet)"); // Will be overwritten later
  }
  ServerState(char* _name, uint16_t _port) { set_server(_name, _port); }
  void set_server(char* _name, uint16_t _port);
  void query_server();
  bool configured() {return server_name != 0;}

  int get_state() {return server_state;}
  int get_players_online() {return players_onl;}
  int get_max_players() {return players_max;}

  // State
  int32_t players_max = 0;
  int32_t players_onl = 0;
  char *description;
  enum server_state_t server_state;

private:
  // Configuration
  char *server_name;
  uint16_t server_port;

  int read_length(TCPClient *client);
  int read_int(char *buffer, TCPClient *client, int *ri);
};



#endif
