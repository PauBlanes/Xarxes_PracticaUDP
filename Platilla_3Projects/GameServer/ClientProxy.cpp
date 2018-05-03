#include "ClientProxy.h"

ClientProxy::ClientProxy(IpAddress ip, unsigned short prt, Coordinates pos) {
	IP = ip;
	port = prt;
	position = pos;
}

bool ClientProxy::CheckDisconnection() {
	Time currTime = disconectionClock.getElapsedTime(); //RESETEJAR QUAN REPS ALGO
	if (currTime.asMilliseconds() > DISCONECTION_WAIT_TIME) {		
		return true;
	}
}
