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



gameEngine::gameEngine()
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


gameEngine::~gameEngine()
{
}

void gameEngine::startGame() {
	
	sf::Vector2f casillaOrigen, casillaDestino;
	
	bool casillaMarcada = true;

	sf::RenderWindow window(sf::VideoMode(640, 640), "MONEY GAME");
	while (window.isOpen())
	{
		//comprovem comandos
		ReceiveCommands();

	

		sf::Event event;

		//Este primer WHILE es para controlar los eventos del mouse
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::A ) {
					me.setMyPos(me.getMyPos().x-1,me.getMyPos().y);
				}
				else if (event.key.code == sf::Keyboard::D) {
					me.setMyPos(me.getMyPos().x + 1, me.getMyPos().y);
				}
				else if (event.key.code == sf::Keyboard::W) {
					me.setMyPos(me.getMyPos().x, me.getMyPos().y-1);
				}
				else if (event.key.code == sf::Keyboard::S) {
					me.setMyPos(me.getMyPos().x, me.getMyPos().y + 1);
				}
			break;

			case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					int x = event.mouseButton.x;
					int y = event.mouseButton.y;
					/*if (!casillaMarcada)
					{
						casillaOrigen = TransformaCoordenadaACasilla(x, y);
						casillaMarcada = true;
						//TODO: Comprobar que la casilla marcada coincide con las posición del raton (si le toca al ratón)
						//o con la posicion de alguna de las piezas del gato (si le toca al gato)

					}
					else
					{
						casillaDestino = TransformaCoordenadaACasilla(x, y);
						if (casillaOrigen.x == casillaDestino.x && casillaOrigen.y == casillaDestino.y)
						{
							casillaMarcada = false;
							//Si me vuelven a picar sobre la misma casilla, la desmarco
						}
						else
						{
							
						}
					}*/
				}
				break;

			default:
				break;

			}
		}

		window.clear();

		//A partir de aquí es para pintar por pantalla
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
		if (me.getMyPos().x < 0) me.setMyPos(0, me.getMyPos().y);
		if (me.getMyPos().y < 0) me.setMyPos(me.getMyPos().x, 0);
		if (me.getMyPos().x > 576) me.setMyPos(576, me.getMyPos().y);
		if (me.getMyPos().y > 576) me.setMyPos(me.getMyPos().x, 576);

		//Pintar pj
		if (me.activated)
			me.Draw(&window);
		
		for each (Player p in others)
		{
			p.Draw(&window);
		}
		

		
			//en el principio marco con un recuadro amarillo para identificar.
			if (casillaMarcada)
			{
				sf::RectangleShape rect(sf::Vector2f(LADO_CASILLA, LADO_CASILLA));
				rect.setPosition(sf::Vector2f(me.getMyPos().x*LADO_CASILLA, me.getMyPos().y*LADO_CASILLA));
				rect.setFillColor(sf::Color::Transparent);
				rect.setOutlineThickness(5);
				rect.setOutlineColor(sf::Color::Yellow);
				window.draw(rect);
			}
		

		window.display();
	}
}

void gameEngine::ReceiveCommands() {
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
			//guardem id del msg
			uint16_t packetId;
			ims.Read(&packetId);

			if (!welcome) {
				cout << "benvingut" << endl;
				welcome = true;			

				//guardo la meva id
				uint8_t newId;
				ims.Read(&newId);
				me.id = newId;

				//setejo la meva pos
				uint8_t newX, newY;
				ims.Read(&newX);
				ims.Read(&newY);
				me.setMyPos(newX, newY);

				//Setejo la posicio dels altres
				uint8_t numOthers;
				ims.Read(&numOthers);
				for (int i = 0; i < (int)numOthers; i++) {
					ims.Read(&newId);
					ims.Read(&newX);
					ims.Read(&newY);
					Player temp(newX, newY, Color::Red, newId);

					others.push_back(temp);
				}
				//Obrir la mapa
				startGame();
			}		

			

			break;
		}
		case NEWPLAYER:
		{
			//guardem id del msg
			uint16_t packetId;
			ims.Read(&packetId);
			
			//Guardem id del player
			uint8_t newId;
			ims.Read(&newId);
			cout << (int)newId << endl;
			//ens guardem el nou jugador
			uint8_t newX, newY;
			ims.Read(&newX);
			ims.Read(&newY);			
			others.push_back(Player(newX, newY, Color::Red, newId));

			SendACK(packetId);

			break;
		}
		default:
			break;		
		}
	}

}
void gameEngine::SendACK(int msgId) {
	
	OutputMemoryStream oms;
	oms.Write((uint8_t)CommandType::ACK);
	oms.Write((uint16_t)msgId);

	//POSAR PARTIAL
	socket.send(oms.GetBufferPtr(), oms.GetLength(), ip, PORT);
	//I GUARDAR EN EL LLISTA PER ANAR REENVIANT
}

bool gameEngine::CheckIfNew(Player p2Check) {
	
}

