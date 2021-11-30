#include "Pathfind.h"
#include <math.h>
#include "Common.h"

Pathfind::Pathfind(EntityManager em)
	: m_entityManager (em)
{

}


Vec2 Pathfind::steer(Vec2 vel, Vec2 tar, Vec2 cur, float scale)
{
	 /* 1. desired = target – pos
		2. desired = desired / desired.length
		3. desired = desired * vel.length
		4. steering = desired – vel
		5. steering = steering * scale
		6. actual = vel + steering*/

	Vec2 d = tar - cur;
	d /= sqrtf(d.x*d.x + d.y*d.y);
	d *= 5; // sqrtf(vel.x * vel.x + vel.y * vel.y);
	d -= vel;
	d *= scale;
	return (vel + d);
}

Vec2 Pathfind::getTarget(Vec2 playerPos, Vec2 entityPos)
{
	std::vector<std::vector<int>> terrain;
	terrain.resize(150, std::vector<int>(12, 0));

	for (auto tile : m_entityManager.getEntities("tile"))
	{
		auto pos = tile->getComponent<CTransform>().homePos;
		terrain[(int)pos.x][(int)pos.y] = 1;
	}

	std::vector< std::vector<int> > distMap;
	distMap.resize(150, std::vector<int>(12, -1));

	distMap[(int)playerPos.x][(int)playerPos.y] = 0;

	for (int y = 11; y >= 0; y--)
	{
		for (int x = 0; x < 150; x++)
		{
			if (terrain[x][y] > 0) distMap[x][y] = -2;
		}
	}

	int done = 0;
	while (done<100)
	{
		for (int y = 11; y >= 0; y--)
		{
			for (int x = 0; x < 150; x++)
			{
				if (distMap[x][y] != -1)
				{
					if (distMap[x][y] == -2) continue;
					//check 8 dir and set to cur+1 if passable and unmarked
					for (int i = -1; i <= 1; i++)
					{
						for (int j = -1; j <= 1; j++)
						{
							int ni = i; int nj = j;
							if (x+i > 149 || x+i < 0) ni = 0;
							if (y+j > 11  || y+j < 0) nj = 0;

							if (distMap[x+ni][y+nj] == -1)
							{
								distMap[x+ni][y+nj] = distMap[x][y] + 1;
							}
						}
					}
				}
			}
		}
		done++;
	}
	//prefer the middle of the screen
	for (int y = 11; y > -1; y--)
	{
		for (int x = 0; x < 150; x++)
		{
			/*if (distMap[x][y] < 10 && distMap[x][y] > 0) std::cout << 0;
			std::cout << distMap[x][y];
			std::cout << " ";*/
			if (distMap[x][y] > 0) distMap[x][y] += abs(6 - y)/4;
		}
		//std::cout << std::endl;
	}

	Vec2 target = { };
	int minDist = 10000;
	for (int i = -1; i <= 1; i++)
	{
		for (int j = -1; j <= 1; j++)
		{
			int ni = i; int nj = j;
			if (entityPos.x + i > 149 || entityPos.x + i < 0) ni = 0;
			if (entityPos.y + j > 11 || entityPos.y + j < 0) nj = 0;
			if (ni == 0 && nj == 0) continue;
			
			if (distMap[entityPos.x + ni][entityPos.y + nj] < minDist && distMap[entityPos.x + ni][entityPos.y + nj] > 0)
			{
				minDist = distMap[entityPos.x + ni][entityPos.y + nj];
				target = { entityPos.x + ni, entityPos.y + nj };
			}
			else if (distMap[entityPos.x + ni][entityPos.y + nj] == minDist)
			{
				if (abs((entityPos.y + ni) - playerPos.y) < abs( target.y - playerPos.y))
				{
					target = { entityPos.x + ni, entityPos.y + nj };
				}
			}
		}
	}
	return target;
}
