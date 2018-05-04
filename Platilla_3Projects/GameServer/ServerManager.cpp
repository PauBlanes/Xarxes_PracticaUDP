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
		ResendCriticalMsgs();
		
		//Comprovem que no s'hagin desconectat
		for (map<int, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			if (it->second.CheckDisconnection()) {
				cout << "Client " << (int)it->first << " disconected" << endl;
				lastDisconnectedPlayer = it->first;				
				

				//Avisem als altres
				for (map<int, ClientProxy>::iterator it2 = clients.begin(); it2 != clients.end(); it2++) {
					if (it2->first != lastDisconnectedPlayer)
						SendCommand(it2->first, DISCONNECTED);
				}

				clients.erase(it);
				break;
			}			
		}

	}
}

void ServerManager::Send(char* m, int l, IpAddress i , unsigned short p, bool isCritical) {
	
	Mesage msg { m,l,i,p };

	//Simulem perdua de paquets
	srand(time(nullptr));
	int rnd = rand() % 100;	
	if (rnd >= PERCENTAGE_PACKET_LOSS) {
		
		socket.send(msg.buffer, msg.len, msg.ip, msg.port);		
	}
	else {
		cout << "Packet perdido" << endl;
	}

	//guardem el missatge pq es critic i actualitzem el id de packet
	if (isCritical) {
		
		criticalPackets[packetId] = msg;
		packetId++;
	}
	
}
void ServerManager::Send(Mesage msg, bool isCritical) {

	//Simulem perdua de paquets
	srand(time(nullptr));
	int rnd = rand() % 100;
	if (rnd >= PERCENTAGE_PACKET_LOSS) {		
	
		socket.send(msg.buffer, msg.len, msg.ip, msg.port);		
		
	}
	else {
		cout << "Packet perdido" << endl;
	}

	//guardem el missatge pq es critic i actualitzem el id de packet
	if (isCritical) {
		criticalPackets[packetId] = msg;
		packetId++; //pq si l'augmentem abans d'enviar aqui queda diferent el que enviem del que ens guardem
	}
}

void ServerManager::ReceiveCommand() {
	IpAddress ipAddr;
	unsigned short newPort;

	char rMsg[100];
	size_t received;

	if (socket.receive(rMsg, 100, received, ipAddr, newPort) == sf::Socket::Done) {
		//llegim el missatge
		InputMemoryBitStream imbs(rMsg, received * 8);
		
		//ens guardem quin comando es
		CommandType cmd = EMPTY;
		imbs.Read(&cmd, BITSIZE_PACKETYPE);


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
			int msgId = 0;
			imbs.Read(&msgId, BITSIZE_MSGID);
			
			//borrem el missatge de la llista de pendents
			for (map<int, Mesage>::iterator it = criticalPackets.begin(); it != criticalPackets.end(); it++) {				
				if (it->first == msgId)
					criticalPackets.erase(it);
			}
							
			cout << "Rebut ack, Tamany packets critics " << criticalPackets.size() << endl;
						

			break;
		}
		case TRYMOVE: 
		{
			//ens guardem quin jugador és
			int clientID = 0;
			imbs.Read(&clientID, BITSIZE_PLAYERID);
			
			int moveID = 0;
			imbs.Read(&moveID, BITSIZE_MSGID);
			
			int newX = 0; int newY = 0;
			imbs.Read(&newX, BITSIZE_POS);
			imbs.Read(&newY, BITSIZE_POS);
			
			
			//Setegem el client que toca
			map<int, ClientProxy>::iterator it = clients.find(clientID);
			if (it != clients.end()) {
				it->second.currMovState.idMove = moveID;
				it->second.currMovState.xToCheck = newX;
				it->second.currMovState.yToCheck = newY;
			}
			
			break;
			
		}
		case PING:
		{
			//ens guardem quin jugador és
			int clientID = 0;
			imbs.Read(&clientID, BITSIZE_PLAYERID);
			map<int, ClientProxy>::iterator it = clients.find(clientID);
			if (it != clients.end()) {				
				it->second.disconectionClock.restart(); //resetegem temoritzador de desconexio
			}
			

		}
		default:
			break;
		}
	}
}

