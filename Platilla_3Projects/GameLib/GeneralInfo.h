#pragma once
#include <SFML\Network.hpp>

struct Coordinates {
	int x;
	int y;
};

enum CommandType {
	EMPTY, HELLO, WC, NEWPLAYER, ACK, DISCONNECTED, TRYMOVE, OKMOVE, UPDATENEMIES, FORCETP, PING
};

struct InputCommand {
	int xToCheck = 0; //la pantalla te 576
	int yToCheck = 0;
	int idMove = 0;	
};

#define BITSIZE_POS 10
#define BITSIZE_PLAYERID 2
#define BITSIZE_MSGID 16
#define BITSIZE_PACKETYPE 4