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

	//cout << "Write your username : " << endl;
	//cin >> nick;

	socket.setBlocking(false);

	ip = IpAddress::getLocalAddress();
	
	//Packet helloPacket;
	//helloPacket << HELLO;
	//helloPacket << nick;
	OutputMemoryStream oms;
	oms.Write((uint8_t)CommandType::HELLO);
	//oms.WriteString(nick);
	
	socket.send(oms.GetBufferPtr(), oms.GetLength(), ip, PORT);
	//socket.send(helloPacket, ip, 50000);
	Clock clock;
	clock.restart();
	//Bucle del joc
	while (!welcome) {

		//Comprovem si hem rebut el welcome
		ReceiveCommands();

		//Si han passat 500 ms tornem a enviar missatge hello		
		Time currTime = clock.getElapsedTime();
		if (currTime.asMilliseconds() >  500) {
			
			socket.send(oms.GetBufferPtr(), oms.GetLength(), ip, 50000);			
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
					//iC.deltaX-= 3;
				}
				else if (event.key.code == sf::Keyboard::D) {
					me.setMyPos(me.getMyPos().x + 3, me.getMyPos().y);
					//iC.deltaX+= 3;
				}
				else if (event.key.code == sf::Keyboard::W) {
					me.setMyPos(me.getMyPos().x, me.getMyPos().y- 3);
					//iC.deltaY-= 3;
				}
				else if (event.key.code == sf::Keyboard::S) {
					me.setMyPos(me.getMyPos().x, me.getMyPos().y + 3);
					//iC.deltaY+= 3;
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

		//draw my pos
		
		//set limit del mapa

		//Pintar pj
		if (me.activated)
			me.Draw(&window, false);
		
		for (int i = 0; i < others.size(); i++)
		{			
			others[i].Draw(&window, true);		
		}
		

		
			//en el principio marco con un recuadro amarillo para identificar.
			/*if (casillaMarcada)
			{
				sf::RectangleShape rect(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));
				rect.setPosition(sf::Vector2f(me.getMyPos().x*LADO_CASILLA, me.getMyPos().y*LADO_CASILLA));
				rect.setFillColor(sf::Color::Transparent);
				rect.setOutlineThickness(5);
				rect.setOutlineColor(sf::Color::Yellow);
				window.draw(rect);
			}*/
		

		window.display();
	}
}