void ServerManager::AddClientIfNew(IpAddress newIp, unsigned short newPort) {
	bool isNew = true;
	
	//Comprovem si és nou
	map<int, ClientProxy>::iterator it = clients.begin();
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
		Coordinates newPos{ clientIndex*64 + 5, clientIndex* 64 +5 };
		
		//Afegim el nou client al mapa
		ClientProxy newClient(newIp, newPort, newPos);
		newClient.currMovState.xToCheck = newPos.x;
		newClient.currMovState.yToCheck = newPos.y;
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

void ServerManager::SendCommand(int clientId, CommandType cmd) {
	
	OutputMemoryBitStream ombs;
	
	//Ens guardem el client	
	ClientProxy receiverClient = GetClient(clientId);
		
	switch (cmd) {
	case WC :
	{
		cout << "envio welcome a client " << (int)clientId << "(" << receiverClient.IP << ":" << receiverClient.port << ")" << endl;	

		//Afegim capçalera
		ombs.Write(WC, BITSIZE_PACKETYPE);
		
		//afegim la nostra pos
		ombs.Write(clientId, BITSIZE_PLAYERID);

		ombs.Write(receiverClient.position.x, BITSIZE_POS);
		ombs.Write(receiverClient.position.y, BITSIZE_POS);
			
		//afegim el numero de jugadors		
		ombs.Write((clients.size() - 1), BITSIZE_PLAYERID);
		//Afegim info dels actuals

		for (map<int, ClientProxy>::iterator it = clients.begin(); it != (--clients.end()); it++) {
			
			ombs.Write(it->first, BITSIZE_PLAYERID);			
			ombs.Write(it->second.position.x, BITSIZE_POS);			
			ombs.Write(it->second.position.y, BITSIZE_POS);

		}
		
		//enviem, el propi send ja el guardara si es critic				
		Send(ombs.GetBufferPtr(), ombs.GetByteLength(), receiverClient.IP, receiverClient.port, false);
		
		break;
	}
	case NEWPLAYER:
	{
		cout << "informo de newPlayer al client " << (int)clientId << "(" << receiverClient.IP << ":" << receiverClient.port << ")" << endl;

		//Afegim capçalera
		ombs.Write(NEWPLAYER, BITSIZE_PACKETYPE);
		ombs.Write(packetId, BITSIZE_MSGID);

		//Agafegim la pos del nou client		
		ClientProxy lastClient = GetClient((int)clientIndex);		
		ombs.Write(clientIndex, BITSIZE_PLAYERID);		
		ombs.Write(lastClient.position.x, BITSIZE_POS);
		ombs.Write(lastClient.position.y, BITSIZE_POS);
		
		//enviem
		Send(ombs.GetBufferPtr(), ombs.GetByteLength(), receiverClient.IP, receiverClient.port, true);
		
		break;
	}
	case OKMOVE: 
	{		
		//capcelera
		ombs.Write(OKMOVE, BITSIZE_PACKETYPE);
		ombs.Write(clientId, BITSIZE_PLAYERID);

		//afegim nostra info		
		ombs.Write(receiverClient.currMovState.idMove,BITSIZE_MSGID);
		ombs.Write(receiverClient.currMovState.xToCheck, BITSIZE_POS);
		ombs.Write(receiverClient.currMovState.yToCheck, BITSIZE_POS);			

		//enviem a tots
		for (map<int, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			Send(ombs.GetBufferPtr(), ombs.GetByteLength(), it->second.IP, it->second.port, false);
		}
		
		break;
	}	
	case FORCETP:
	{
		//capcelera
		ombs.Write(FORCETP, BITSIZE_PACKETYPE);
		ombs.Write(packetId, BITSIZE_MSGID);
		
		//afegim la pos on ha de fer tp
		map<int, ClientProxy>::iterator it = clients.find(clientId);
		if (it != clients.end()) {
			ombs.Write(it->second.position.x, BITSIZE_POS);
			ombs.Write(it->second.position.y, BITSIZE_POS);
		}
		//enviem	
		Send(ombs.GetBufferPtr(), ombs.GetByteLength(), receiverClient.IP, receiverClient.port, true);

		break;
	}
	case DISCONNECTED:
	{
		//capcelera
		ombs.Write(DISCONNECTED, BITSIZE_PACKETYPE);
		ombs.Write(packetId, BITSIZE_MSGID);
		
		ombs.Write(lastDisconnectedPlayer, BITSIZE_PLAYERID); //Afegim qui s'ha desconectat

		//enviem		
		Send(ombs.GetBufferPtr(), ombs.GetByteLength(), receiverClient.IP, receiverClient.port, true);
	}
	}

}

ClientProxy ServerManager::GetClient(int id) {
	map<int, ClientProxy>::iterator it = clients.find(id);
	if (it != clients.end()) {		
		return it->second;
	}
}

void ServerManager::ResendCriticalMsgs() {	
	Time currTime = resendClock.getElapsedTime();
	if (currTime.asMilliseconds() > RESEND_TIME) {
		
		for (map<int, Mesage>::iterator it = criticalPackets.begin(); it != criticalPackets.end(); it++) {
			
			Send(it->second, false);//perque no volem tornarlo a afegir al array, ja que no l'ehm borrat encara
			
		}
		
		resendClock.restart();
	}

	currTime = sendPosClock.getElapsedTime();
	if (currTime.asMilliseconds() > SEND_POS_WAIT_TIME) {		

		//enviem posicions
		for (map<int, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			
			//VALIDEM LA POSICIO			
			int testX = it->second.currMovState.xToCheck;
			int testY = it->second.currMovState.yToCheck;
			
			if (testX < 590 && testX > 0 && testY < 590 && testY > 0) { //si es bona ens actualitzem la nostra i enviem ok
				
				//Si la delta no ha canviat no envio
				if (it->second.currMovState.xToCheck != it->second.position.x || it->second.currMovState.yToCheck != it->second.position.y) {
					SendCommand(it->first, OKMOVE); //nomes s'enviara si hi ha hagut moviment. AQUI ENVIEM A TOTHOM EL OK I DES DEL CLIENT JA VEURAN SI ÉS EL SEU				
				}
				
				it->second.position.x = testX;
				it->second.position.y = testY;
				
			}
			else { //si no es bona enviem que es teletransporti a l'antiga posicio
				SendCommand(it->first, FORCETP);
			}			

			
		}
		sendPosClock.restart();
	}
}