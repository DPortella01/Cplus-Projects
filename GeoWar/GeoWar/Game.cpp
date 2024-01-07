#include "Game.h"
#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "Utilities.h"
#include <random>


namespace {
	std::random_device rd;
	std::mt19937 rng(rd());
}


const sf::Time Game::TIME_PER_FRAME = sf::seconds((1.f / 60.f));


Game::Game(const std::string& path) {

	// load the game configuration from file "path"
	loadConfigFromFile(path);

	// now that you have the config loaded you can create the RenderWindow
	m_window.create(sf::VideoMode(m_windowSize.x, m_windowSize.y), "GEX Engine");

	// set up stats text to display FPS
	m_statisticsText.setFont(m_font);
	m_statisticsText.setPosition(15.0f, 15.0f);
	m_statisticsText.setCharacterSize(15);

	// spawn the player
	spawnPlayer();
}


void Game::sUserInput() {

	// TODO read input from user
	//
	//    * update players CInput component for player movement
	//    * spawnBullet on mouse click in the direction of the mouse
	//    * pause/un-pause the game on Escape keypress
	//    * quit the game on  Q keypress

	if (!m_isPaused) {

		auto& uInput = m_player->getComponent<CInput>();

		sf::Event event;
		while (m_window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				m_window.close();
				m_isRunning = false;
			}


			// TODO handle key press events
			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {

				case sf::Keyboard::Up:
				case sf::Keyboard::W:
					uInput.up = true;
					break;

				case sf::Keyboard::Down:
				case sf::Keyboard::S:
					uInput.down = true;
					break;

				case sf::Keyboard::Left:
				case sf::Keyboard::A:
					uInput.left = true;
					break;

				case sf::Keyboard::Right:
				case sf::Keyboard::D:
					uInput.right = true;
					break;

				case sf::Keyboard::Escape:
					m_isPaused = !m_isPaused;
					break;

				case sf::Keyboard::Q:
					m_isRunning = false;
					break;

				case sf::Keyboard::G:
					m_drawBB = !m_drawBB;
					break;

				default:
					break;
				}
			}


			// TODO handle key released events
			if (event.type == sf::Event::KeyReleased) {
				switch (event.key.code) {

				case sf::Keyboard::Up:
				case sf::Keyboard::W:
					uInput.up = false;
					break;

				case sf::Keyboard::Down:
				case sf::Keyboard::S:
					uInput.down = false;
					break;

				case sf::Keyboard::Left:
				case sf::Keyboard::A:
					uInput.left = false;
					break;

				case sf::Keyboard::Right:
				case sf::Keyboard::D:
					uInput.right = false;
					break;

				default:
					break;
				}

			}


			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					// TODO spawnBullet with velocity in direction of mouse
					// see sf::event::MouseButtonEvent for details
					spawnBullet(sf::Vector2f(
						event.mouseButton.x,
						event.mouseButton.y
					));
				}
			}
		}
	}
	else {
		sf::Event event;
		while (m_window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				m_window.close();
				m_isRunning = false;
			}

			if (event.type == sf::Event::KeyPressed) {
				switch (event.key.code) {

				case sf::Keyboard::Escape:
					m_isPaused = !m_isPaused;
					break;

				case sf::Keyboard::Q:
					m_isRunning = false;
					break;

				default:
					break;
				}
			}
		}
	}
}


void Game::sUpdate(sf::Time dt) {

	// TODO pause the game if m_isPaused is true
	if (m_isPaused)
		return;


	// TODO spawn a new player if player was destroyed
	if (!m_player->isActive())
		m_player.reset();

	m_entityManager.update();

	if (m_player == nullptr)
		spawnPlayer();

	sEnemySpawner(dt);
	sLifespan(dt);
	sMovement(dt);
	sCollision();

}


