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
		/*for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			if (it->second.CheckDisconnection()) {
				cout << "Client " << it->first << "disconected" << endl;
			}
		}*/

	}
}

void ServerManager::Send(char* m, int l, IpAddress i , unsigned short p, bool isCritical) {
	
	Mesage msg { m,l,i,p };

	//Simulem perdua de paquets
	srand(time(nullptr));
	int rnd = rand() % 100;	
	if (rnd >= PERCENTAGE_PACKET_LOSS) {
		
		//FER EL PARTIAL
		socket.send(msg.buffer, msg.len, msg.ip, msg.port);		
	}
	else {
		cout << "Packet perdido en las profundidades de la red" << endl;
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
		//FER EL PARTIAL
	
		socket.send(msg.buffer, msg.len, msg.ip, msg.port);		
		
	}
	else {
		cout << "Packet perdut" << endl;
	}

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
			int16_t msgId;
			ims.Read(&msgId);

			//borrem el missatge de la llista de pendents
			map<int16_t, Mesage>::iterator it = criticalPackets.find(msgId);
			if (it != criticalPackets.end())
				criticalPackets.erase(it);
			cout << "Rebut ack, Tamany packets critics " << criticalPackets.size() << endl;
			break;
		}
		case TRYMOVE: 
		{
			//ens guardem quin jugador és
			uint8_t clientID;
			ims.Read(&clientID);

			uint8_t moveID = 0;
			ims.Read(&moveID);
			
			int16_t newX = 0; int16_t newY = 0;
			ims.Read(&newX);
			ims.Read(&newY);
			
			//Setegem el client que toca
			map<uint8_t, ClientProxy>::iterator it = clients.find(clientID);
			if (it != clients.end()) {
				it->second.currMovState.idMove = moveID;
				it->second.currMovState.xToCheck = newX;
				it->second.currMovState.yToCheck = newY;
			}
			
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
		Coordinates newPos{ clientIndex*64 + 5, clientIndex* 64 +5 };
		
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
		//cout << "envio welcome a client " << (int)clientId << "(" << receiverClient.IP << ":" << receiverClient.port << ")" << endl;	

		//Afegim capçalera
		oms.Write((uint8_t)CommandType::WC);
		
		//afegim la nostra pos
		oms.Write(clientId);
		oms.Write(receiverClient.position.x);
		oms.Write(receiverClient.position.y);		
		//afegim el numero de jugadors		
		oms.Write((uint8_t)(clients.size()-1)); //pq ja ens hem afegit a nosaltres mateixos
		//Afegim info dels actuals
		for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != (--clients.end()); it++) {
			oms.Write(it->first);
			oms.Write(it->second.position.x);
			oms.Write(it->second.position.y);
			//cout << (int)it->second.position.x /*<< "," << it->second.position.y*/ << endl;
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
		oms.Write((uint8_t)clientIndex);
		oms.Write(lastClient.position.x);
		oms.Write(lastClient.position.y);
		
		//enviem		
		Send(oms.GetBufferPtr(), oms.GetLength(), receiverClient.IP, receiverClient.port, true);
		
		
		break;
	}
	case OKMOVE: 
	{		
		//capcelera
		oms.Write((uint8_t)CommandType::OKMOVE);
		oms.Write(clientId); //Afegim el idPlayer per des del client saber si és el nostre moviment o el de l'altre

		//afegim nostra info		
		oms.Write(receiverClient.currMovState.idMove);
		oms.Write(receiverClient.currMovState.xToCheck);
		oms.Write(receiverClient.currMovState.yToCheck);
			

		//enviem a tots
		for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			Send(oms.GetBufferPtr(), oms.GetLength(), it->second.IP, it->second.port, false); //el id del packet podria coincidir amb el idMove i donar problemes, s'ha de fer llista a part
		}
			
		break;
	}
	/*case UPDATENEMIES:
	{
		//afegim pos dels altres
		oms.Write((uint8_t)CommandType::UPDATENEMIES);
		oms.Write((uint8_t)(clients.size() - 1));

		for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			if (it->first != clientId ) {				
				oms.Write(it->first);
				oms.Write(it->second.currMovState.deltaX);
				oms.Write(it->second.currMovState.deltaY);
			}
		}

		//enviem		
		Send(oms.GetBufferPtr(), oms.GetLength(), receiverClient.IP, receiverClient.port, false); //el id del packet podria coincidir amb el idMove i donar problemes, s'ha de fer llista a part

		break;
	}*/
	case FORCETP:
	{
		//capcelera
		oms.Write((uint8_t)CommandType::FORCETP);
		//afegim la pos on ha de fer tp
		map<uint8_t, ClientProxy>::iterator it = clients.find(clientId);
		if (it != clients.end()) {
			oms.Write(it->second.position.x);
			oms.Write(it->second.position.y);
		}
		//enviem		
		Send(oms.GetBufferPtr(), oms.GetLength(), receiverClient.IP, receiverClient.port, false); //HA DE SER CRITIC?
		
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
		
		for (map<int16_t, Mesage>::iterator it = criticalPackets.begin(); it != criticalPackets.end(); it++) {
			
			Send(it->second, false);//perque no volem tornarlo a afegir al array, ja que no l'ehm borrat encara
			
		}
		
		resendClock.restart();
	}

	currTime = sendPosClock.getElapsedTime();
	if (currTime.asMilliseconds() > SEND_POS_WAIT_TIME) {		

		//enviem posicions
		for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			
			//VALIDEM LA POSICIO			
			int16_t testX = /*it->second.position.x + */it->second.currMovState.xToCheck;
			int16_t testY = /*it->second.position.y +*/ it->second.currMovState.yToCheck;
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

			//SendCommand(it->first, UPDATENEMIES); 
			

			
		}

		//netegem els deltes
		for (map<uint8_t, ClientProxy>::iterator it = clients.begin(); it != clients.end(); it++) {
			
			it->second.currMovState.xToCheck = 0;
			it->second.currMovState.yToCheck = 0;
			
		}	

		sendPosClock.restart();
	}
}