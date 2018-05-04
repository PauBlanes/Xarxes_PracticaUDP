#include "Coins.h"

#define MAX 100
#define SIZE_TABLERO 64
#define SIZE_FILA_TABLERO 8
#define LADO_CASILLA 64
#define RADIO_AVATAR 25.f
#define OFFSET_AVATAR 5




Coins::Coins()
{
	myColor = sf::Color::Blue;
}

Coins::Coins(int x, int y, int myId)
{
	myColor = sf::Color::Color(255,153,0);
	position = { (float)x,(float)y };
	id = myId;
}


Coins::~Coins()
{
}

sf::CircleShape Coins::Draw(sf::RenderWindow* window)
{
	sprite.setRadius(RADIO_AVATAR);
	sprite.setFillColor(myColor);
	
	sprite.setPosition(position);
	
	window->draw(sprite);

	return sprite;
}


void Coins::setPos(int x, int y)
{
	position = { (float)x,(float)y };
}

Vector2f Coins::getPos()
{
	return position;
}


sf::Vector2f Coins::BoardToWindows(sf::Vector2f _position)
{
	return sf::Vector2f(_position.x*LADO_CASILLA + OFFSET_AVATAR, _position.y*LADO_CASILLA + OFFSET_AVATAR);
}
