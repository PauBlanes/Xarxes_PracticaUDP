#pragma once
#include <SFML\Network.hpp>

struct Coordinates {
	uint8_t x;
	uint8_t y;
};

enum CommandType {
	EMPTY, HELLO, WC, NEWPLAYER, PING, ACK, DISCONNECTED
};

struct InputCommand {
	Coordinates delta;
	uint8_t idMove;	
};
