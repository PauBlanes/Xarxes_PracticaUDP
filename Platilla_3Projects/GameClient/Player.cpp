#include "Player.h"

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



Player::Player()
{
	myColor = sf::Color::Yellow;
	speed = 5;
}

Player::Player(int16_t x, int16_t y, sf::Color col, uint8_t myId)
{
	position = { (float)x,(float)y };	
	myColor = col;
	id = myId;
	speed = 5;
	
	
}


Player::~Player()
{

}

sf::CircleShape Player::Draw(sf::RenderWindow* window, bool interpolate) {
	
	
	sprite.setRadius(RADIO_AVATAR);
	sprite.setFillColor(myColor);
	//sf::Vector2f M_posicion(position.x,position.y);
	//position = BoardToWindows(position);
	//if (!interpolate || InterpPositions.empty()) {
		sprite.setPosition(position);
	//}
	//else {		
		//sprite.setPosition(InterpPositions[interpIndex]);	
		//if (lerpIndex < InterpPositions.size()-1)
			lerpIndex += 1;
		cout << lerpIndex << " : " << (int)myColor.r << (int)myColor.g << (int)myColor.b << endl; //PQ SOLO SE SUMA EL LERPINDEX EN EL JUGADOR PRINCIPAL(EL YELLOW)????
	//}	

	window->draw(sprite);

	return sprite;
}

void Player::setMyPos(int16_t x, int16_t y) {
	position = {(float)x,(float)y};	
	if (!activated)
		activated = true;
};
Vector2f Player::getMyPos() {
	return position;
};

sf::Vector2f Player::BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}

bool Player::receivePos() {
	return activated;
}

void Player::setMyName(string name) {
	myName = name;
}

string Player::getMyName() {
	return myName;
}

void Player::CreateLerpPath(int16_t dX, int16_t dY) {
	Vector2f start = position;
	Vector2f end(position.x + dX, position.y + dY);	
	
	InterpPositions.clear();

	for (int i = 0; i < 10; i++) { //donava problemes amb fer-ho amb float i
		float step = i / 10;
		Vector2f temp = (1 - step)*start + end * step;
		InterpPositions.push_back(temp);		
	}
	
	//index = 0;
	
}


