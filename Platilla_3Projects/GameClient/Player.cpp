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
}

Player::Player(uint8_t x, uint8_t y, sf::Color col)
{
	position = { x,y };
	myColor = col;
}


Player::~Player()
{

}

sf::CircleShape Player::Draw(sf::RenderWindow* window) {
	
	sprite.setRadius(RADIO_AVATAR);
	sprite.setFillColor(myColor);
	sf::Vector2f M_posicion(position.x,position.y);
	M_posicion = BoardToWindows(M_posicion);
	sprite.setPosition(M_posicion);

	window->draw(sprite);

	return sprite;
}

void Player::setMyPos(uint8_t x, uint8_t y) {
	position = {x,y};
	
	if (!activated)
		activated = true;
};
POSITION Player::getMyPos() {
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
