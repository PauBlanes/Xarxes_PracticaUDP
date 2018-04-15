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

struct POSITION
{
	uint8_t x, y;
};

class Player
{
private:
	Vector2f position;
	sf::CircleShape sprite;
	sf::Color myColor;
	string  myName;	
	
public:
	Player();
	Player(uint8_t x, uint8_t y, sf::Color myColor, uint8_t);
	~Player();

	float speed;
	bool activated;
	sf::CircleShape Draw(sf::RenderWindow*);
	void setMyPos(uint8_t, uint8_t);
	Vector2f getMyPos();
	sf::Vector2f BoardToWindows(sf::Vector2f);
	bool receivePos();
	void setMyName(string);
	string getMyName();
	uint8_t id;
	
};