void GameEngine::ReceiveCommands() {
	//Packet rPack;
	IpAddress ipAddr;
	unsigned short newPort;

	char rMsg[100];
	size_t received;	
	
	if (socket.receive(rMsg, 100, received, ipAddr, newPort) == sf::Socket::Done) {
	//if (socket.receive(rPack, ipAddr, newPort) == sf::Socket::Done) {
		//int intCmd;
		//rPack >> intCmd;
		//PacketType cmd = (PacketType)intCmd;

		InputMemoryStream ims(rMsg, received);
		uint8_t cmdInt;
		ims.Read(&cmdInt);
		CommandType cmd = (CommandType)cmdInt;
		
		switch (cmd)
		{
		case WC:
		{			

			if (!welcome) {
				cout << "benvingut" << endl;
				welcome = true;			

				//guardo la meva id
				uint8_t newId;
				ims.Read(&newId);
				me.id = newId;

				//setejo la meva pos
				int16_t  newX, newY;
				ims.Read(&newX);
				ims.Read(&newY);
				me.setMyPos(newX, newY);
				//cout << (int)newX << "," << (int)newY << endl;
				//Setejo la posicio dels altres
				uint8_t numOthers;
				ims.Read(&numOthers);
				
				for (int i = 0; i < numOthers; i++) {				
					uint8_t provaID = 0;
					int16_t  provaX = 0;int16_t  provaY = 0;
					ims.Read(&provaID);					
					ims.Read(&provaX);
					ims.Read(&provaY);
					//cout << (int)provaX/* << "," << (int)provaY*/ << endl;
					others.push_back(Player(provaX, provaY, Color::Red, provaID));
				}
				//Obrir la mapa
				startGame();
			}		

			

			break;
		}
		case NEWPLAYER:
		{
			//guardem id del msg
			int16_t  packetId;
			ims.Read(&packetId);
			
			//Guardem id del player
			uint8_t newId;
			ims.Read(&newId);
			
			//si és nou ens guardem el nou jugador
			if (CheckIfNew(newId)) {
				
				int16_t  newX, newY;
				ims.Read(&newX);
				ims.Read(&newY);
				others.push_back(Player(newX, newY, Color::Red, newId));

			}
			
			SendACK(packetId);
			cout << "sending ack of new player" << endl;

			break;
		}
		case OKMOVE: 
		{			
			//guardem id de a qui pertany el moviment
			uint8_t clientID = 0;
			ims.Read(&clientID);

			//guardem id del msg
			uint8_t moveID = 0;
			ims.Read(&moveID);			
			//guardem pos. AQUI NO SON DELTES SINO POS TOTAL
			int16_t  newX = 0; int16_t  newY = 0;
			ims.Read(&newX);
			ims.Read(&newY);
			
			//si soc jo em guardo que vaig be
			if (clientID == me.id) {				
				cout << "ok move to : " << newX << "," << newY << endl;
			}
			//si es un dels enemics fem la interpolacio
			else if (newX != 0 || newY != 0) {
				for (int i = 0; i < others.size(); i++) {
					if (others[i].id == clientID) {												
						others[i].CreateLerpPath(newX, newY);
						cout << "move enemy to : " << newX << "," << newY << endl;
					}
				}
			}						

			break;
		}
		/*case UPDATENEMIES:
		{
			
			//setejo pos dels altres
			uint8_t numOthers = 0;
			ims.Read(&numOthers);
			
			uint8_t newID = 0;
			for (int i = 0; i < numOthers; i++) {
				ims.Read(&newID);
				
				for (int j = 0; j < others.size(); j++) {
					if (others[j].id == newID) {
						int16_t  newX = 0; int16_t  newY = 0;
						ims.Read(&newX);
						ims.Read(&newY);

						//Setegem valors per interpolar
						if (newX != 0 || newY != 0) {													
							others[j].CreateLerpPath(newX, newY);
							
							
						}						
					}
				}
			}
			break;
		}*/
		case FORCETP:
		{
			int16_t  newX = 0; int16_t  newY = 0;
			ims.Read(&newX);
			ims.Read(&newY);
			//setejo meva pos			
			me.setMyPos(newX, newY);
			break;
		}
		default:
			break;		
		}
	}

}
void GameEngine::SendACK(int msgId) {
	
	OutputMemoryStream oms;
	oms.Write((uint8_t)CommandType::ACK);
	oms.Write((int16_t )msgId);

	//POSAR PARTIAL
	socket.send(oms.GetBufferPtr(), oms.GetLength(), ip, PORT);
	//I GUARDAR EN EL LLISTA PER ANAR REENVIANT
}

void GameEngine::SendCommands(CommandType cmd) {
	
	OutputMemoryStream oms;
	
	switch (cmd)
	{	
	case TRYMOVE:
		//cout << (int)iC.deltaX << "," << (int)iC.deltaY << endl;
		oms.Write((uint8_t)CommandType::TRYMOVE);
		oms.Write(me.id);

		oms.Write(iC.idMove); iC.idMove++;		
		oms.Write((int16_t)me.getMyPos().x);
		oms.Write((int16_t)me.getMyPos().y);
		
		socket.send(oms.GetBufferPtr(), oms.GetLength(), ip, PORT);
		break;
	case PING:
		oms.Write((uint8_t)CommandType::PING);
		oms.Write(me.id);

		socket.send(oms.GetBufferPtr(), oms.GetLength(), ip, PORT);

	default:
		break;
	}
}

bool GameEngine::CheckIfNew(uint8_t p2Check) {
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
		//if (iC.deltaX != 0 || iC.deltaY != 0) { //Nomes envio si t'has mogut
			SendCommands(TRYMOVE);	
			SendCommands(PING);
			//iC.deltaX = 0; iC.deltaY = 0;
			sendPosClock.restart();
		//}		
	}
}

