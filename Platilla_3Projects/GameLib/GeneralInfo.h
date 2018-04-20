#pragma once
#include <SFML\Network.hpp>

struct Coordinates {
	int16_t x;
	int16_t y;
};

enum CommandType {
	EMPTY, HELLO, WC, NEWPLAYER, ACK, DISCONNECTED, TRYMOVE, OKMOVE, UPDATENEMIES, FORCETP
};

struct InputCommand {
	uint16_t deltaX = 0; //la pantalla te 576
	uint16_t deltaY = 0;
	uint8_t idMove = 0;	
};
