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

struct POSITION
{
	int x, y;
};

class Player
{
private:
	Vector2f position;
	sf::CircleShape sprite;
	sf::Color myColor;
	string  myName;	
	int score;
	int lerpIndex;
public:
	Player();
	Player(int x, int y, sf::Color myColor, int);
	~Player();

	float speed;
	bool activated;
	sf::CircleShape Draw(sf::RenderWindow*, bool);
	void setMyPos(int, int);
	Vector2f getMyPos();
	sf::Vector2f BoardToWindows(sf::Vector2f);
	bool receivePos();
	void setMyName(string);
	string getMyName();
	int id;

	//score
	void addScore();
	int getScore();

	//Per fer interpolacio
	vector<Vector2f> InterpPositions;
	void CreateLerpPath(int, int);
	
};