void Game::sMovement(sf::Time dt) {

	// keep the player and enemies in bounds
	adjustPlayerPosition();
	keepObjecsInBounds();


	// TODO set the player velocity so it is moved  according to userInput
	// *Make sure the player always travels at the speed defined in m_playerConfig*
	sf::Vector2f pv;
	auto& pInput = m_player->getComponent<CInput>();
	if (pInput.left)
		pv.x -= 1;
	if (pInput.right)
		pv.x += 1;
	if (pInput.up)
		pv.y -= 1;
	if (pInput.down)
		pv.y += 1;

	pv = m_playerConfig.S * normalize(pv);
	m_player->getComponent<CTransform>().vel = pv;

	// move all the entities
	for (auto& e : m_entityManager.getEntities()) {

		// if it has a transform move it
		if (e->hasComponent<CTransform>() == true) {

			//e->getComponent<CTransform>().tx->move(dt.asSeconds() * e->getComponent<CTransform>().vel);
			auto& tfm = e->getComponent<CTransform>();


			// move entity
			tfm.pos += tfm.vel * dt.asSeconds();
			tfm.rot += tfm.rotSpeed * dt.asSeconds();
		}
	}
}


void Game::sRender() {

	// TODO have a different colour background to indicate the game is paused (200,200,255)
	if (m_isPaused)
	{
		m_window.clear(sf::Color(200, 200, 255));
	}
	else
	{
		m_window.clear(sf::Color(100, 100, 255));
	}


	for (auto& e : m_entityManager.getEntities()) {
		auto& tfm = e->getComponent<CTransform>();
		auto& shape = e->getComponent<CShape>().circle;
		shape.setPosition(tfm.pos);
		shape.setRotation(tfm.rot);

		// TODO fade fill color if e has a Clifespan component
		// the alpha should be the ratio of time remaining to total time
		if (e->hasComponent<CLifespan>())
		{
			auto& lifespan = e->getComponent<CLifespan>();
			sf::Color color = shape.getFillColor();

			float alpha = lifespan.remaining / lifespan.total;
			color.a = std::max(0.0f, alpha * 255);

			shape.setFillColor(color);
		}

		m_window.draw(shape);
	}


	if (m_drawBB)
		drawCR();

	sf::Text score("Score: " + std::to_string(m_score), m_font);
	score.setPosition(5, 30);
	m_window.draw(score);
	m_window.draw(m_statisticsText);
	m_window.display();
}


void Game::drawCR() {
	for (auto e : m_entityManager.getEntities()) {
		if (e->hasComponent<CCollision>()) {
			auto cr = e->getComponent<CCollision>().radius;
			auto trf = e->getComponent<CTransform>();
			sf::CircleShape bCirc;
			bCirc.setRadius(cr);
			centerOrigin(bCirc);

			bCirc.setPosition(trf.pos);
			bCirc.setFillColor(sf::Color(0, 0, 0, 0));
			bCirc.setOutlineColor(sf::Color(0, 255, 0));
			bCirc.setOutlineThickness(1.f);
			m_window.draw(bCirc);
		}
	}
}

void Game::run() {
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while (m_isRunning) {

		sUserInput();

		sf::Time elapsedTime = clock.restart();
		timeSinceLastUpdate += elapsedTime;
		while (timeSinceLastUpdate > TIME_PER_FRAME) {
			timeSinceLastUpdate -= TIME_PER_FRAME;
			sUserInput();
			sUpdate(TIME_PER_FRAME);
		}

		updateStatistics(elapsedTime);  // times per second world is rendered
		sRender();
	}
}


