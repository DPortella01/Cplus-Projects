////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
//  Assignment:      
//  Instructor:     David Burchill
//  Year / Term:    Fall 2023
//  File name:      Source.cpp
// 
//  Student name:   Diego Portella
//  Student email:  portella.sdmf@hotmail.com
// 
//     I certify that this work is my work only, any work copied from Stack Overflow, textbooks, 
//     or elsewhere is properly cited. 
// 
// ////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  BUG 
//  list any and all bugs in your code 
//  1. I didnt get abc to work because of xyz, I tried ijk...
//

 
#include <fstream>
#include <SFML/Graphics.hpp> 
#include <iostream>
#include <memory> // for using smarter pointer
#include "Utilities.h"

struct Config
{
	sf::Vector2u winSize;
	std::string fontName;
	unsigned int fontSize;
	sf::Color fontColor;

	std::vector<std::string> circName;
	std::vector<sf::Vector2f> circPos;
	std::vector<sf::Color> circColor;
	std::vector<sf::Vector2f> circVel;
	std::vector<float> circRad;

	std::vector<sf::String> rectName;
	std::vector<sf::Vector2f> rectPos;
	std::vector<sf::Color> rectColor;
	std::vector<sf::Vector2f> rectVel;
	std::vector<sf::Vector2f> rectSize;
};

struct Shape {
	// member
	std::string name;
	sf::Shape* shape{ nullptr };
	sf::Vector2f vel{ 0.f, 0.f };

	// This part is for copying
	Shape(const Shape&) = delete;
	Shape& operator=(const Shape&) = delete;

	// This function is for moving
	Shape(Shape&& rv) : shape(rv.shape), vel(rv.vel), name(rv.name) {
		rv.shape = nullptr;
	}

	// RAII
	Shape(sf::Shape* s) : shape(s) {}
	~Shape() {
		if (shape) {
			delete shape;
			shape = nullptr; //double deleting
		}
	}

	float top() { return shape->getGlobalBounds().top; }
	float bot() { return shape->getGlobalBounds().top + shape->getGlobalBounds().height; }
	float left() { return shape->getGlobalBounds().left; }
	float right() { return shape->getGlobalBounds().left + shape->getGlobalBounds().width; }
};

Config readConfig(std::string path);

