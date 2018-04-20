#pragma once
#include <SFML\Network.hpp>

struct Coordinates {
	uint16_t x;
	uint16_t y;
};

enum CommandType {
	EMPTY, HELLO, WC, NEWPLAYER, ACK, DISCONNECTED, TRYMOVE, OKMOVE, UPDATENEMIES
};

struct InputCommand {
	uint16_t deltaX = 0; //la pantalla te 576
	uint16_t deltaY = 0;
	uint8_t idMove = 0;	
};
