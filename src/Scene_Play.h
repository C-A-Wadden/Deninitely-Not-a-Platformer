#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>
#include <array>
#include "EntityManager.h"

class Scene_Play : public Scene
{
    struct PlayerConfig
    {
        float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
        
        PlayerConfig()
        {
            X = 0;
            Y = 0;
            CX = 0;
            CY = 0;
            SPEED = 0;
            MAXSPEED = 0;
            JUMP = 0;
            GRAVITY = 0;
        }

        PlayerConfig(const PlayerConfig& pc)
        {
            X = pc.X;
            Y = pc.Y;
            CX = pc.CX;
            CY = pc.CY;
            SPEED = pc.SPEED;
            MAXSPEED = pc.MAXSPEED;
            JUMP = pc.JUMP;
            GRAVITY = pc.GRAVITY;
        }
    };

    struct WeaponConfig
    {
        int currentWeapon, numWeapons;
        std::vector<int>ammo = { -1, 0, 0, 0 };
    };

    struct RaycastTarget
    {
        float angle = 0.0f;
        Vec2 pos = Vec2(0, 0);
        bool moving = false;

        RaycastTarget() {};
    };



    // https://www.youtube.com/watch?v=fc3nnG2CG8U&t=1430s&ab_channel=javidx9
    struct Edge
    {
        Vec2 startPoint;
        Vec2 endPoint;

        Edge(Vec2 start, Vec2 end)
        {
            startPoint = start;
            endPoint = end;
        }
    };
    struct StaticTileGridCell
    {
        bool exist = false;
        std::array<bool, 4> edgeExist{ {false, false, false, false} };
        std::array<int, 4> edgeID{ {-1, -1, -1, -1} };
        StaticTileGridCell() {}
    };


protected:

    std::shared_ptr<Entity> m_player;
    std::shared_ptr<Entity> m_cameraTarget;
    std::shared_ptr<Entity> m_control;
    std::shared_ptr<Entity> m_movingPlatPlayerRiding;
    std::string             m_levelPath;
    PlayerConfig            m_playerConfig;
    PlayerConfig            m_basePlayerConfig;
    WeaponConfig            m_weaponConfig;
    bool                    m_drawTextures = true;
    bool                    m_drawCollision = false;
    bool                    m_drawGrid = false;
    const Vec2              m_gridSize = { 64, 64 };
    sf::Text                m_gridText;

    float                   m_playerSize;
    float                   m_slideAmount = 0;
    bool                    m_playerHasWallJumped = true;
    bool                    m_playerIsGrounded = false;
    sf::Text                m_timerText;
    int                     m_timerOffset = 0;
    sf::Text                m_deathCountText;
    int                     m_deathCount = 0;
    std::vector<sf::Text>   m_onScreenText;
    sf::RectangleShape      m_healthBar;
    sf::RectangleShape      m_healthBarBackground;
    bool                    m_smoothedCamera = false;
    float                   m_xCenter;
    std::vector<float>      m_camStartX;
    int                     m_statusTimer = 0;
    bool                    m_playerIsInvis = false;
    bool                    m_playerIsFast = false;
    bool                    m_playerIsBig = false;
    int                     m_playerEnlargementCount = 0;
    bool                    m_won = false;

    Vec2                    m_levelBoundaries = Vec2(0, 0);
    std::vector<std::array<StaticTileGridCell, 12>>   m_staticTileLocations;
    std::vector<Vec2>       m_staticTileGridInfo;
    std::vector<Edge>       m_levelEdges;
    std::vector<sf::CircleShape> m_cornerDisplayCircles;
    std::vector<sf::RectangleShape> m_edgeDisplayRectangles;
    Vec2                    m_goalPos = Vec2(0, 0);
    std::vector<float>      m_raycastAngles;
    std::vector<RaycastTarget> m_raycastTargets;
    std::vector<RaycastTarget> m_staticRaycastTargets;
    std::vector<Vec2>       m_raycastResultPoints;
    std::vector<sf::ConvexShape> m_lightPolygons;
    bool                    m_useFancyLight = true;

    std::vector<sf::Sprite> m_background4;
    std::vector<sf::Sprite> m_background3;
    std::vector<sf::Sprite> m_background2;
    std::vector<sf::Sprite> m_background1;
    std::vector<std::shared_ptr<sf::Texture>> m_backText4;
    std::vector<std::shared_ptr<sf::Texture>> m_backText3;
    std::vector<std::shared_ptr<sf::Texture>> m_backText2;
    std::vector<std::shared_ptr<sf::Texture>> m_backText1;

    std::vector<sf::Texture> m_ammoIcons;
    sf::RectangleShape     m_ammoIconRect;
    sf::Text               m_ammoCount;

    sf::Shader              m_invisShader;
    sf::Shader              m_fastShader;
    sf::Shader              m_gradientShader;

    // Screen Shake
    int m_frameToStopScreenShake = 0;
    Vec2 m_currentViewOffset = Vec2(0, 0);
    int m_frameRate = 60;
    int m_screenShakeIntensity = 0;
    
    void init(const std::string & levelPath);

    void loadLevel(const std::string & filename);

    void update();
    void onEnd();
    void spawnPlayer();
    void playerDie();
    void finishLevel();
    void attack(std::shared_ptr<Entity> entity);
    void spawnBullet(std::shared_ptr<Entity> entity);
    void spawnBrickBullet(std::shared_ptr<Entity> entity);
    void spawnCameraBullet(std::shared_ptr<Entity> entity);
    void spawnHealBullet(std::shared_ptr<Entity> entity);
    void spawnMissles();
    void switchWeapon();
    void drawLine(const Vec2 & p1, const Vec2 & p2);
    sf::Color heathBarColorBlend(float t);
    std::string ammoCountStringFormatter(int ammoCount);

    Vec2 gridToMidPixel(float x, float y, std::shared_ptr<Entity> entity);
    Vec2 BasicCollision(std::shared_ptr<Entity> tile, std::shared_ptr<Entity> character);
    void SetGrounded(bool isGrounded);
    // Screen Shake
    void shakeScreen(float intensity, float duration);
    void sScreenShake();
    
    void pathFindToPoint(std::shared_ptr<Entity> character, Vec2 goal, Vec2 start, float speed);

    void updateFancyLighting(bool isFirstFrame);
    void getLevelEdges();
    std::vector<Vec2> getPlayerCorners();

    void sDoAction(const Action& action);
    void sDoMouseAction(const Action& action);
    void sPickUp(const int index);

    void sMovement();
    void sAI();
    void sLifespan();
    void sStatus();
    void sAnimation();
    void sCollision();
    void sCamera();
    void sPlayerCamera();
    void sBulletCamera();
    void sWinCam();
    void sRender();
    void updateBackground(std::vector<sf::Sprite>& background, float parallaxVal, int camStartIndex);
    void enlarge(bool beeg);

public:

    Scene_Play(GameEngine * gameEngine, const std::string & levelPath);

};