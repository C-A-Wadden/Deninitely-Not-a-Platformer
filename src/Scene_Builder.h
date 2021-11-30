#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

class Scene_Builder : public Scene
{
    struct PlayerConfig
    {
        float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
    };

protected:

    std::shared_ptr<Entity> m_player;
    std::vector<std::string> m_animations;
    std::vector<std::string> m_decorations;
    std::vector<std::string> m_pickups;
    std::vector<std::string> m_enemies;
    std::vector<std::string> m_enemyNames;
    std::vector<std::string> m_tileMenuText;
    std::vector<Vec2>       m_positions;
    std::string             m_level;
    PlayerConfig            m_playerConfig;
    const Vec2              m_gridSize = { 64, 64 };
    std::vector<sf::Text>   m_onScreenText;
    sf::Text                m_gridText;
    sf::Text                m_noticeText;
    bool                    m_dragging;
    bool                    m_addingTile;
    bool                    m_addingPickup;
    bool                    m_makeMoving;
    bool                    m_addingDec;
    bool                    m_addingEnemies;
    bool                    m_renderTile;
    bool                    m_aiSelection;
    bool                    m_patrolSelection;
    bool                    m_healthChange;
    bool                    m_damageChange;
    bool                    m_speedChange;
    std::shared_ptr<Entity> m_currentlyDragging;
    std::shared_ptr<Entity> m_currentlyAdding;
    int                     m_selectedIndex;
    int                     m_selectedMenuIndex;
    int                     m_screenOffset;

    void init(std::string level);

    void update();
    void onEnd();
    void sDoAction(const Action& action);
    void sDoMouseAction(const Action& action);
    void sRender();
    void readLevel();
    void removeAI();
    void drawLine(const Vec2& p1, const Vec2& p2);
    void SpawnPlayer();
    bool isMouseInside(std::shared_ptr<Entity> a, sf::Vector2i mouse);
    void save();
    void moveCamera(int xMove);
    Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
    Vec2 saveGrid(std::shared_ptr<Entity> entity);
    void midPixelToGrid(std::shared_ptr<Entity> entity);
    void changeTile(int indexChange, std::shared_ptr<Entity> entity);
    void changePickup(int indexChange, std::shared_ptr<Entity> entity);
    void changeDec(int indexChange, std::shared_ptr<Entity> entity);
    void changeEnemy(int indexChange, std::shared_ptr<Entity> entity);
    void makeMovingTile();
    void addFollow();
    void addPatrol();

public:

    Scene_Builder(GameEngine* gameEngine, std::string level);

};