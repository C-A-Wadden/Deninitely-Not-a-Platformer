#include "AI.h"
#include "GameEngine.h"
#include "Physics.h"
#include "EntityManager.h"

AI::AI(std::shared_ptr<Entity> e, std::shared_ptr<Entity> p, GameEngine* g, EntityManager em)
    : entity(e),
      m_player(p),
      m_game(g),
      m_entityManager(em)
{

}

void AI::update(bool playerInvis)
{
    m_playerIsInvis = playerInvis;
    auto& state = entity->getComponent<CState>().state;
    auto& anim = entity->getComponent<CAnimation>().animation; 
    //if (anim.getName() != entity->tag() + state) anim = m_game->assets().getAnimation(entity->tag() + state);

    if (state == "Death")
    {
        die(state, anim);
        return;
    }
    if (state == "Atk" || state == "Tele0" || state == "Tele1") return; // finish all atk/tele anim
    if (m_player->getComponent<CTransform>().pos.dist(entity->getComponent<CTransform>().pos) < entity->getComponent<CDamage>().atkRange && !m_playerIsInvis) //Attack range
    {
        state = "Atk";
    }
    else
    {
        entity->getComponent<CBoundingBox>().size.x = 40;
        entity->getComponent<CBoundingBox>().halfSize.x = 20;
        state = "Walk";
    }
    
    
         if (state == "Atk" && (entity->tag() == "Boss"))              teleport();
    else if (state == "Atk")                                           attack();
         if (entity->hasComponent<CPatrol>() && state == "Walk")       movePatrol();
    else if (entity->tag() == "GoomBot" && state == "Walk")            moveFollow();
    else if (entity->tag() == "Guard" && state == "Walk")              moveGuard();
    else if (entity->tag() == "Boss" && state == "Walk")               moveFollow();

    
    // Set animation based on state and scale(dir) based on velocity
         //if (entity->tag() == "Boss") state = "Wake";
    
    if (anim.getName() != entity->tag() + state) anim = m_game->assets().getAnimation(entity->tag() + state);
    
}

void AI::teleport()
{
    entity->getComponent<CTransform>().velocity = Vec2(0, 0);
    auto& state = entity->getComponent<CState>().state;
    auto& anim = entity->getComponent<CAnimation>().animation;
    state = "Tele0";
    if (anim.getName() != entity->tag() + state) anim = m_game->assets().getAnimation(entity->tag() + state);
    entity->getComponent<CAnimation>().repeat = false;
}

void AI::attack()
{
    entity->getComponent<CAnimation>().repeat = false;
    
    // warp behind and face player 
    if (entity->tag() == "Guard")
    {
        entity->getComponent<CTransform>().pos = Vec2(m_player->getComponent<CTransform>().pos.x + -50 * m_player->getComponent<CTransform>().scale.x, m_player->getComponent<CTransform>().pos.y);
        auto eDir = m_player->getComponent<CTransform>().pos.x - entity->getComponent<CTransform>().pos.x;
        if (eDir > 0)
        {
            entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x);
        } 
        else
        {
            entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x)*-1;
        }
    }

    if (entity->tag() == "Boss")
    {
        
    }
    else
    {
        entity->getComponent<CBoundingBox>().size.x = (entity->getComponent<CAnimation>().animation.getFrame() + 11) * 10;
        entity->getComponent<CBoundingBox>().halfSize.x = entity->getComponent<CBoundingBox>().size.x / 2;
        entity->getComponent<CTransform>().velocity.x = 0;
    }
}

void AI::die(std::string state, Animation& anim)
{
    entity->removeComponent<CDamage>();
    if (anim.getName() != entity->tag() + state) anim = m_game->assets().getAnimation(entity->tag() + state);
    entity->getComponent<CAnimation>().repeat = false;
    entity->getComponent<CTransform>().velocity = Vec2(0,0);
}

