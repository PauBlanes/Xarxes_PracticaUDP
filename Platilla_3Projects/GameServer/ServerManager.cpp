#include "ServerManager.h"

ServerManager::ServerManager() {
	//Connectem el port
	Socket::Status status = socket.bind(50000);
	if (status != sf::Socket::Done)
	{
		cout << "No se puede vincular al puerto" << endl;
	}
	else
		cout << "Port ok" << endl;

	socket.setBlocking(false);
}

void ServerManager::Send(Mesage msg) {
	socket.send(msg.buffer, msg.len, msg.ip, msg.port);
}