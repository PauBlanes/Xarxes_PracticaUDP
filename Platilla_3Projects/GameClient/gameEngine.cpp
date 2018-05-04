#include "gameEngine.h"


#define MAX 100
#define SIZE_TABLERO 64
#define SIZE_FILA_TABLERO 8
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5

#define SIZE_TABLERO 64
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5


char tablero[SIZE_TABLERO];


/**
* Cuando el jugador clica en la pantalla, se nos da una coordenada del 0 al 512.
* Esta función la transforma a una posición entre el 0 y el 7
*/
sf::Vector2f TransformaCoordenadaACasilla(int _x, int _y)
{
	float xCasilla = _x / LADO_CASILLA;
	float yCasilla = _y / LADO_CASILLA;
	sf::Vector2f casilla(xCasilla, yCasilla);
	return casilla;
}

/**
* Si guardamos las posiciones de las piezas con valores del 0 al 7,
* esta función las transforma a posición de ventana (pixel), que va del 0 al 512
*/
sf::Vector2f BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}



GameEngine::GameEngine()
{

	socket.setBlocking(false);

	ip = IpAddress::getLocalAddress();
	
	OutputMemoryBitStream ombs;
	ombs.Write(HELLO, BITSIZE_PACKETYPE);
	
	socket.send(ombs.GetBufferPtr(), ombs.GetByteLength(), ip, PORT);
	
	Clock clock;
	clock.restart();
	//Bucle del joc
	while (!welcome) {

		//Comprovem si hem rebut el welcome
		ReceiveCommands();

		//Si han passat 500 ms tornem a enviar missatge hello		
		Time currTime = clock.getElapsedTime();
		if (currTime.asMilliseconds() >  500) {
			
			socket.send(ombs.GetBufferPtr(), ombs.GetByteLength(), ip, 50000);			
			cout << "sending hello again" << endl;
			clock.restart();
		}		

	}

}


GameEngine::~GameEngine()
{
}

void GameEngine::startGame() {
	
	sf::Vector2f casillaOrigen, casillaDestino;
	
	bool casillaMarcada = true;

	sf::RenderWindow window(sf::VideoMode(640, 640), "MONEY GAME");
	while (window.isOpen())
	{
		//comprovem comandos
		ReceiveCommands();
		
		//enviem la pos periodicament
		SendPosRoutine();

		sf::Event event;

		//Este primer WHILE es para controlar los eventos del mouse
		while (window.pollEvent(event))
		{
			//cout << others.size() << endl;
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::A ) {
					me.setMyPos(me.getMyPos().x- 3,me.getMyPos().y);
					IMoved = true;
				}
				else if (event.key.code == sf::Keyboard::D) {
					me.setMyPos(me.getMyPos().x + 3, me.getMyPos().y);
					IMoved = true;
				}
				else if (event.key.code == sf::Keyboard::W) {
					me.setMyPos(me.getMyPos().x, me.getMyPos().y- 3);
					IMoved = true;
				}
				else if (event.key.code == sf::Keyboard::S) {
					me.setMyPos(me.getMyPos().x, me.getMyPos().y + 3);
					IMoved = true;
				}
			break;
			default:
				break;

			}
		}

		window.clear();

		//A partir de aqu?es para pintar por pantalla
		//Este FOR es para el tablero
		for (int i = 0; i<10; i++)
		{
			for (int j = 0; j<10; j++)
			{
				sf::RectangleShape rectBlanco(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));

				rectBlanco.setFillColor(sf::Color::Black);
				rectBlanco.setOutlineColor(sf::Color::White);
				rectBlanco.setOutlineThickness(2);
				rectBlanco.setPosition(sf::Vector2f(i*LADO_CASILLA, j*LADO_CASILLA));
				window.draw(rectBlanco);
			}
		}


		//Pintar pj
		if (me.activated)
			me.Draw(&window, false);
		//Pintar enemies
		for (int i = 0; i < others.size(); i++)
		{			
			others[i].Draw(&window, true);		
		}
		//Pintar moneda
		for (int i = 0; i < coin.size(); i++)
		{
			coin[i].Draw(&window);
			//pillar moneda
			Vector2f dist(me.getMyPos() - coin[i].getPos());
			float distF = sqrt(dist.x*dist.x+ dist.y*dist.y);
			if (distF <= 50) {
				me.addScore();
				cout << me.getScore() << endl;
				coin[i].setPos(rand()%10*LADO_CASILLA + OFFSET_AVATAR, rand() % 10 *LADO_CASILLA + OFFSET_AVATAR);
			}

		}

		//pillar moneda
		

		window.display();
	}
}

