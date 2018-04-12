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

	while (true) {

		//Anem rent i processant comandos
		ReceiveCommand();

		//tornem a enviar els missatges critics si ha passat el temps
		//ResendCriticalMsgs();
		
		//Comprovem que no s'hagin desconectat
		/*for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			if (it->second.CheckDisconnection()) {
				cout << "Client " << it->first << "disconected" << endl;
			}
		}*/

	}
}

void ServerManager::Send(char* m, int l, IpAddress i , unsigned short p, bool isCritical) {
	
	//AFEGIR EL RANDOM DE LA PERDUA DE PAQUETS

	Mesage msg{ m,l,i,p };

	//FER EL PARTIAL
	socket.send(msg.buffer, msg.len, msg.ip, msg.port);
	
	//guardem el missatge pq es critic i actualitzem el id de packet
	if (isCritical) {		
		criticalPackets[packetId] = msg;
		packetId++;
	}
	cout << criticalPackets.size() << endl;
}
void ServerManager::Send(Mesage msg, bool isCritical) {

	//FER EL PARTIAL
	socket.send(msg.buffer, msg.len, msg.ip, msg.port);

	//guardem el missatge pq es critic i actualitzem el id de packet
	if (isCritical) {
		criticalPackets[packetId] = msg;
		packetId++;
	}
}

void ServerManager::ReceiveCommand() {
	IpAddress ipAddr;
	unsigned short newPort;

	char rMsg[100];
	size_t received;

	if (socket.receive(rMsg, 100, received, ipAddr, newPort) == sf::Socket::Done) {
		//llegim el missatge
		InputMemoryStream ims(rMsg, received);
		
		//ens guardem quin comando es
		uint8_t cmdInt;
		ims.Read(&cmdInt);
		CommandType cmd = (CommandType)cmdInt;

		switch (cmd)
		{		
		case HELLO:
			cout << "rebut HELLO de " << ipAddr << ":" << newPort << endl;
			//quan rebem el nick tmb l'haurem de passar per aqui
			AddClientIfNew(ipAddr, newPort);
			break;		
		case ACK:
		{
			//ens guardem id de missatge
			uint8_t msgId;
			ims.Read(&msgId);

			//borrem el missatge de la llista de pendents
			map<uint16_t, Mesage>::iterator it = criticalPackets.find(msgId);
			if (it != criticalPackets.end())
				criticalPackets.erase(it);

			break;
		}
		default:
			break;
		}
	}
}

void ServerManager::AddClientIfNew(IpAddress newIp, unsigned short newPort) {
	bool isNew = true;
	
	//Comprovem si és nou
	map<uint8_t, ClientProxy>::iterator it = clients.begin();
	while (it != clients.end())
	{
		if (newIp == it->second.IP && newPort == it->second.port) {
			isNew = false;
			SendCommand(it->first, WC);
		}
		it++;
	}

	

	if (isNew) {			

		//Generem nova pos
		//GenerateNewPos();
		Coordinates newPos{ clientIndex, clientIndex };
		
		//Afegim el nou client al mapa
		ClientProxy newClient(newIp, newPort, newPos);		
		clients[clientIndex] = newClient;

		//enviem welcome	
		SendCommand(clientIndex, WC);
		//informem als clients actuals que hi ha un nou
		for (int i = 0; i < clientIndex; i++) {
			SendCommand(i, NEWPLAYER);
		}
				
		clientIndex++;		
	}
}

void ServerManager::SendCommand(uint8_t clientId, CommandType cmd) {
	OutputMemoryStream oms;
	
	//Ens guardem el client	
	ClientProxy receiverClient = GetClient(clientId);
		
	switch (cmd) {
	case WC :
	{
		cout << "envio welcome a client " << (int)clientId << "(" << receiverClient.IP << ":" << receiverClient.port << ")" << endl;	

		//Afegim capçalera
		oms.Write((uint8_t)CommandType::WC);
		oms.Write(packetId);
		//afegim la nostra pos		
		oms.Write(receiverClient.position.x);
		oms.Write(receiverClient.position.y);		
		//afegim el numero de jugadors		
		oms.Write((uint8_t)clients.size()-1); //pq ja ens hem afegit a nosaltres mateixos
		//Afegim info dels actuals
		for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != (--clients.end()); it++) {
			oms.Write(it->second.position.x);
			oms.Write(it->second.position.y);
		}

		//enviem, el propi send ja el guardara si es critic		
		Send(oms.GetBufferPtr(), oms.GetLength(), receiverClient.IP, receiverClient.port, false);
		
		break;
	}
	case NEWPLAYER:
	{
		cout << "informo de newPlayer al client " << (int)clientId << "(" << receiverClient.IP << ":" << receiverClient.port << ")" << endl;

		//Afegim capçalera
		oms.Write((uint8_t)CommandType::NEWPLAYER);
		oms.Write(packetId);
		//Agafegim la pos del nou client		
		ClientProxy lastClient = GetClient((uint8_t)clientIndex);		
		oms.Write(lastClient.position.x);
		oms.Write(lastClient.position.y);
		
		//enviem		
		Send(oms.GetBufferPtr(), oms.GetLength(), receiverClient.IP, receiverClient.port, true);
		
		break;
	}
	}

}

ClientProxy ServerManager::GetClient(uint8_t id) {
	map<uint8_t, ClientProxy>::iterator it = clients.find(id);
	if (it != clients.end()) {		
		return it->second;
	}
}

void ServerManager::ResendCriticalMsgs() {
	Time currTime = resendClock.getElapsedTime();
	if (currTime.asMilliseconds() > RESEND_TIME) {
		
		for (map<uint16_t, Mesage>::iterator it = criticalPackets.begin(); it != criticalPackets.end(); it++) {
			Send(it->second, true);
		}
		resendClock.restart();
	}
}