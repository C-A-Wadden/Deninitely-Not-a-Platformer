#pragma once
#include "Vec2.h"
#include "EntityManager.h"

class Pathfind
{
public:
	Pathfind(EntityManager em);

	EntityManager m_entityManager;


	Vec2 steer(Vec2 vel, Vec2 tar, Vec2 cur, float scale);
	Vec2 getTarget(Vec2 playerPos, Vec2 entityPos);
};