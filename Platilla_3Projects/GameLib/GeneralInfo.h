#pragma once
#include <SFML\Network.hpp>

struct Coordinates {
	uint8_t x;
	uint8_t y;
};

enum CommandType {
	EMPTY, HELLO, WC, NEWPLAYER, ACK, DISCONNECTED, POS
};

struct InputCommand {
	uint16_t deltaX; //la pantalla te 576
	uint16_t deltaY;
	uint8_t idMove;	
};
