#pragma once
#include "ClientProxy.h"
#include <iostream>
#include "InputMemoryStream.h"
#include "OutputMemoryStream.h"
#include "InputMemoryBitStream.h"
#include "OutputMemoryBitStream.h"
#include <time.h>

#define RESEND_TIME 500
#define PERCENTAGE_PACKET_LOSS 0
#define SEND_POS_WAIT_TIME 200

struct Mesage {
	char* buffer;
	int len;
	IpAddress ip;
	unsigned short port;
};

struct SendInfo {
	IpAddress ip;
	unsigned short port;
};

class ServerManager {
private:
	map<int, Mesage> criticalPackets; //idPacket, missatge+adress
	int packetId;
	Clock resendClock;

	int clientIndex;//per assignar id als clients quan els rebem
	map<int, ClientProxy> clients; //idClient, client

	UdpSocket socket;
	int lastDisconnectedPlayer;
	Clock sendPosClock;
public:
	ServerManager();
	void Send(char*, int, IpAddress, unsigned short, bool);
	void Send(Mesage, bool);
	void SendCommand(int, CommandType);
	void ReceiveCommand();
	void ResendCriticalMsgs();
	void AddClientIfNew(IpAddress, unsigned short);
	Coordinates GenerateNewPos();
	ClientProxy GetClient(int); 
};