void
Game::loadConfigFromFile(const std::string& path) {

	std::ifstream config(path);
	if (config.fail()) {
		std::cerr << "Open file " << path << " failed\n";
		config.close();
		exit(1);
	}

	// TODO read the config file and set the game configuration
	// windowSize, font, playerConfig, enemyConfig, and bulletConfig
	std::string token{ "" };
	config >> token;
	while (!config.eof()) {
		if (token == "Window") {
			config >> m_windowSize.x >> m_windowSize.y;
		}
		else if (token == "Font") {
			std::string path;
			config >> path;
			if (!m_font.loadFromFile(path)) {
				std::cerr << "Failed to load font " << path << "\n";
				exit(-1);
			}
		}
		else if (token == "Player") {
			auto& pcf = m_playerConfig;

			config >> pcf.SR >> pcf.CR >> pcf.S >> pcf.AS
				>> pcf.FR >> pcf.FG >> pcf.FB
				>> pcf.OR >> pcf.OG >> pcf.OB
				>> pcf.OT >> pcf.V;
		}
		else if (token == "Enemy") {
			auto& ecf = m_enemyConfig;

			config >> ecf.SR >> ecf.CR >> ecf.SMIN >> ecf.SMAX
				>> ecf.OR >> ecf.OG >> ecf.OB >> ecf.OT
				>> ecf.VMIN >> ecf.VMAX >> ecf.L >> ecf.SI;
		}
		else if (token == "Bullet") {
			auto& bcf = m_bulletConfig;

			config >> bcf.SR >> bcf.CR >> bcf.S
				>> bcf.FR >> bcf.FG >> bcf.FB
				>> bcf.OR >> bcf.OG >> bcf.OB
				>> bcf.OT >> bcf.V >> bcf.L;
		}
		else if (token[0] == '#') {
			std::string tmp;
			std::getline(config, tmp);
			std::cout << tmp << "\n";
		}

		config >> token;
	}

	config.close();
}


void Game::updateStatistics(sf::Time dt) {
	m_statisticsUpdateTime += dt;
	m_statisticsNumFrames += 1;
	if (m_statisticsUpdateTime >= sf::seconds(1.0f)) {
		m_statisticsText.setString("FPS: " + std::to_string(m_statisticsNumFrames));
		m_statisticsUpdateTime -= sf::seconds(1.0f);
		m_statisticsNumFrames = 0;
	}

}


void Game::sCollision() {


	// TODO player collides with enemy
	// detect collision with large enemy (ignore small enemies for collisions with player)
	//      * destroy entity
	//      * destroy player (we will respawn destroyed player in update)
	//      * score -500 points for being killed

	for (auto& e : m_entityManager.getEntities("largeEnemy"))
	{
		auto& pRadius = m_player->getComponent<CCollision>().radius;
		auto& pPos = m_player->getComponent<CTransform>().pos;

		auto& eRadius = e->getComponent<CCollision>().radius;
		auto& ePos = e->getComponent<CTransform>().pos;

		float sumOfRadius = pRadius + eRadius;
		float distance = dist(pPos, ePos);

		if (distance < sumOfRadius)
		{
			e->destroy();

			m_player->destroy();
			m_score -= 500;
		}
	}

	// TODO collisions with bullets


	// * for each bullet

	//      * is the bullet colliding with a Large Enemy
	//          * destroy bullet
	//          * destroy enemy
	//          * add to score (CScore component from enemy tells you how many points)
	//          * spawn small enemies


	//      * is the bullet colliding with a small enemy
	//          * destroy the bullet
	//          * destroy the enemy
	//          * score the points

	for (auto& bullet : m_entityManager.getEntities("bullet"))
	{
		for (auto& enemy : m_entityManager.getEntities("largeEnemy"))
		{
			auto& bRadius = bullet->getComponent<CCollision>().radius;
			auto& bPos = bullet->getComponent<CTransform>().pos;

			auto& eRadius = enemy->getComponent<CCollision>().radius;
			auto& ePos = enemy->getComponent<CTransform>().pos;

			float sumOfRadius = bRadius + eRadius;
			float distance = dist(bPos, ePos);

			if (distance < sumOfRadius)
			{
				bullet->destroy();
				enemy->destroy();

				m_score += enemy->getComponent<CScore>().score;

				spawnSmallEnemies(enemy);
			}
		}

		for (auto& enemy : m_entityManager.getEntities("smallEnemy"))
		{
			auto& bRadius = bullet->getComponent<CCollision>().radius;
			auto& bPos = bullet->getComponent<CTransform>().pos;

			auto& eRadius = enemy->getComponent<CCollision>().radius;
			auto& ePos = enemy->getComponent<CTransform>().pos;

			float sumOfRadius = bRadius + eRadius;
			float distance = dist(bPos, ePos);

			if (distance < sumOfRadius)
			{
				bullet->destroy();
				enemy->destroy();

				m_score += enemy->getComponent<CScore>().score;
			}
		}
	}
}


