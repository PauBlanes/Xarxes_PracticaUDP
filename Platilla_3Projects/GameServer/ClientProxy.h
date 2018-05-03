#pragma once
#include <SFML\Network.hpp>
#include "GeneralInfo.h"
#include <iostream>

using namespace sf;
using namespace std;

#define DISCONECTION_WAIT_TIME 2000

class ClientProxy {
public:
	IpAddress IP;
	unsigned short port;
	Coordinates position;

	InputCommand currMovState;

	Clock disconectionClock;
	
	
	ClientProxy() {};
	ClientProxy(IpAddress, unsigned short, Coordinates);
	bool CheckDisconnection();
	
};
