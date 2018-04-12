#pragma once
#include "ClientProxy.h"
#include <iostream>


struct Mesage {
	char* buffer;
	int len;
	IpAddress ip;
	unsigned short port;
};

class ServerManager {
private:
	map<uint16_t, Mesage> criticalPackets; //idPacket, missatge+adress
	uint16_t packetId;

	int clientIndex;//per assignar id als clients quan els rebem
	map<uint8_t, ClientProxy> clients; //idClient, client

	UdpSocket socket;
public:
	ServerManager();
	void Send(Mesage msg);
	void SendCommand(CommandType cmd);
	void ReceiveCommand();
	void ResendCriticalMsgs();
};