void Game::keepObjecsInBounds() {

	auto vb = getViewBounds();

	// TODO Keep all enemy objects in bounds
	// if an object collides with a wall it should bounce off the wall
	for (auto& e : m_entityManager.getEntities())
	{
		auto& radius = e->getComponent<CCollision>().radius;
		auto& transform = e->getComponent<CTransform>();

		if (transform.pos.x - radius <= vb.left ||
			transform.pos.x + radius >= vb.left + vb.width)
		{
			transform.vel.x *= -1;
		}

		if (transform.pos.y - radius <= vb.top ||
			transform.pos.y + radius >= vb.top + vb.height)
		{
			transform.vel.y *= -1;
		}
	}
}


void Game::adjustPlayerPosition() {
	auto vb = getViewBounds();

	// TODO keep player in bounds
	auto& radius = m_player->getComponent<CCollision>().radius;
	auto& pos = m_player->getComponent<CTransform>().pos;

	pos.x = std::max(pos.x, vb.left + radius);
	pos.x = std::min(pos.x, vb.left + vb.width - radius);
	pos.y = std::max(pos.y, vb.top + radius);
	pos.y = std::min(pos.y, vb.top + vb.height - radius);
}


void Game::spawnPlayer() {

	// we will always spawn the player in the middle
	auto spawnPoint = sf::Vector2f(m_windowSize.x / 2.f, m_windowSize.y / 2.f);

	// TODO  Spawn player
	// create new player entity
	// assign new player entity to m_player
	// add all needed components
	// configure player according to m_playerConfig
	m_player = m_entityManager.addEntity("player");

	m_player->addComponent<CShape>(
		m_playerConfig.SR,                                                  // Shape radius
		m_playerConfig.V,                                                   // Vertices (points)
		sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), // Fill Color
		sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), // Outline Color
		m_playerConfig.OT                                                   // Outline Thickness
	);

	m_player->addComponent<CInput>();

	m_player->addComponent<CCollision>(m_playerConfig.CR);

	m_player->addComponent<CTransform>(
		spawnPoint,                     // position
		sf::Vector2f(0.0f, 0.0f),       // velocity
		m_playerConfig.SR               // angular velocity
	);
}


void Game::sLifespan(sf::Time dt) {

	// TODO for all entities that have a CLifespan compnent
	// reduce the remaining life by dt time.
	// if the lifespan has run out destroy the entity
	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CLifespan>())
		{
			auto& lifeSpawn = e->getComponent<CLifespan>();
			lifeSpawn.remaining -= dt;

			if (lifeSpawn.remaining <= sf::Time::Zero)
				e->destroy();
		}
	}
}


void Game::sEnemySpawner(sf::Time dt) {
	//
	// exponential distribution models random arrival times with an average
	// arrival interval of SI.
	// when the timer expires spawn a new enemy and re-set the timer for the
	// next enemy arrival time.
	std::exponential_distribution<float> exp(1.f / m_enemyConfig.SI);

	static sf::Time countDownTimer{ sf::Time::Zero };
	countDownTimer -= dt;
	if (countDownTimer < sf::Time::Zero) {
		countDownTimer = sf::seconds(exp(rng));
		spawnEnemy();
	}

}