void GameEngine::ReceiveCommands() {
	
	IpAddress ipAddr;
	unsigned short newPort;

	char rMsg[100];
	size_t received;	
	
	if (socket.receive(rMsg, 100, received, ipAddr, newPort) == sf::Socket::Done) {		

		InputMemoryBitStream imbs(rMsg, received * 8);
		CommandType cmd = EMPTY;
		imbs.Read(&cmd, BITSIZE_PACKETYPE);
			
		
		switch (cmd)
		{
		case WC:
		{		
			

			if (!welcome) {
				cout << "benvingut" << endl;
				welcome = true;			

				//guardo la meva id
				int nId = 0;
				imbs.Read(&nId, BITSIZE_PLAYERID);
				me.id = nId;							

				//setejo la meva pos
				int firstX = 0; int firstY = 0;
				imbs.Read(&firstX, BITSIZE_POS);
				imbs.Read(&firstY, BITSIZE_POS);
				me.setMyPos(firstX, firstY);				

				//Setejo la posicio dels altres
				int numOthers = 0;
				imbs.Read(&numOthers, BITSIZE_PLAYERID);
				
				for (int i = 0; i < numOthers; i++) {
					int tempID = 0;
					int tempX = 0; int tempY = 0;
					imbs.Read(&tempID, BITSIZE_PLAYERID);
					imbs.Read(&tempX, BITSIZE_POS);
					imbs.Read(&tempY, BITSIZE_POS);

					others.push_back(Player(tempX, tempY, Color::Red, tempID));
				}

				//pintar coins
				int provaID = 0;
				coin.push_back(Coins(rand() % 10 * LADO_CASILLA + OFFSET_AVATAR, rand() % 10 * LADO_CASILLA + OFFSET_AVATAR,provaID));
				
				//Obrir la mapa
				startGame();
				
			}		

			

			break;
		}
		case NEWPLAYER:
		{
			//guardem id del msg
			int packetID = 0;
			imbs.Read(&packetID, BITSIZE_MSGID);
			
			//Guardem id del player
			int newId = 0;
			imbs.Read(&newId, BITSIZE_PLAYERID);
			
			//si és nou ens guardem el nou jugador
			if (CheckIfNew(newId)) {
				
				int newX = 0; int newY = 0;
				imbs.Read(&newX, BITSIZE_POS);
				imbs.Read(&newY, BITSIZE_POS);

				others.push_back(Player(newX, newY, Color::Red, newId));

			}			
			SendACK(packetID);

			break;
		}
		case OKMOVE:
		{			
			//guardem id de a qui pertany el moviment
			int clientID = 0;
			imbs.Read(&clientID, BITSIZE_PLAYERID);

			//guardem id del msg
			int moveID = 0;
			imbs.Read(&moveID, BITSIZE_MSGID);
			//guardem pos. AQUI NO SON DELTES SINO POS TOTAL
			int newX = 0; int newY = 0;
			imbs.Read(&newX, BITSIZE_POS);
			imbs.Read(&newY, BITSIZE_POS);
			
			//si soc jo em guardo que vaig be
			if (clientID == me.id) {				
				//cout << "ok move to : " << newX << "," << newY << endl;
			}
			//si es un dels enemics fem la interpolacio
			else if (newX != 0 || newY != 0) {
				for (int i = 0; i < others.size(); i++) {
					if (others[i].id == clientID) {												
						others[i].CreateLerpPath(newX, newY);
						//cout << "move enemy to : " << newX << "," << newY << endl;
					}
				}
			}						

			break;
		}		
		case FORCETP:
		{
			//guardem id del msg
			int packetID = 0;
			imbs.Read(&packetID, BITSIZE_MSGID);

			int newX = 0; int newY = 0;
			imbs.Read(&newX, BITSIZE_POS);
			imbs.Read(&newY, BITSIZE_POS);
			//setejo meva pos			
			me.setMyPos(newX, newY);

			SendACK(packetID);
			//cout << "sending ack of force tp" << endl;

			break;
		}
		case DISCONNECTED:
		{

			//guardem id del msg
			int packetID = 0;
			imbs.Read(&packetID, BITSIZE_MSGID);

			//guardem id de a qui pertany el moviment
			int clientID = 0;
			imbs.Read(&clientID, BITSIZE_PLAYERID);

			for (int i = 0; i < others.size(); i++) {
				if (others[i].id == clientID) {
					others.erase(others.begin() + i);
					cout << "Cliente " << (int)clientID << " se ha desconectado" << endl;
				}
			}

			SendACK(packetID);

			break;
		}
		default:
			break;		
		}
	}

}
void GameEngine::SendACK(int msgId) {
	
	OutputMemoryBitStream ombs;
	ombs.Write(ACK, BITSIZE_PACKETYPE);
	ombs.Write(msgId, BITSIZE_MSGID);	
	socket.send(ombs.GetBufferPtr(), ombs.GetByteLength(), ip, PORT);
	
}

void GameEngine::SendCommands(CommandType cmd) {
	
	OutputMemoryBitStream ombs;
	
	switch (cmd)
	{	
	case TRYMOVE:
		
		ombs.Write(TRYMOVE, BITSIZE_PACKETYPE);
		ombs.Write(me.id, BITSIZE_PLAYERID);

		ombs.Write(iC.idMove, BITSIZE_MSGID); 
		ombs.Write((int)me.getMyPos().x, BITSIZE_POS);
		ombs.Write((int)me.getMyPos().y, BITSIZE_POS);
		
		iC.idMove++;
		
		socket.send(ombs.GetBufferPtr(), ombs.GetByteLength(), ip, PORT);
		break;
	case PING:
		
		ombs.Write(PING, BITSIZE_PACKETYPE);
		
		ombs.Write(me.id, BITSIZE_PLAYERID);

		socket.send(ombs.GetBufferPtr(), ombs.GetByteLength(), ip, PORT);

	default:
		break;
	}
}

bool GameEngine::CheckIfNew(int p2Check) {
	for each (Player p in others)
	{
		if (p2Check == p.id)
			return false;
	}
	return true;
}

void GameEngine::SendPosRoutine() {
	Time currTime = sendPosClock.getElapsedTime();
	if (currTime.asMilliseconds() > SEND_POS_TIME) {
		if (IMoved) { //Nomes envio si t'has mogut
			SendCommands(TRYMOVE);	
			
			IMoved = false;
			sendPosClock.restart();
		}	
		SendCommands(PING);
	}
}