void AI::movePatrol()
{
    auto& patrol = entity->getComponent<CPatrol>();
    int target = patrol.goalPosition;
    auto targetPos = patrol.positions[target];
    auto ePos = entity->getComponent<CTransform>().pos;
    if (entity->getComponent<CTransform>().velocity == Vec2(0, 0))
    {
        Vec2 d = targetPos - ePos;
        auto l = sqrtf(d.x * d.x + d.y * d.y);
        Vec2 N = { d.x / l, d.y / l };
        entity->getComponent<CTransform>().velocity.x = patrol.speed * N.x;
    }
    // if at target patrol point set vector to next point
    if (abs(targetPos.x - ePos.x) < 10)
    {
        entity->getComponent<CTransform>().velocity = Vec2(0, 0);
        patrol.prevPosition = patrol.goalPosition;
        patrol.goalPosition = (patrol.goalPosition + 1) % patrol.positions.size(); // % to loop points
    }

    if (entity->getComponent<CTransform>().velocity.x < 0) entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x )*-1;
    else entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x);
}

void AI::moveFollow()
{
    auto& state = entity->getComponent<CState>().state;
    auto ePos = entity->getComponent<CTransform>().pos;
    const auto& follow = entity->getComponent<CFollowPlayer>();
    bool canSee = true;
    if (m_playerIsInvis) 
    {
        canSee = false;
    }
    else 
    {
        for (auto& b : m_entityManager.getEntities())
        {
            if (b != m_player)
            {
                if (Physics::EntityIntersect(entity->getComponent<CTransform>().pos, m_player->getComponent<CTransform>().pos, b))
                {
                    canSee = false;
                    break; // exit loop if obstruction found
                }
            }
        }
    }
    //std::cout << entity->getComponent<CTransform>().velocity.x << " " << entity->getComponent<CTransform>().pos.x << std::endl;
    Vec2 targetPos;
    if (canSee) // vector to player
    {
        targetPos = m_player->getComponent<CTransform>().pos;
    }
    else // vector to home
    {
        targetPos = follow.home;
    }

    if (abs(ePos.x - targetPos.x) > 1)
    {
        auto ePos = entity->getComponent<CTransform>().pos;
        Vec2 d = targetPos - ePos;
        auto l = sqrtf(d.x * d.x + d.y * d.y);
        Vec2 N = { d.x / l, d.y / l };
        entity->getComponent<CTransform>().velocity.x = follow.speed * N.x;
        state = "Walk";
    }
    else
    {
        entity->getComponent<CTransform>().velocity = Vec2(0, 0); // stop moving if home
        state = "Idle";
    }

    if (entity->getComponent<CTransform>().velocity.x < 0) entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x) * -1;
    else entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x);
}

void AI::moveGuard()
{
    auto& state = entity->getComponent<CState>().state;
    auto ePos = entity->getComponent<CTransform>().pos;
    const auto& follow = entity->getComponent<CFollowPlayer>();
    auto pFacing = m_player->getComponent<CTransform>().scale.x;
    auto eDir = m_player->getComponent<CTransform>().pos.x - entity->getComponent<CTransform>().pos.x;
    bool canSee = true;
    if ((pFacing < 0 && eDir > 0) || (pFacing > 0 && eDir < 0) || m_playerIsInvis)
    {
        canSee = false;
    }
    else
    {
        for (auto& b : m_entityManager.getEntities())
        {

            if (b != m_player)
            {
                if (Physics::EntityIntersect(entity->getComponent<CTransform>().pos, m_player->getComponent<CTransform>().pos, b))
                {
                    canSee = false;
                    break; // exit loop if obstruction found
                }
            }
        }
    }
    Vec2 targetPos;
    if (canSee) // vector to player
    {
        targetPos = m_player->getComponent<CTransform>().pos;
    }
    else // vector to home
    {
        targetPos = follow.home;
    }

    if (abs(ePos.x - targetPos.x) > 1)
    {
        auto ePos = entity->getComponent<CTransform>().pos;
        Vec2 d = targetPos - ePos;
        auto l = sqrtf(d.x * d.x + d.y * d.y);
        Vec2 N = { d.x / l, d.y / l };
        entity->getComponent<CTransform>().velocity.x = follow.speed * N.x;
        entity->getComponent<CTransform>().velocity.y = follow.speed * N.y;
        state = "Walk";
    }
    else
    {
        entity->getComponent<CTransform>().velocity = Vec2(0, 0); // stop moving if home
        state = "Walk";
    }

    if (entity->getComponent<CTransform>().velocity.x < 0) entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x) * -1;
    else entity->getComponent<CTransform>().scale.x = abs(entity->getComponent<CTransform>().scale.x);
};