void Game::spawnEnemy() {
	auto bounds = getViewBounds();
	std::uniform_real_distribution<float>   d_width(m_enemyConfig.CR, bounds.width - m_enemyConfig.CR);
	std::uniform_real_distribution<float>   d_height(m_enemyConfig.CR, bounds.height - m_enemyConfig.CR);
	std::uniform_int_distribution<>         d_points(m_enemyConfig.VMIN, m_enemyConfig.VMAX);
	std::uniform_real_distribution<float>   d_speed(m_enemyConfig.SMIN, m_enemyConfig.SMAX);
	std::uniform_int_distribution<>         d_color(0, 255);
	std::uniform_real_distribution<float>   d_dir(-1, 1);

	sf::Vector2f  pos(d_width(rng), d_height(rng));
	sf::Vector2f  vel = sf::Vector2f(d_dir(rng), d_dir(rng));
	vel = normalize(vel);
	vel = d_speed(rng) * vel;


	// TODO spawn a new enemy with random settings according to m_enemyConfig
	//  the CScore component will be the number of points the player gets for destroying this
	//  enenmy. It should be set to the number of Verticies the enemy has
	// tag is largeEnemy
	auto enemy = m_entityManager.addEntity("largeEnemy");

	int points = d_points(rng);

	int r = d_color(rng);
	int g = d_color(rng);
	int b = d_color(rng);

	enemy->addComponent<CShape>(
		m_enemyConfig.SR,
		d_points(rng),
		sf::Color(r, g, b),
		sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB),
		m_enemyConfig.OT
	);

	enemy->addComponent<CCollision>(m_enemyConfig.CR);
	enemy->addComponent<CScore>(points);
	enemy->addComponent<CTransform>(pos, vel);
}


void Game::spawnSmallEnemies(sPtrEntt e) {
	// TODO
	// Enemy entity e just collided with a bullet.
	// spawn small enemies according to game rules.
	//    * the radius of the small enemies is half the radius of the enemy that was hit
	//    * There are v small enemies where v is the number of vertices that the large enemy had
	//    * each small enemy has the same number of vertices and is the same colour as the large
	//          enemy, e, that was blown up.
	//    * the small enemies should all be moving away from the e.pos spread out equaly
	//           ie, if there e had 5 vertices there should be 5 small enemies each with 5 vertices
	//               with angle 360/5 degrees between each of them.
	//    *  Each small enemy has a CLifetime component with lifetime as specified in enemyConfig
	//    *  Each small enemy should have it's CScore component set to 10 times the number of
	//           vertices it has.
	//   tag is smallEnemy

	auto& circle = e->getComponent<CShape>().circle;
	float angle = 360.0f / circle.getPointCount();

	for (int i = 0; i < circle.getPointCount(); i++)
	{
		auto enemy = m_entityManager.addEntity("smallEnemy");

		enemy->addComponent<CShape>(
			circle.getRadius() / 2,
			circle.getPointCount(),
			circle.getFillColor(),
			circle.getOutlineColor(),
			circle.getOutlineThickness()
		);

		enemy->addComponent<CCollision>(e->getComponent<CCollision>().radius / 2);
		enemy->addComponent<CScore>(e->getComponent<CScore>().score * 10);

		auto& tfm = e->getComponent<CTransform>();

		sf::Vector2f dir = uVecBearing(i * angle);

		enemy->addComponent<CTransform>(tfm.pos, m_enemyConfig.SMAX * normalize(dir));
		enemy->addComponent<CLifespan>(m_enemyConfig.L);
	}
}


void Game::spawnBullet(sf::Vector2f mPos) {
	// TODO create a Bullet object
	// the bullet is spawned at the players location
	// the bullets velocity is in the direction of the mouse click location
	// the bullets config is according to m_bulletConfig
	// tag is "bullet"
	auto bullet = m_entityManager.addEntity("bullet");

	bullet->addComponent<CShape>(
		m_bulletConfig.SR,
		m_bulletConfig.V,
		sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB),
		sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB),
		m_bulletConfig.OT
	);

	bullet->addComponent<CCollision>(m_bulletConfig.CR);

	auto pPos = m_player->getComponent<CTransform>().pos;
	mPos -= pPos;

	bullet->addComponent<CTransform>(
		pPos,
		m_bulletConfig.S * normalize(mPos));

	bullet->addComponent<CLifespan>(m_bulletConfig.L);
}


void Game::spawnSpecialWeapon() {

}


// convenience function to return the view bounds as a FloatRect
sf::FloatRect Game::getViewBounds() {
	auto view = m_window.getView();
	return sf::FloatRect(
		(view.getCenter().x - view.getSize().x / 2.f), (view.getCenter().y - view.getSize().y / 2.f),
		view.getSize().x, view.getSize().y);
}
