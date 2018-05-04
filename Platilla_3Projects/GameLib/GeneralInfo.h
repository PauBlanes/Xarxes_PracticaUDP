#pragma once
#include <SFML\Network.hpp>

struct Coordinates {
	int16_t x;
	int16_t y;
};

enum CommandType {
	EMPTY, HELLO, WC, NEWPLAYER, ACK, DISCONNECTED, TRYMOVE, OKMOVE, UPDATENEMIES, FORCETP, PING
};

struct InputCommand {
	int16_t xToCheck = 0; //la pantalla te 576
	int16_t yToCheck = 0;
	uint8_t idMove = 0;	
};

#define BITSIZE_POS 10
#define BITSIZE_PLAYERID 2
#define BITSIZE_MSGID 16
