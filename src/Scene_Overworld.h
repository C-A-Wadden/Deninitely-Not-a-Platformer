#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

class Scene_Overworld : public Scene
{
    struct PlayerConfig
    {
        float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
    };

protected:

    std::shared_ptr<Entity> m_player;
    int                     m_level;
    PlayerConfig            m_playerConfig;
    float                   m_playerSize;
    int                     m_currentLocation;
    int                     m_currentLevel;
    bool                    m_godMode;
    sf::Text                m_menuText;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    bool                    m_drawGrid = false;
    bool                    m_movePlayerRight;
    bool                    m_movePlayerLeft;
    const Vec2              m_gridSize = { 64, 64 };
    sf::Text                m_gridText;

    float                   m_slideAmount = 0;
    bool                    m_playerHasWallJumped = true;
    bool                    m_playerIsGrounded = false;
    sf::Text                m_timerText;

    std::vector<std::vector<int>>       m_locations;
    std::vector<std::vector<int>>       m_levels;
    std::vector<std::string>            m_paths;
    std::vector<std::string>            m_levelNames;

    // Screen Shake
    int m_frameToStopScreenShake = 0;
    Vec2 m_currentViewOffset = Vec2(0, 0);
    int m_frameRate = 60;
    int m_screenShakeIntensity = 0;

    void init();

    void update();
    void onEnd();
    void sDoAction(const Action& action);
    void sDoMouseAction(const Action& action);
    void sRender();
    void SpawnPlayer();
    void movePlayerRight();
    void movePlayerLeft();
    void drawDots();
    void save();
    void newFile();

public:

    Scene_Overworld(GameEngine* gameEngine);

};