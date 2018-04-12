#pragma once
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

using namespace std;

struct POSITION
{
	uint8_t x, y;
};

class Player
{
private:
	POSITION position;
	sf::CircleShape sprite;
	sf::Color myColor;
	string  myName;
public:
	Player();
	Player(uint8_t x, uint8_t y, sf::Color myColor);
	~Player();

	bool activated;
	sf::CircleShape Draw(sf::RenderWindow*);
	void setMyPos(uint8_t, uint8_t);
	POSITION getMyPos();
	sf::Vector2f BoardToWindows(sf::Vector2f);
	bool receivePos();
	void setMyName(string);
	string getMyName();
	
};

