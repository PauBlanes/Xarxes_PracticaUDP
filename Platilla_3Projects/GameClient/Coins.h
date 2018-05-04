
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

class Coins
{
private:
	Vector2f position;
	sf::Color myColor;
	sf::CircleShape sprite;

public:
	int id;

	Coins();
	Coins(int x, int y, int);
	~Coins();

	sf::CircleShape Draw(sf::RenderWindow*);
	void setPos(int, int);
	Vector2f getPos();

	sf::Vector2f BoardToWindows(sf::Vector2f);
};
