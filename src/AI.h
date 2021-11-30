#pragma once
#include "Common.h"
#include "EntityManager.h"
#include "GameEngine.h"

class AI
{
public:

	std::shared_ptr<Entity> entity;
	std::shared_ptr<Entity> m_player;
	GameEngine* m_game;
	EntityManager m_entityManager;
	bool m_playerIsInvis = false;

	AI(std::shared_ptr<Entity> e, std::shared_ptr<Entity> p, GameEngine* g, EntityManager em);

	void update(bool playerInvis);
	void teleport();
	void attack();
	void die(std::string state, Animation& anim);
	void movePatrol();
	void moveFollow();
	void moveGuard();
};