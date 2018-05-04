#pragma once
#include <iostream>
#include <SFML\Network.hpp>
#include <SFML\Graphics.hpp>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include "Player.h"
#include "Coins.h"
#include "GeneralInfo.h"
#include "OutputMemoryStream.h"
#include "InputMemoryStream.h"


using namespace sf;
using namespace std;

#define PORT 50000
#define SEND_POS_TIME 150

class GameEngine
{
private:
	bool welcome;
	UdpSocket socket;
	string nick;
	IpAddress ip;
	bool IMoved;
	InputCommand iC;
	Clock sendPosClock;

public:
	Player me;
	vector<Player> others;
	vector<Coins> coin;
	GameEngine();
	~GameEngine();
	void startGame();	
	void ReceiveCommands();
	void SendCommands(CommandType cmd); //FER RANDOM PER NOMES ENVIAR A VEGADES
	void SendACK(int msgId);
	bool CheckIfNew(uint8_t);
	void SendPosRoutine();
};

