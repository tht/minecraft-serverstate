
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

int read_length(TCPClient *client);
void query_server(char *server, uint16_t port);

#endif
