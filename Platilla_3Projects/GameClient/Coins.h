
#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

#define INTERPOLATION_STEPS 10
/*
struct POSITION
{
	uint8_t x, y;
};
*/
class Coins
{
private:
	Vector2f position;
	sf::Color myColor;
	sf::CircleShape sprite;

public:
	uint8_t id;

	Coins();
	Coins(int16_t x, int16_t y, uint8_t);
	~Coins();

	sf::CircleShape Draw(sf::RenderWindow*);
	void setPos(int16_t, int16_t);
	Vector2f getPos();

	sf::Vector2f BoardToWindows(sf::Vector2f);
};