int main()
{
	const auto config = readConfig("config.txt");

	sf::RenderWindow window(sf::VideoMode(config.winSize.x, config.winSize.y), "SFML works!");

	sf::Font myFont;

	if (!myFont.loadFromFile(config.fontName)) {
		std::cerr << "Fail to open font \n";
		exit(1);
	}

	//stats
	sf::Text statisticsTexts;
	sf::Time statisticsUpdateTime{ sf::Time::Zero };
	unsigned int statisticsNumFrames{ 0 };

	statisticsTexts.setFont(myFont);
	statisticsTexts.setString("FPS: " + std::to_string(statisticsNumFrames));
	statisticsTexts.setPosition(15.f, 15.f);

	std::vector<Shape> entities;

	for (int i = 0; i < config.circName.size(); i++)
	{
		Shape shape(new sf::CircleShape(config.circRad[i]));
		shape.shape->setFillColor(config.circColor[i]);
		centerOrigin(*(shape.shape));
		shape.shape->setPosition(config.circPos[i]);
		shape.vel = config.circVel[i];
		shape.name = config.circName[i];
		entities.push_back(std::move(shape));
	}

	for (int i = 0; i < config.rectName.size(); i++)
	{
		Shape shape(new sf::RectangleShape(config.rectSize[i]));
		shape.shape->setFillColor(config.rectColor[i]);
		centerOrigin(*(shape.shape));
		shape.shape->setPosition(config.rectPos[i]);
		shape.vel = config.rectVel[i];
		shape.name = config.rectName[i];
		entities.push_back(std::move(shape));
	}

	//Game Loop
	static const sf::Time TIME_PER_FRAME = sf::seconds(1.f / 60.f);
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;



	/////////////////////////
	// GET INPUT
	/////////////////////////
	while (window.isOpen()) {

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		sf::Time elapsedTime = clock.restart();
		timeSinceLastUpdate += elapsedTime;


		/////////////////////////
		// UPDATE THE WORLD
		/////////////////////////
		while (timeSinceLastUpdate > TIME_PER_FRAME) {

			timeSinceLastUpdate -= TIME_PER_FRAME;
			float dt = TIME_PER_FRAME.asSeconds();

			// collisions with walls
			for (auto& s : entities) {

				s.shape->move(s.vel);

				if (s.left() < 0.f or s.right() > config.winSize.x)
					s.vel.x *= -1.f;

				if (s.top() < 0.f or s.bot() > config.winSize.y)
					s.vel.y *= -1.f;
			}
		}
		/////////////////////////
		// DRAW WORLD
		/////////////////////////
		window.clear(sf::Color(100, 100, 255));  // clear back buffer

		sf::Text t("name", myFont, 20);


		/////////////////////////
		// DRAW THE WORLD
		/////////////////////////
		for (auto& s : entities) {
			window.draw(*(s.shape));

			t.setString(s.name);
			t.setCharacterSize(config.fontSize);
			t.setFillColor(config.fontColor);
			//centralizando o texto no meio dos shapes
			centerOrigin(t);
			t.setPosition(s.shape->getPosition());
			window.draw(t);
		}
		window.draw(statisticsTexts);

		//This serves as the end of the frame
		window.display();


		//update statistics
		statisticsUpdateTime += elapsedTime;
		statisticsNumFrames += 1;
		if (statisticsUpdateTime >= sf::seconds(1.0f)) {
			statisticsTexts.setString("FPS: " + std::to_string(statisticsNumFrames));
			statisticsUpdateTime -= sf::seconds(1.0f);
			statisticsNumFrames = 0;

		}

	}

	return 0;

}

Config readConfig(std::string path)
{
	Config c;

	std::ifstream confFile(path);
	if (confFile.fail()) {
		std::cerr << "Open file: " << path << " failed\n";
		confFile.close();
		exit(1);
	}

	std::string token{ "" };
	confFile >> token;
	while (!confFile.eof()) {
		if (token == "Window") {
			confFile >> c.winSize.x >> c.winSize.y;
		}
		else if (token == "Font") {
			confFile >> c.fontName >> c.fontSize;

			int r, g, b;
			confFile >> r >> g >> b;
			c.fontColor = sf::Color(r, g, b);
		}
		else if (token == "Circle") {
			std::string name;
			confFile >> name;
			c.circName.push_back(name);

			sf::Vector2f pos;
			confFile >> pos.x >> pos.y;
			c.circPos.push_back(pos);

			int r, g, b;
			confFile >> r >> g >> b;
			c.circColor.push_back(sf::Color(r, g, b));

			sf::Vector2f vel;
			confFile >> vel.x >> vel.y;
			c.circVel.push_back(vel);

			float rad;
			confFile >> rad;
			c.circRad.push_back(rad);
		}
		else if (token == "Rect") {
			std::string name;
			confFile >> name;
			c.rectName.push_back(name);

			sf::Vector2f pos;
			confFile >> pos.x >> pos.y;
			c.rectPos.push_back(pos);

			int r, g, b;
			confFile >> r >> g >> b;
			c.rectColor.push_back(sf::Color(r, g, b));

			sf::Vector2f vel;
			confFile >> vel.x >> vel.y;
			c.rectVel.push_back(vel);

			sf::Vector2f size;
			confFile >> size.x >> size.y;
			c.rectSize.push_back(size);
		}
		else if (token[0] == '#') {
			std::string discard;
			std::getline(confFile, discard);
		}
		else {
			std::cerr << "Unknown asset type: " << token << std::endl;
		}
		confFile >> token;
	}
	return c;
}