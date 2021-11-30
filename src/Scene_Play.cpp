#include "Scene_Play.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "Scene_Menu.h"
#include "Scene_Pause.h"
#include "AI.h"
#include <array>
#include "Pathfind.h"

Scene_Play::Scene_Play(GameEngine * gameEngine, const std::string & levelPath)
    : Scene(gameEngine)
    , m_levelPath(levelPath)
{
    init(m_levelPath);
}

void Scene_Play::init(const std::string & levelPath)
{
    registerAction(sf::Keyboard::P,     "PAUSE");
    registerAction(sf::Keyboard::Escape,"QUIT");
    registerAction(sf::Keyboard::T,     "TOGGLE_TEXTURE");      // Toggle drawing (T)extures
    registerAction(sf::Keyboard::C,     "TOGGLE_COLLISION");    // Toggle drawing (C)ollision Boxes
    registerAction(sf::Keyboard::G,     "TOGGLE_GRID");         // Toggle drawing (G)rid
    registerAction(sf::Keyboard::L, "TOGGLE_FANCY_LIGHTING");
    registerAction(options::m_rebindKeys[2], "UP");
    registerAction(options::m_rebindKeys[3], "DOWN");
    registerAction(options::m_rebindKeys[0], "LEFT");
    registerAction(options::m_rebindKeys[1], "RIGHT");
    registerAction(options::m_rebindKeys[4], "SPACE");
    registerAction(options::m_rebindKeys[5], "SWITCH");
    registerAction(sf::Keyboard::Q, "SMOOTH");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Arial"));

    m_backText4.push_back(std::make_shared<sf::Texture>());
    m_backText3.push_back(std::make_shared<sf::Texture>());
    m_backText2.push_back(std::make_shared<sf::Texture>());
    m_backText1.push_back(std::make_shared<sf::Texture>());

    m_game->playSound("Play");
    m_game->volSound("Play", options::m_optionSettings[2]);

    loadLevel(levelPath);
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
    Vec2 outputPos = Vec2(gridX, gridY);
    // Multiply grid pos by grid size
    outputPos *= m_gridSize.x;
    // Invert the sign of the y coord
    outputPos.y *= -1;
    // Correct the y pos
    outputPos.y += m_game->window().getSize().y;
    // Set the the position to the middle of the sprite
    if (entity != nullptr) 
    {
        outputPos.x += entity->getComponent<CAnimation>().animation.getSize().x / 2;
        outputPos.y -= entity->getComponent<CAnimation>().animation.getSize().y / 2;
    }
    return outputPos;
}

void Scene_Play::loadLevel(const std::string & filename)
{
    // Reset the entity manager when the level loads
    m_entityManager = EntityManager();

    // Set up some variables for later
    std::string name, lineLabel;
    float GX, GY;

    // Open the specified level file
    std::ifstream fin(filename);

    // While there are still lines to read
    while (fin >> lineLabel)
    {
        // Load in every tile
        if (lineLabel == "Tile" || lineLabel == "MovingTile")
        {
            fin >> name >> GX >> GY;
            auto brick = m_entityManager.addEntity("tile");
            brick->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
            brick->addComponent<CTransform>(gridToMidPixel(GX, GY, brick));
            brick->getComponent<CTransform>().homePos = Vec2(GX, GY);
            // With a bounding box
            brick->addComponent<CBoundingBox>(m_game->assets().getAnimation(name).getSize());
            if (lineLabel == "MovingTile")
            {
                std::vector<Vec2> path;
                int SPEED, N, PX, PY, isTrigger;
                fin >> isTrigger >> SPEED >> N;

                // Read in the waypoints correctly
                for (int i = 0; i < N; i++)
                {
                    fin >> PX >> PY;
                    path.push_back(Vec2(PX, PY));
                }
                brick->addComponent<CPatrol>(path, SPEED, isTrigger);
            }
            else
            {
                if (GX > m_levelBoundaries.x)
                {
                    m_levelBoundaries.x = GX;
                }
                m_staticTileGridInfo.push_back(Vec2(GX, GY));
            }
        }
        // Load in every decoration
        else if (lineLabel == "Dec")
        {
            fin >> name >> GX >> GY;
            auto brick = m_entityManager.addEntity("dec");
            brick->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
            brick->addComponent<CTransform>(gridToMidPixel(GX, GY, brick));
            // Without a bounding box
        }
        // Load in goals
        else if (lineLabel == "Goal")
        {
            fin >> GX >> GY;
            auto brick = m_entityManager.addEntity("goal");
            brick->addComponent<CAnimation>(m_game->assets().getAnimation("Goal"), true);
            m_goalPos = gridToMidPixel(GX + 0.5, GY + 1, brick);
            brick->addComponent<CTransform>(m_goalPos);
            brick->getComponent<CTransform>().scale = Vec2(64, 128);
            brick->addComponent<CBoundingBox>(Vec2(64, 128));
        }
        // Load in pickups
        else if (lineLabel == "PickUp")
        {
            int effect;
            fin >> name >> GX >> GY >> effect;
            auto brick = m_entityManager.addEntity("pickup");
            brick->addComponent<CAnimation>(m_game->assets().getAnimation(name), true);
            Vec2 pos = gridToMidPixel(GX, GY, brick);
            brick->addComponent<CTransform>(pos);
            brick->getComponent<CTransform>().scale *= 0.4f;
            brick->addComponent<CBoundingBox>(m_game->assets().getAnimation(name).getSize() * 0.4f);
            brick->getComponent<CBoundingBox>().blockVision = false;
            brick->addComponent<CPickUp>(pos, effect);
        }
        // Load in text
        else if (lineLabel == "Text")
        {
            std::string text;
            int size;
            fin >> GX >> GY >> size >> text;
            std::replace(text.begin(), text.end(), '_', ' ');
            sf::Text t = sf::Text(text, m_game->assets().getFont("Megaman"), size);
            Vec2 pos = gridToMidPixel(GX, GY, nullptr);
            t.setPosition(pos.x, pos.y);
            m_onScreenText.push_back(t);
        }
        // Load in the player
        else if (lineLabel == "Player")
        {
            fin >> m_playerConfig.X >> m_playerConfig.Y >> m_playerConfig.CX >> m_playerConfig.CY >> m_playerConfig.SPEED >> m_playerConfig.JUMP >> m_playerConfig.MAXSPEED >> m_playerConfig.GRAVITY;
            m_basePlayerConfig = m_playerConfig;
        }
        else if (lineLabel == "NPC")
        {
            int H, D, R, S, scale=3;
            std::string AI;
            fin >> name >> GX >> GY >> H >> D >> R >> AI >> S;
            if (name == "Boss") scale = 9;


            auto NPC = m_entityManager.addEntity(name);
            NPC->addComponent<CAnimation>(m_game->assets().getAnimation(name + "Walk"), true);
            NPC->addComponent<CTransform>(gridToMidPixel(GX, GY, NPC));
            NPC->addComponent<CBoundingBox>(Vec2(50, 90));
            NPC->getComponent<CBoundingBox>().blockVision = false;
            NPC->getComponent<CTransform>().scale *= scale;
            NPC->addComponent<CState>("Walk");
            NPC->addComponent<CHealth>(H, H);
            NPC->addComponent<CDamage>(D, R);
            if (AI == "Follow")
            {
                NPC->addComponent<CFollowPlayer>(gridToMidPixel(GX, GY, NPC), S);
            }

            if (AI == "Patrol")
            {
                float numPos, PX, PY;
                std::vector<Vec2> pos;
                fin >> numPos;

                for (int i = 0; i < numPos; i++)
                {
                    fin >> PX >> PY;
                    pos.push_back(gridToMidPixel(PX, PY, NPC));
                }

                NPC->addComponent<CPatrol>(pos, S, false);
            }

            if (name != "Guard") NPC->addComponent<CGravity>(m_playerConfig.GRAVITY);
        }
        // Errors just in case
        else
        {
            std::cout << "ERROR" << std::endl;
        }
    }
    // Close the file
    fin.close();

    m_timerText = sf::Text(std::to_string(0), m_game->assets().getFont("Megaman"), 16);
    m_timerText.setFillColor(sf::Color::White);
    m_timerText.setPosition(sf::Vector2f(10, 10));

    m_deathCountText = sf::Text("Death Count: " + std::to_string(m_deathCount), m_game->assets().getFont("Megaman"), 16);
    m_deathCountText.setFillColor(sf::Color::White);
    m_deathCountText.setPosition(sf::Vector2f(10, 10));


    m_healthBar = sf::RectangleShape(sf::Vector2f(400, 20));
    m_healthBar.setFillColor(heathBarColorBlend(1));
    m_healthBar.setOrigin(sf::Vector2f(400, 10));
    m_healthBar.setPosition(sf::Vector2f(640 - 230, m_game->window().getSize().y - 10));
    m_healthBarBackground = sf::RectangleShape(sf::Vector2f(410, 30));
    m_healthBarBackground.setFillColor(sf::Color::Black);
    m_healthBarBackground.setOrigin(sf::Vector2f(405, 15));
    m_healthBarBackground.setPosition(sf::Vector2f(640 - 230, m_game->window().getSize().y - 10));

    m_weaponConfig.currentWeapon = 0; m_weaponConfig.numWeapons = 4;

    m_ammoIcons = {
        m_game->assets().getTexture("TexLaserIcon"),
        m_game->assets().getTexture("TexBridge"),
        m_game->assets().getTexture("TexArrowIcon"),
        m_game->assets().getTexture("TexHeartIcon")
    };

    m_ammoIconRect = sf::RectangleShape(sf::Vector2f(40, 40));
    m_ammoIconRect.setPosition(sf::Vector2f(1280 - 100, 768 - 15 - 32));
    m_ammoIconRect.setTexture(&m_ammoIcons[1]);
    m_ammoIconRect.setTexture(&m_ammoIcons[m_weaponConfig.currentWeapon]);
    m_ammoCount = sf::Text("99", m_game->assets().getFont("Megaman"), 20);
    m_ammoCount.setFillColor(sf::Color::White);
    //m_ammoCount.setOrigin()
    m_ammoCount.setPosition(sf::Vector2f(1280 - 20, 768 - 15 - 32));

    std::array<StaticTileGridCell, 12> levelColumn;
    levelColumn.fill(StaticTileGridCell());
    for (int i = 0; i <= m_levelBoundaries.x; i++)
    {
        m_staticTileLocations.push_back(levelColumn);
    }
    for (int i = 0; i < m_staticTileGridInfo.size(); i++)
    {
        m_staticTileLocations[m_staticTileGridInfo[i].x][11 - m_staticTileGridInfo[i].y].exist = true;
    }
    /*for (int i = 0; i < m_staticTileLocations.size(); i++)
    {
        for (int j = 0; j < 12; j++)
        {
            std::cout << m_staticTileLocations[i][j].exist;
        }
        std::cout << std::endl;
    }*/

    getLevelEdges();
    /*for (int i = 0; i < m_levelEdges.size(); i++)
    {
        m_cornerDisplayCircles.push_back(sf::CircleShape(4));
        m_cornerDisplayCircles.back().setOrigin(4,4);
        m_cornerDisplayCircles.back().setPosition(m_levelEdges[i].startPoint.x, m_levelEdges[i].startPoint.y);
        m_cornerDisplayCircles.back().setFillColor(sf::Color::Red);
        m_cornerDisplayCircles.push_back(sf::CircleShape(4));
        m_cornerDisplayCircles.back().setOrigin(4, 4);
        m_cornerDisplayCircles.back().setPosition(m_levelEdges[i].endPoint.x, m_levelEdges[i].endPoint.y);
        m_cornerDisplayCircles.back().setFillColor(sf::Color::Red);
    }
    for (int i = 0; i < m_levelEdges.size(); i++)
    {
        m_edgeDisplayRectangles.push_back(sf::RectangleShape());
        m_edgeDisplayRectangles.back().setPosition(m_levelEdges[i].startPoint.x, m_levelEdges[i].startPoint.y);
        m_edgeDisplayRectangles.back().setSize(sf::Vector2f(m_levelEdges[i].endPoint.x - m_levelEdges[i].startPoint.x, m_levelEdges[i].endPoint.y - m_levelEdges[i].startPoint.y));
        m_edgeDisplayRectangles.back().setFillColor(sf::Color::Transparent);
        m_edgeDisplayRectangles.back().setOutlineThickness(2);
        m_edgeDisplayRectangles.back().setOutlineColor(sf::Color::White);
    }*/

    std::set<float> raycastAngleSet;
    for (int i = 0; i < m_levelEdges.size(); i++)
    {
        float angle = atan2(m_levelEdges[i].startPoint.y - m_goalPos.y, m_levelEdges[i].startPoint.x - m_goalPos.x);
        raycastAngleSet.insert(angle);
        angle = atan2(m_levelEdges[i].endPoint.y - m_goalPos.y, m_levelEdges[i].endPoint.x - m_goalPos.x);
        raycastAngleSet.insert(angle);
    }
    // https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
    m_raycastAngles.assign(raycastAngleSet.begin(), raycastAngleSet.end());
    std::vector<float> tempNewAnglesVec;
    for (int i = 0; i < m_raycastAngles.size(); i++)
    {
        tempNewAnglesVec.push_back(m_raycastAngles[i] + 0.00001f);
        tempNewAnglesVec.push_back(m_raycastAngles[i] - 0.00001f);
    }
    m_raycastAngles.clear();
    m_raycastAngles = tempNewAnglesVec;
    for (int i = 0; i < m_raycastAngles.size(); i++)
    {
        Vec2 newPointPos = Vec2(cosf(m_raycastAngles[i]), sinf(m_raycastAngles[i]));
        newPointPos *= 800;
        newPointPos += m_goalPos;
        m_raycastTargets.push_back(RaycastTarget());
        m_raycastTargets.back().pos = newPointPos;
        m_raycastTargets.back().angle = m_raycastAngles[i];
    }
    //https://stackoverflow.com/questions/29286439/how-to-use-sort-in-c-with-custom-sort-member-function/29286506
    std::sort(m_raycastTargets.begin(), m_raycastTargets.end(), [this](RaycastTarget a, RaycastTarget b) {return a.angle < b.angle; });
    m_staticRaycastTargets = m_raycastTargets;
    /*for (int i = 0; i < m_raycastTargetPoints.size(); i++)
    {
        m_cornerDisplayCircles.push_back(sf::CircleShape(4));
        m_cornerDisplayCircles.back().setOrigin(4, 4);
        m_cornerDisplayCircles.back().setPosition(m_raycastTargetPoints[i].x, m_raycastTargetPoints[i].y);
        m_cornerDisplayCircles.back().setFillColor(sf::Color::Red);
    }*/

    updateFancyLighting(true);


    for (int i = 0; i < m_raycastResultPoints.size(); i++)
    {
        m_cornerDisplayCircles.push_back(sf::CircleShape(4));
        m_cornerDisplayCircles.back().setOrigin(4, 4);
        m_cornerDisplayCircles.back().setPosition(m_raycastResultPoints[i].x, m_raycastResultPoints[i].y);
        m_cornerDisplayCircles.back().setFillColor(sf::Color::Red);
    }

    for (int i = 0; i < 4; i++)
    {
        m_camStartX.push_back(0);
    }

    m_invisShader.loadFromFile("shaders/invis.frag", sf::Shader::Fragment);
    m_fastShader.loadFromFile("shaders/red.frag", sf::Shader::Fragment);
    // https://github.com/SFML/SFML/wiki/Source:-Radial-Gradient-Shader
    m_gradientShader.loadFromFile("shaders/gradient.frag", sf::Shader::Fragment);
    m_gradientShader.setParameter("color", sf::Color::White);
    m_gradientShader.setParameter("center", 100, 600);
    m_gradientShader.setParameter("radius", 800);
    m_gradientShader.setParameter("expand", 0.f);
    m_gradientShader.setParameter("windowHeight", static_cast<float>(m_game->window().getSize().y)); // this must be set, but only needs to be set once (or whenever the size of the window changes)

    spawnPlayer();

    m_weaponConfig.currentWeapon = 0; m_weaponConfig.numWeapons = 4;


    auto nesWrestler = m_entityManager.addEntity("wrestler");
    nesWrestler->addComponent<CAnimation>(m_game->assets().getAnimation("Win"), true);
    nesWrestler->addComponent<CTransform>(Vec2(-5000, -5000));
    nesWrestler->getComponent<CTransform>().scale *= 3;
}

std::vector<Vec2> Scene_Play::getPlayerCorners()
{
    Vec2 playerPos = m_player->getComponent<CTransform>().pos;
    Vec2 hBB = m_player->getComponent<CBoundingBox>().halfSize;
    std::vector<Vec2> corners;
    corners.push_back(Vec2(-hBB.x, hBB.y) + playerPos);
    corners.push_back(Vec2(hBB.x, hBB.y) + playerPos);
    corners.push_back(Vec2(hBB.x, -hBB.y) + playerPos);
    corners.push_back(Vec2(-hBB.x, -hBB.y) + playerPos);
    return corners;
}

void Scene_Play::updateFancyLighting(bool isFirstFrame)
{
    m_raycastResultPoints.clear();
    m_lightPolygons.clear();


    if (!isFirstFrame)
    {
        std::vector<Vec2> playerCorners = getPlayerCorners();
        for (int i = 0; i < 4; i++)
        {
            int nextIndex = (i + 1) % 4;
            m_levelEdges.push_back(Edge(playerCorners[i], playerCorners[nextIndex]));
        }
        for (int i = 0; i < 4; i++)
        {
            float angle = atan2(playerCorners[i].y - m_goalPos.y, playerCorners[i].x - m_goalPos.x);
            std::array<float, 2> offsetAngles{ {angle + 0.00001f, angle - 0.00001f} };
            for (int j = 0; j < 2; j++)
            {
                Vec2 newPointPos = Vec2(cosf(offsetAngles[j]), sinf(offsetAngles[j]));
                newPointPos *= 800;
                newPointPos += m_goalPos;
                m_raycastTargets.push_back(RaycastTarget());
                m_raycastTargets.back().pos = newPointPos;
                m_raycastTargets.back().angle = offsetAngles[j];
            }
        }
        // https://stackoverflow.com/questions/29286439/how-to-use-sort-in-c-with-custom-sort-member-function/29286506
        std::sort(m_raycastTargets.begin(), m_raycastTargets.end(), [this](RaycastTarget a, RaycastTarget b) {return a.angle < b.angle; });
    }

    /*for (int i = 0; i < 100; i++)
    {
        std::cout << "lag" << std::endl;
        double e = sin(sin(sin(sin(sin(sin(sin(sin(sin(sin(0.5))))))))));
    }*/

    for (int i = 0; i < m_raycastTargets.size(); i++)
    {
        Vec2 point = m_raycastTargets[i].pos;
        float minSquaredDist = 9999999999;
        for (int j = 0; j < m_levelEdges.size(); j++)
        {
            Intersect raycastHit = Physics::LineIntersect(m_goalPos, point, m_levelEdges[j].startPoint, m_levelEdges[j].endPoint);
            if (raycastHit.result)
            {
                Vec2 delta = raycastHit.pos - m_goalPos;
                float squaredDist = delta.x * delta.x + delta.y * delta.y;
                if (squaredDist < minSquaredDist)
                {
                    minSquaredDist = squaredDist;
                    point = raycastHit.pos;
                }
            }
        }
        m_raycastResultPoints.push_back(point);
    }


    int pointNum = m_raycastResultPoints.size();
    for (int i = 0; i < pointNum; i++)
    {
        int nextIndex = (i + 1) % pointNum;
        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, sf::Vector2f(m_goalPos.x, m_goalPos.y));
        triangle.setPoint(1, sf::Vector2f(m_raycastResultPoints[i].x, m_raycastResultPoints[i].y));
        triangle.setPoint(2, sf::Vector2f(m_raycastResultPoints[nextIndex].x, m_raycastResultPoints[nextIndex].y));
        triangle.setFillColor(sf::Color::Transparent);
        m_lightPolygons.push_back(triangle);
    }

    if (!isFirstFrame)
    {
        for (int i = 0; i < 4; i++)
        {
            m_levelEdges.pop_back();
        }
        m_raycastTargets = m_staticRaycastTargets;
    }
}

// https://www.youtube.com/watch?v=fc3nnG2CG8U&t=1430s&ab_channel=javidx9
void Scene_Play::getLevelEdges()
{
    m_levelEdges.clear();

    for (int y = 0; y < 12; y++)
    {
        for (int x = 0; x < m_levelBoundaries.x; x++)
        {
            // If tile exists
            if (m_staticTileLocations[x][y].exist)
            {
                // WEST
                if (x != 0 && !m_staticTileLocations[x - 1][y].exist)
                {
                    if (y != 0 && m_staticTileLocations[x][y - 1].edgeExist[3])
                    {
                        m_levelEdges[m_staticTileLocations[x][y - 1].edgeID[3]].endPoint.y += 64;
                        m_staticTileLocations[x][y].edgeID[3] = m_staticTileLocations[x][y - 1].edgeID[3];
                        m_staticTileLocations[x][y].edgeExist[3] = true;
                    }
                    else
                    {
                        Edge edge = Edge(Vec2(x * 64, y * 64), Vec2(x * 64, (y + 1) * 64));
                        int edgeID = m_levelEdges.size();
                        m_levelEdges.push_back(edge);
                        m_staticTileLocations[x][y].edgeExist[3] = true;
                        m_staticTileLocations[x][y].edgeID[3] = edgeID;
                    }
                }
                // EAST
                if (x != (m_levelBoundaries.x - 1) && !m_staticTileLocations[x + 1][y].exist)
                {
                    if (y != 0 && m_staticTileLocations[x][y - 1].edgeExist[1])
                    {
                        m_levelEdges[m_staticTileLocations[x][y - 1].edgeID[1]].endPoint.y += 64;
                        m_staticTileLocations[x][y].edgeID[1] = m_staticTileLocations[x][y - 1].edgeID[1];
                        m_staticTileLocations[x][y].edgeExist[1] = true;
                    }
                    else
                    {
                        Edge edge = Edge(Vec2((x + 1) * 64, (y) * 64), Vec2((x + 1) * 64, (y + 1) * 64));
                        int edgeID = m_levelEdges.size();
                        m_levelEdges.push_back(edge);
                        m_staticTileLocations[x][y].edgeExist[1] = true;
                        m_staticTileLocations[x][y].edgeID[1] = edgeID;
                    }
                }
                // NORTH
                if (y != 0 && !m_staticTileLocations[x][y - 1].exist)
                {
                    if (x != 0 && m_staticTileLocations[x - 1][y].edgeExist[0])
                    {
                        m_levelEdges[m_staticTileLocations[x - 1][y].edgeID[0]].endPoint.x += 64;
                        m_staticTileLocations[x][y].edgeID[0] = m_staticTileLocations[x - 1][y].edgeID[0];
                        m_staticTileLocations[x][y].edgeExist[0] = true;
                    }
                    else
                    {
                        Edge edge = Edge(Vec2(x * 64, y * 64), Vec2((x + 1) * 64, y * 64));
                        int edgeID = m_levelEdges.size();
                        m_levelEdges.push_back(edge);
                        m_staticTileLocations[x][y].edgeExist[0] = true;
                        m_staticTileLocations[x][y].edgeID[0] = edgeID;
                    }
                }
                // SOUTH
                if (y != 11 && !m_staticTileLocations[x][y + 1].exist)
                {
                    if (x != 0 && m_staticTileLocations[x - 1][y].edgeExist[2])
                    {
                        m_levelEdges[m_staticTileLocations[x - 1][y].edgeID[2]].endPoint.x += 64;
                        m_staticTileLocations[x][y].edgeID[2] = m_staticTileLocations[x - 1][y].edgeID[2];
                        m_staticTileLocations[x][y].edgeExist[2] = true;
                    }
                    else
                    {
                        Edge edge = Edge(Vec2((x) * 64, (y + 1) * 64), Vec2((x + 1) * 64, (y + 1) * 64));
                        int edgeID = m_levelEdges.size();
                        m_levelEdges.push_back(edge);
                        m_staticTileLocations[x][y].edgeExist[2] = true;
                        m_staticTileLocations[x][y].edgeID[2] = edgeID;
                    }
                }
            }
        }
    }
}

void Scene_Play::spawnPlayer()
{
    // Create player entity
    m_player = m_entityManager.addEntity("player");
    // Add components
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
    m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player));
    m_playerSize = 3;
    m_player->getComponent<CTransform>().scale *= m_playerSize;
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY) * m_playerSize);
    m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
    m_player->addComponent<CState>();
    m_player->addComponent<CInput>();
    m_player->addComponent<CAdvancedMovement>();
    m_player->addComponent<CHealth>(5.0f, 5.0f);
}

void Scene_Play::enlarge(bool beeg) //beeg
{
    float enlarged = 8.;
    float newScale = (beeg) ? (enlarged / 3.) : (3. / enlarged);
    float speedScale = (beeg) ? 0.6 : 1 / 0.6;
    m_player->getComponent<CTransform>().scale *= newScale;
    m_playerSize *= newScale;
    m_player->getComponent<CBoundingBox>().size *= newScale;
    m_player->getComponent<CBoundingBox>().halfSize *= newScale;
    m_playerConfig.SPEED /= (newScale * speedScale);
    m_playerConfig.JUMP /= (newScale * speedScale);
}

void Scene_Play::playerDie()
{
    m_player->addComponent<CInvincibility>(60);
    m_game->playSound("PlayerDie");
    m_game->volSound("PlayerDie", options::m_optionSettings[1]);
    if (m_player->getComponent<CAnimation>().animation.getName() == "Die") return;
    m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Die");
    m_player->getComponent<CAnimation>().repeat = false;

    m_deathCount++;
    m_player->getComponent<CHealth>().current = m_player->getComponent<CHealth>().max;
    m_healthBar.setScale(sf::Vector2f(1, 1));
    m_healthBar.setFillColor(heathBarColorBlend(1));
    m_deathCountText.setString("Death Count: " + std::to_string(m_deathCount));
}

void Scene_Play::finishLevel() 
{
    if (m_levelPath != "bossBattle.txt") m_game->changeScene("OVERWORLD", nullptr, true);
    m_game->stopSound("Play");
    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", options::m_optionSettings[2]);
    //logic to set beaten levels for overworld progression
    if (m_levelPath == "level1.txt")
    {
        options::m_beatenLevels[0] = true;
    }
    else if (m_levelPath == "level2.txt")
    {
        options::m_beatenLevels[1] = true;
    }
    else if (m_levelPath == "level3.txt")
    {
        options::m_beatenLevels[2] = true;
    }
    else if (m_levelPath == "bossBattle.txt")
    {
        options::m_beatenLevels[3] = true;
        m_won = true;
        m_game->stopSound("MainMenu");
        m_game->playSound("Win");
        m_game->volSound("Win", options::m_optionSettings[2]);
    }
}

void Scene_Play::spawnBullet(std::shared_ptr<Entity> entity)
{
    // Create bullet entity
    auto thisEntity = m_entityManager.addEntity("bullet");
    // Add components
    thisEntity->addComponent<CAnimation>(m_game->assets().getAnimation("Buster"), true);
    thisEntity->getComponent<CAnimation>().animation.getSprite().setOrigin(sf::Vector2f(25, 5));
    thisEntity->addComponent<CTransform>(m_player->getComponent<CTransform>().pos);
    thisEntity->getComponent<CTransform>().scale *= (2 / 3);
    thisEntity->getComponent<CTransform>().scale = m_player->getComponent<CTransform>().scale;
    thisEntity->addComponent<CBoundingBox>(m_game->assets().getAnimation("Buster").getSize());
    thisEntity->getComponent<CBoundingBox>().blockVision = false;
    thisEntity->addComponent<CLifeSpan>(60, m_currentFrame);
    if (m_playerIsBig) thisEntity->addComponent<CDamage>(3.f / options::m_optionSettings[0]);
    else thisEntity->addComponent<CDamage>(1.f / options::m_optionSettings[0]);
    // Set speed
    thisEntity->getComponent<CTransform>().velocity.x = 12 * m_player->getComponent<CTransform>().scale.x/3;
}

void Scene_Play::spawnBrickBullet(std::shared_ptr<Entity> entity)
{
    entity->getComponent<CInput>().canShoot = false;

    //variables based of player entity
    Vec2 pPos = entity->getComponent<CTransform>().pos;
    auto pAngle = entity->getComponent<CTransform>().angle;
    auto pVelocity = m_playerConfig.SPEED * 2;

    //create bullet with brick animation
    auto bullet = m_entityManager.addEntity("bullet");
    bullet->addComponent<CAnimation>(m_game->assets().getAnimation("Bridge"), true);

    //shoot bullet depending on which way the player is facing
    if (entity->getComponent<CTransform>().scale.x > 0)
    {
        bullet->addComponent<CTransform>(pPos, Vec2(pVelocity, 0), Vec2(0.25, 0.25), pAngle);
    }
    else
    {
        bullet->addComponent<CTransform>(pPos, Vec2(-pVelocity, 0), Vec2(0.25, 0.25), pAngle);
    }

    //add lifespan and boundingbox
    bullet->addComponent<CBoundingBox>(m_game->assets().getAnimation("Buster").getSize());
    bullet->getComponent<CBoundingBox>().blockVision = false;
    bullet->addComponent<CLifeSpan>(60, currentFrame());
    bullet->getComponent<CTransform>().scale = m_player->getComponent<CTransform>().scale/3;
}

void Scene_Play::spawnCameraBullet(std::shared_ptr<Entity> entity)
{
    if (m_cameraTarget != m_player) return;
    // Create bullet entity
    auto thisEntity = m_entityManager.addEntity("bullet");
    // Add components
    thisEntity->addComponent<CAnimation>(m_game->assets().getAnimation("Arrow"), true);
    thisEntity->addComponent<CTransform>(m_player->getComponent<CTransform>().pos);
    thisEntity->addComponent<CBoundingBox>(m_game->assets().getAnimation("Arrow").getSize());
    thisEntity->getComponent<CBoundingBox>().blockVision = false;
    thisEntity->addComponent<CLifeSpan>(240, m_currentFrame);
    thisEntity->addComponent<CInput>();
    if (m_playerIsBig) thisEntity->addComponent<CDamage>(9.f / options::m_optionSettings[0]);
    else thisEntity->addComponent<CDamage>(3.f / options::m_optionSettings[0]);
    thisEntity->getComponent<CTransform>().scale = (m_playerIsBig) ? m_player->getComponent<CTransform>().scale.abs() : m_player->getComponent<CTransform>().scale.abs()/3;
    // Set speed
    thisEntity->getComponent<CTransform>().velocity.x = 9 * m_player->getComponent<CTransform>().scale.x / 3;
    if (entity->getComponent<CTransform>().scale.x < 0) thisEntity->getComponent<CTransform>().angle = 180;

    m_control = thisEntity;
    m_cameraTarget = thisEntity;
}

void Scene_Play::spawnHealBullet(std::shared_ptr<Entity> entity)
{
    // Create bullet entity
    auto thisEntity = m_entityManager.addEntity("bullet");
    // Add components
    thisEntity->addComponent<CAnimation>(m_game->assets().getAnimation("HealArrow"), true);
    thisEntity->addComponent<CTransform>(m_player->getComponent<CTransform>().pos);
    thisEntity->addComponent<CBoundingBox>(m_game->assets().getAnimation("HealArrow").getSize());
    thisEntity->getComponent<CBoundingBox>().blockVision = false;
    thisEntity->addComponent<CLifeSpan>(30, m_currentFrame);
    thisEntity->getComponent<CTransform>().scale = m_player->getComponent<CTransform>().scale/3;
    // Set speed
    thisEntity->getComponent<CTransform>().velocity.x = 12 * m_player->getComponent<CTransform>().scale.x / 3;
}

void Scene_Play::spawnMissles()
{
    if (m_levelPath != "bossBattle.txt") return;


    //std::cout << (m_currentFrame + 1) % 180;
    if (((m_currentFrame + 1) % 300) == 0)
    {
        auto missle = m_entityManager.addEntity("Missle");
        missle->addComponent<CAnimation>(m_game->assets().getAnimation("Missle"), true);
        missle->addComponent<CTransform>(gridToMidPixel(2, 9, missle));
        missle->getComponent<CTransform>().velocity = Vec2(5, 5);
        missle->getComponent<CTransform>().scale *= 1.5;
        missle->addComponent<CBoundingBox>(m_game->assets().getAnimation("Missle").getSize()*4);
        missle->getComponent<CBoundingBox>().blockVision = false;
        missle->addComponent<CLifeSpan>(180, m_currentFrame);

        auto missle2 = m_entityManager.addEntity("Missle");
        missle2->addComponent<CAnimation>(m_game->assets().getAnimation("Missle"), true);
        missle2->addComponent<CTransform>(gridToMidPixel(15, 9, missle2));
        missle2->getComponent<CTransform>().velocity = Vec2(5, 5);
        missle2->getComponent<CTransform>().scale *= 1.5;
        missle2->addComponent<CBoundingBox>(m_game->assets().getAnimation("Missle").getSize() * 4);
        missle2->getComponent<CBoundingBox>().blockVision = false;
        missle2->addComponent<CLifeSpan>(180, m_currentFrame);
    }
}

void Scene_Play::attack(std::shared_ptr<Entity> entity)
{
    if (m_weaponConfig.ammo[m_weaponConfig.currentWeapon] != 0)
    {
        if (m_weaponConfig.currentWeapon == 0) spawnBullet(m_player);
        if (m_weaponConfig.currentWeapon == 1) spawnBrickBullet(m_player);
        if (m_weaponConfig.currentWeapon == 2) spawnCameraBullet(m_player);
        if (m_weaponConfig.currentWeapon == 3) spawnHealBullet(m_player);
        m_weaponConfig.ammo[m_weaponConfig.currentWeapon] -= 1;
        if (m_playerIsBig)
        {
            m_game->playSound("potentiallyBEEGERLazer");
            m_game->volSound("potentiallyBEEGERLazer", options::m_optionSettings[1]*2);
        }
        else
        {
            m_game->playSound("Shoot");
            m_game->volSound("Shoot", options::m_optionSettings[1]);
        }
        
        m_player->getComponent<CTransform>().velocity.x -= m_player->getComponent<CTransform>().scale.x * 4;
        if (m_player->getComponent<CTransform>().velocity.x > m_playerConfig.MAXSPEED || m_player->getComponent<CTransform>().velocity.x < -m_playerConfig.MAXSPEED)
        {
            m_player->getComponent<CTransform>().velocity.x = -m_playerConfig.MAXSPEED * m_player->getComponent<CTransform>().scale.x / m_playerSize;
        }
        if (!m_playerIsGrounded) { m_player->getComponent<CAdvancedMovement>().shotInAir = true; }
        //m_player->getComponent<CAdvancedMovement>().usesPhysics = true;
        shakeScreen(1.5 * (m_playerSize / 3.) * (m_playerSize / 3.), 0.05);
    }
    else
    {
        m_game->playSound("Bow");
        m_game->volSound("Bow", options::m_optionSettings[1]);
    }
    m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Attack");
    m_ammoCount.setString(ammoCountStringFormatter(m_weaponConfig.ammo[m_weaponConfig.currentWeapon]));
}
void Scene_Play::switchWeapon()
{
    m_weaponConfig.currentWeapon = (m_weaponConfig.currentWeapon + 1) % m_weaponConfig.numWeapons;
    m_ammoIconRect.setTexture(&m_ammoIcons[m_weaponConfig.currentWeapon]);
    m_ammoCount.setString(ammoCountStringFormatter(m_weaponConfig.ammo[m_weaponConfig.currentWeapon]));
}

void Scene_Play::update()
{
    m_entityManager.update();

    // TODO: implement pause functionality
    /*m_active = (m_active + 1) % m_timeScale;
    (!m_isSlow || m_active == 0) ? m_currentFrame++ : 0;*/
    m_currentFrame++;
    spawnMissles();
    sAI();
    sMovement();
    sLifespan();
    sStatus();
    sCollision();
    sAnimation();
    sCamera();
}

// Might be implemented to stop m_playerIsGrounded from being set every frame
// But the if statement itself might be more expensive
void Scene_Play::SetGrounded(bool isGrounded) 
{
    if (m_playerIsGrounded != isGrounded)
    {
        m_playerIsGrounded = isGrounded;
        if (isGrounded) 
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Land");
            m_player->getComponent<CAdvancedMovement>().shotInAir = false;
        }
    }
}

// Get the axis of the player input
Vec2 playerInputDirectionsCalc(CInput playerInput)
{
    int x = 0, y = 0;
    if (playerInput.up) { y -= 1; }
    if (playerInput.down) { y += 1; }
    if (playerInput.right) { x += 1; }
    if (playerInput.left) { x -= 1; }
    return Vec2(x, y);
}

void Scene_Play::sMovement()
{
    if (!m_control || !m_control->isActive()) m_control = m_player;
    Vec2 playerInputDirections = playerInputDirectionsCalc(m_player->getComponent<CInput>());
    // X movement and sprite facing correct direction
    if (m_control == m_player) // controling player
    {
        if ((playerInputDirections.x != 0 || !m_player->getComponent<CAdvancedMovement>().usesPhysics) && (m_player->getComponent<CState>().state != "Slide"))
        {
            if (m_player->getComponent<CTransform>().scale.x / m_playerSize != playerInputDirections.x && playerInputDirections.x != 0)
            {
                m_player->getComponent<CTransform>().scale.x = playerInputDirections.x * m_playerSize;
            }
            float target = playerInputDirections.x * m_playerConfig.SPEED * 1.2;
            Vec2 accel = (m_player->getComponent<CAdvancedMovement>().usesPhysics) ? Vec2(0.03, 0.03) : Vec2(0.2, 0.5);
            float scale = (!playerInputDirections.x) ? accel.y : accel.x;
            m_player->getComponent<CTransform>().velocity.x += (target - m_player->getComponent<CTransform>().velocity.x) * scale;
        }

        // Jump Forgiveness
        if (m_playerIsGrounded)
        {
            m_player->getComponent<CAdvancedMovement>().jumpForgivenessCounter = m_player->getComponent<CAdvancedMovement>().jumpForgivenessTime * 60;
        }
        else
        {
            m_player->getComponent<CAdvancedMovement>().jumpForgivenessCounter--;
        }
        if (!m_playerHasWallJumped)
        {
            m_player->getComponent<CAdvancedMovement>().wallJumpForgivenessCounter = m_player->getComponent<CAdvancedMovement>().wallJumpForgivenessTime * 60;
        }
        else
        {
            m_player->getComponent<CAdvancedMovement>().wallJumpForgivenessCounter--;
        }
        // Jump Buffer
        if (m_player->getComponent<CInput>().tryJump)
        {
            m_player->getComponent<CAdvancedMovement>().jumpBufferCounter = m_player->getComponent<CAdvancedMovement>().jumpBufferTime * 60;
        }
        else
        {
            m_player->getComponent<CAdvancedMovement>().jumpBufferCounter--;
        }


        // Y movement
        if (m_player->getComponent<CTransform>().velocity.y > m_playerConfig.MAXSPEED) { m_player->getComponent<CTransform>().velocity.y = m_playerConfig.MAXSPEED; }
        // Jumping
        if (m_player->getComponent<CAdvancedMovement>().jumpBufferCounter > 0)
        {
            if (m_player->getComponent<CAdvancedMovement>().jumpForgivenessCounter > 0 &&
                !m_player->getComponent<CAdvancedMovement>().hasJumped)
            {
                m_player->getComponent<CAdvancedMovement>().jumpForgivenessCounter = 0;
                m_player->getComponent<CAdvancedMovement>().jumpBufferCounter = 0;
                m_player->getComponent<CAdvancedMovement>().hasJumped = true;
                m_player->getComponent<CTransform>().velocity.y = m_playerConfig.JUMP;
                if (m_player->getComponent<CState>().state == "Slide")
                {
                    m_player->getComponent<CTransform>().velocity.x = m_playerConfig.JUMP * m_player->getComponent<CTransform>().scale.x / m_playerSize;
                    m_player->getComponent<CAdvancedMovement>().usesPhysics = true;
                    m_player->getComponent<CAdvancedMovement>().frameToAllowInfluence = m_currentFrame + 60;
                }
                m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Jump");
                m_game->playSound("Jump");
                m_game->volSound("Jump", options::m_optionSettings[1]);
            }
            else if (m_player->getComponent<CAdvancedMovement>().wallJumpForgivenessCounter > 0)
            {
                m_player->getComponent<CAdvancedMovement>().jumpBufferCounter = 0;
                m_player->getComponent<CAdvancedMovement>().usesPhysics = true;
                m_player->getComponent<CAdvancedMovement>().frameToAllowInfluence = m_currentFrame + 60;
                m_player->getComponent<CTransform>().scale.x = m_player->getComponent<CAdvancedMovement>().lastWallSide * m_playerSize;
                m_player->getComponent<CTransform>().velocity = Vec2(5 * m_player->getComponent<CTransform>().scale.x / m_playerSize, m_playerConfig.JUMP);
                if (m_movingPlatPlayerRiding != nullptr)
                {
                    m_player->getComponent<CTransform>().velocity += m_movingPlatPlayerRiding->getComponent<CTransform>().velocity;
                }
                m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Jump");
            }
        }

        // Stop jump if let go of button
        else if (!m_player->getComponent<CInput>().up)
        {
            if (m_player->getComponent<CTransform>().velocity.y < 0)
            {
                m_player->getComponent<CTransform>().velocity.y /= 3;
            }
            m_player->getComponent<CAdvancedMovement>().hasJumped = false;
        }
        m_player->getComponent<CInput>().tryJump = false;
        m_playerHasWallJumped = true;

        // Sliding
        if (m_player->getComponent<CState>().state == "Slide" && m_player->getComponent<CInput>().down)
        {
            m_player->getComponent<CTransform>().velocity.x = 10 * (m_player->getComponent<CTransform>().scale.x / m_playerSize) * m_slideAmount;
            if (m_movingPlatPlayerRiding != nullptr)
            {
                m_player->getComponent<CTransform>().velocity += m_movingPlatPlayerRiding->getComponent<CTransform>().velocity;
            }
            if (m_playerIsGrounded)
            {
                m_slideAmount *= 0.95;
            }
        }

    }
    else //controlling bullet
    {
        m_player->getComponent<CTransform>().velocity = Vec2(0, 6);
        m_player->getComponent<CInput>().left = false; m_player->getComponent<CInput>().right = false;
        m_player->getComponent<CInput>().down = false; m_player->getComponent<CInput>().up = false;
        m_player->getComponent<CState>().state = "Stand";
        auto input = m_control->getComponent<CInput>();
        auto& transform = m_control->getComponent<CTransform>();
        if (input.up && !input.down)
        {
            if (m_player->getComponent<CTransform>().scale.x < 0) transform.angle += 3; //invert control if player facing other dir
            else transform.angle -= 3;
        }
        else if (input.down && !input.up)
        {
            if (m_player->getComponent<CTransform>().scale.x < 0) transform.angle -= 3;
            else transform.angle += 3;
        }
        auto x = 10 * std::cos(transform.angle * 3.14159 / 180);
        auto y = 10 * std::sin(transform.angle * 3.14159 / 180);
        transform.velocity = Vec2(x, y);
    }
    
    // Gravity
    for (auto& e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CGravity>())
        {
            e->getComponent<CTransform>().velocity.y += m_player->getComponent<CGravity>().gravity;
        }
    }
    
    for (auto& e : m_entityManager.getEntities("tile"))
    {
        if (e->hasComponent<CPatrol>() && e->getComponent<CPatrol>().moving)
        {
            int speed = e->getComponent<CPatrol>().speed;
            // Calculate the position of the goal
            Vec2 pos = e->getComponent<CTransform>().pos;
            Vec2 adjustment = Vec2(0.5f, 0.5f);
            Vec2 goalPosTile = e->getComponent<CPatrol>().positions[e->getComponent<CPatrol>().goalPosition] + adjustment;
            Vec2 goalPos = gridToMidPixel(goalPosTile.x, goalPosTile.y, nullptr);
            Vec2 prevPosTile = e->getComponent<CPatrol>().positions[e->getComponent<CPatrol>().prevPosition] + adjustment;
            Vec2 prevPos = gridToMidPixel(prevPosTile.x, prevPosTile.y, nullptr);
            // Try and reach the goal
            pathFindToPoint(e, goalPos, prevPos, speed);
            // Switch to the next goal if it's close enough to the current goal
            if (e->getComponent<CPatrol>().progress >= 1)
            {
                e->getComponent<CPatrol>().prevPosition = e->getComponent<CPatrol>().goalPosition;
                e->getComponent<CPatrol>().goalPosition = (e->getComponent<CPatrol>().goalPosition + 1) % e->getComponent<CPatrol>().positions.size();
                e->getComponent<CPatrol>().progress = 0.0;
                //std::cout << "Turn Around: " << e->getComponent<CPatrol>().goalPosition << ", " << e->getComponent<CPatrol>().prevPosition << std::endl;
                if (e->getComponent<CPatrol>().trigger && e->getComponent<CPatrol>().prevPosition == 0 && !e->getComponent<CPatrol>().playerOn)
                {
                    e->getComponent<CPatrol>().moving = false;
                    e->getComponent<CTransform>().velocity = Vec2(0, 0);
                }
            }
        }
    }

    // Pickups
    for (auto& e : m_entityManager.getEntities("pickup"))
    {
        float diff = e->getComponent<CTransform>().pos.y - e->getComponent<CPickUp>().animTarget[!e->getComponent<CPickUp>().goingUp];
        if (diff < 0.1 && diff > -0.1)
        {
            e->getComponent<CPickUp>().goingUp = !e->getComponent<CPickUp>().goingUp;
            e->getComponent<CPickUp>().animProgress = 0;
        }
        e->getComponent<CPickUp>().animProgress += 0.008;
        float t = e->getComponent<CPickUp>().animProgress;
        float a = (t * t * t * t);
        float b = (1 - ((1 - t) * (1 - t) * (1 - t) * (1 - t)));
        t = a + t * (b - a);
        e->getComponent<CTransform>().pos.y = e->getComponent<CPickUp>().animTarget[e->getComponent<CPickUp>().goingUp] +
            (e->getComponent<CPickUp>().animTarget[!e->getComponent<CPickUp>().goingUp] - e->getComponent<CPickUp>().animTarget[e->getComponent<CPickUp>().goingUp]) * t;
    }

    // Change pos for all entities
    for (auto& e : m_entityManager.getEntities())
    {
        if (e == m_player && m_player->getComponent<CAnimation>().animation.getName() == "Die") continue;
        e->getComponent<CTransform>().velocity += e->getComponent<CTransform>().acceleration;
        e->getComponent<CTransform>().prevPos = e->getComponent<CTransform>().pos;
        e->getComponent<CTransform>().pos += e->getComponent<CTransform>().velocity;
    }
}

void Scene_Play::sAI()
{
    for (auto e : m_entityManager.getEntities("Enemy"))
    {
        if (e->tag() != "BossAtk")
        {
            AI ai = AI(e, m_player, m_game, m_entityManager);
            ai.update(m_playerIsInvis);
        }
    }
    
    if (m_player->getComponent<CAnimation>().animation.getName() == "Die" || m_playerIsInvis) return;
    Pathfind path(m_entityManager);
    for (auto m : m_entityManager.getEntities("Missle"))
    {
        Vec2 pGridPos = Vec2((int)m_player->getComponent<CTransform>().pos.x / 64, 11 - (int)m_player->getComponent<CTransform>().pos.y / 64);
        Vec2 mGridPos = Vec2((int)m->getComponent<CTransform>().pos.x / 64, 11 - (int)m->getComponent<CTransform>().pos.y / 64);
        //std::cout <<gridPos.x << " " << gridPos.y << std::endl;
        Vec2 tar =  path.getTarget(pGridPos, mGridPos);

        //std::cout << tar.x << " " << tar.y << std::endl;
        

        auto target = gridToMidPixel(tar.x, tar.y, m); auto p = m->getComponent<CTransform>().pos;
        auto vel = m->getComponent<CTransform>().velocity;
        auto nVel = path.steer(vel, target, p, 0.1);
        //std::cout <<nVel.x << " " << nVel.y << std::endl;


        //auto p = m->getComponent<CTransform>().pos;

        Vec2 d = { p.x - target.x, p.y - target.y };
        /*auto l = sqrtf(d.x * d.x + d.y * d.y);
        Vec2 N = { -d.x / l, -d.y / l };
        Vec2 v = { 10 * N.x, 10 * N.y };*/

        /*std::cout << atan2f(d.x, -d.y);
        m->getComponent<CTransform>().angle = atan2f(d.x, -d.y);*/

        if (d.x < 0) m->getComponent<CTransform>().scale = m->getComponent<CTransform>().scale.abs();
        else m->getComponent<CTransform>().scale = m->getComponent<CTransform>().scale.abs() *-1;

        m->getComponent<CTransform>().velocity = nVel;
    }
}

// Adapted from https://www.youtube.com/watch?v=mr5xkf6zSzk&list=PLVmb_qp6XRcwzN9l5mcia6Gh3HOgut3bH&ab_channel=GDC
inline float Bezier5(float B, float C, float D, float E, float t)
{
    float s = 1.f - t;
    float t2 = t * t;
    float s2 = s * s;
    float t3 = t2 * t;
    float s3 = s2 * s;
    float t4 = t3 * t;
    float s4 = s3 * s;
    float t5 = t4 * t;
    return (5.f * B * s4 * t) + (15.f * C * s3 * t2) + (15.f * D * s2 * t3) + (5.f * E * s * t4) + (t5);
}

// Make the entity move toward the specified point
void Scene_Play::pathFindToPoint(std::shared_ptr<Entity> character, Vec2 goal, Vec2 start, float speed)
{
    Vec2 vecToGoal = goal - start;
    float& prog = character->getComponent<CPatrol>().progress;
    // https://www.youtube.com/watch?v=mr5xkf6zSzk&list=PLVmb_qp6XRcwzN9l5mcia6Gh3HOgut3bH&ab_channel=GDC
    prog += 0.016;
    float t = Bezier5(0.1, 0.14, 1.2, 0.85, prog);
    t = 1.f - fabs(1.f - t);
    Vec2 newPos = start + (vecToGoal * t);
    Vec2 newVelo = newPos - character->getComponent<CTransform>().pos;
    character->getComponent<CTransform>().velocity = newVelo;
    if (character->getComponent<CPatrol>().playerOn)
    {
        m_player->getComponent<CTransform>().pos += newVelo;
    }
}

float randFloatInRange(float min, float max)
{
    // This formula comes from https://stackoverflow.com/questions/686353/random-float-number-generation
    float r = min + (float)(rand()) / ((float)(RAND_MAX / (max - min)));
    return r;
}
float roundTo2Places(float var)
{
    // Formula from https://www.geeksforgeeks.org/rounding-floating-point-number-two-decimal-places-c-c/
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

// Sets up the glabal variables for the screen shake system
void Scene_Play::shakeScreen(float intensity, float duration)
{
    m_screenShakeIntensity = intensity;
    m_frameToStopScreenShake = (duration * m_frameRate) + m_currentFrame;
}

// Shakes the screen
void Scene_Play::sScreenShake()
{
    if (m_frameToStopScreenShake > m_currentFrame)
    {
        sf::View view = m_game->window().getView();
        view.move(-1 * m_currentViewOffset.x, -1 * m_currentViewOffset.y);
        Vec2 delta = Vec2(randFloatInRange(-1.0f, 1.0f) * m_screenShakeIntensity, randFloatInRange(-1.0f, 1.0f) * m_screenShakeIntensity);
        m_currentViewOffset = delta;
        view.move(m_currentViewOffset.x, m_currentViewOffset.y);
        m_game->window().setView(view);
    }
    else if (m_currentViewOffset.x != 0 || m_currentViewOffset.y != 0)
    {
        //Re-centers the view of the window after the shaking stops
        sf::View view = m_game->window().getView();
        view.move(-1 * m_currentViewOffset.x, -1 * m_currentViewOffset.y);
        m_currentViewOffset.x = 0;
        m_currentViewOffset.y = 0;
        m_game->window().setView(view);
    }
}

sf::Color Scene_Play::heathBarColorBlend(float t)
{
    int green = 255 * t;
    int red = 255 * (1 - t);
    return sf::Color(red, green, 0);
}

void Scene_Play::sStatus()
{
    for (auto& e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CInvincibility>())
        {
            if (e->getComponent<CInvincibility>().iframes == 0) e->removeComponent<CInvincibility>();
            e->getComponent<CInvincibility>().iframes -= 1;
        }
    }

    if (m_playerIsInvis && !(m_currentFrame < m_statusTimer))
    {
        m_playerIsInvis = false;
        //TURN ON PLAYER TRACKING
    }

    if (m_playerIsBig && !(m_currentFrame < m_statusTimer))
    {
        m_playerIsBig = false;
        for (int i = 0; i < m_playerEnlargementCount; i++)
        {
            enlarge(false);
        }
        m_playerEnlargementCount = 0;
    }

    if (m_playerIsFast && !(m_currentFrame < m_statusTimer))
    {
        m_playerIsFast = false;
        m_playerConfig.SPEED = m_basePlayerConfig.SPEED;
        m_playerConfig.JUMP = m_basePlayerConfig.JUMP;
    }
}

void Scene_Play::sLifespan()
{
    for each (auto & e in m_entityManager.getEntities())
    {
        if (e->isActive() && e->hasComponent<CLifeSpan>() && e->tag() != "pickup")
        {
            int timeAlive = m_currentFrame - e->getComponent<CLifeSpan>().frameCreated;
            if (timeAlive >= e->getComponent<CLifeSpan>().lifespan)
            {
                if (e->getComponent<CAnimation>().animation.getName() == "HealArrow")
                {
                    auto& aoe = m_entityManager.addEntity("aoe");
                    aoe->addComponent<CAnimation>(m_game->assets().getAnimation("Heal"), true);
                    aoe->addComponent<CTransform>(e->getComponent<CTransform>().pos);
                    aoe->addComponent<CBoundingBox>(m_game->assets().getAnimation("Heal").getSize());
                    aoe->addComponent<CLifeSpan>(120, currentFrame());
                }
                e->destroy();
            }
        }
    }

    // Respawn pickups by moving them back to spawn location
    for each (auto & e in m_entityManager.getEntities("pickup"))
    {
        if (e->isActive() && e->hasComponent<CLifeSpan>())
        {
            int timeAlive = m_currentFrame - e->getComponent<CLifeSpan>().frameCreated;
                if (timeAlive >= e->getComponent<CLifeSpan>().lifespan)
                {
                    e->getComponent<CTransform>().pos = e->getComponent<CTransform>().homePos;
                    e->removeComponent<CLifeSpan>();
                }
        }
    }
}

void Scene_Play::sCollision()
{
    // The player is considered in the air unless something else happens
    //m_player->getComponent<CState>().state = "Air";
    std::string futureState = "Air";

    Vec2 playerAdjustment = Vec2(0, 0);
    
    bool groundedFlag = false;

    if (m_player->getComponent<CAdvancedMovement>().wallJumpForgivenessCounter <= 0)
    {
        m_movingPlatPlayerRiding = nullptr;
    }

    // For every tile
    for (auto& e : m_entityManager.getEntities("tile"))
    {
        if (e->hasComponent<CPatrol>())
        {
            e->getComponent<CPatrol>().playerOn = false;
        }
        if (e->hasComponent<CBoundingBox>())
        {
            // Player on tile collisions
            Vec2 overlap = BasicCollision(e, m_player);
            if (overlap != Vec2(0, 0))
            {
                m_player->getComponent<CAdvancedMovement>().usesPhysics = false;
                m_player->getComponent<CTransform>().acceleration.x = 0;

                std::string tileName = e->getComponent<CAnimation>().animation.getName();
                Vec2 prevOverlap = Physics::GetPreviousOverlap(e, m_player);
                if (overlap.y != 0)
                {
                    playerAdjustment.y = overlap.y;
                    m_player->getComponent<CTransform>().velocity.y = 0;
                    // Stand on stuff
                    if (overlap.y < 0)
                    {
                        groundedFlag = true;

                        std::string newState;
                        // This logic probably shouldn't be here
                        if (!m_player->getComponent<CInput>().down || (m_slideAmount < 0.15 && m_slideAmount > 0))
                        {
                            newState = (m_player->getComponent<CInput>().left == m_player->getComponent<CInput>().right) ? "Stand" : "Run";
                        }
                        else {
                            newState = "Slide";
                            if (m_slideAmount == 0)
                            {
                                m_slideAmount = 1;
                            }
                        }
                        futureState = newState;
                        if (e->hasComponent<CPatrol>())
                        {
                            e->getComponent<CPatrol>().playerOn = true;
                            e->getComponent<CPatrol>().moving = true;
                            m_movingPlatPlayerRiding = e;
                        }
                    }
                    // If hit from beneath
                    else
                    {
                        // Break brick
                        if (tileName == "Bridge")
                        {
                            e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Explosion");
                            e->getComponent<CAnimation>().repeat = false;
                            e->removeComponent<CBoundingBox>();
                        }
                        // Activate question
                        else if (tileName == "Question")
                        {
                            e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Question2");
                            std::shared_ptr<Entity> coin = m_entityManager.addEntity("Coin");
                            coin->addComponent<CAnimation>(m_game->assets().getAnimation("Coin"), true);
                            coin->addComponent<CTransform>(Vec2(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y - 64));
                            coin->addComponent<CLifeSpan>(30, m_currentFrame);
                        }
                    }
                }
                // Fix horizontal clipping
                else if (overlap.x != 0)
                {
                    playerAdjustment.x = overlap.x;
                    m_player->getComponent<CAdvancedMovement>().lastWallSide = overlap.x / std::abs(overlap.x);
                    // Can wall jump
                    if (m_player->getComponent<CState>().state == "Air" && !m_player->getComponent<CAdvancedMovement>().shotInAir)
                    {
                        m_playerHasWallJumped = false;
                    }
                    if (e->hasComponent<CPatrol>())
                    {
                        m_movingPlatPlayerRiding = e;
                    }
                }
            }
            // Tile on bullet collisions
            for (auto& b : m_entityManager.getEntities("bullet"))
            {
                Vec2 overlap = Physics::GetOverlap(e, b);
                auto bulletName = b->getComponent<CAnimation>().animation.getName();
                if (overlap != Vec2(0, 0))
                {
                    // Destroy if brick
                    if (bulletName == "Bridge")
                    {
                        auto tile = m_entityManager.addEntity("tile");
                        auto pos = e->getComponent<CTransform>().pos;
                        if (b->getComponent<CTransform>().pos.x < b->getComponent<CTransform>().prevPos.x) //left check
                        {
                            pos.x += m_gridSize.x;
                        }
                        else //right check
                        {
                            pos.x -= m_gridSize.x;
                        }

                        tile->addComponent<CAnimation>(m_game->assets().getAnimation("Bridge"), true);
                        tile->addComponent<CTransform>(pos);
                        tile->addComponent<CBoundingBox>(m_game->assets().getAnimation("Bridge").getSize());
                        tile->addComponent<CLifeSpan>(120, currentFrame());
                    }
                    else if (bulletName == "HealArrow")
                    {
                        auto& aoe = m_entityManager.addEntity("aoe");
                        aoe->addComponent<CAnimation>(m_game->assets().getAnimation("Heal"), true);
                        aoe->addComponent<CTransform>(Vec2(e->getComponent<CTransform>().pos.x - 64, e->getComponent<CTransform>().pos.y));
                        aoe->addComponent<CBoundingBox>(m_game->assets().getAnimation("Heal").getSize());
                        aoe->addComponent<CLifeSpan>(120, currentFrame());
                    }
                    else if (e->getComponent<CAnimation>().animation.getName() == "mColumn3" && bulletName == "Arrow")
                    {
                        e->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Explosion");
                        e->getComponent<CAnimation>().repeat = false;
                        e->removeComponent<CBoundingBox>();
                    }
                    // Destroy bullet regardless
                    b->destroy();
                }
            }
        }
    }
    
    for (auto& aoe : m_entityManager.getEntities("aoe"))
    {
        if (BasicCollision(aoe, m_player) != Vec2(0, 0))
        {
            if (m_player->getComponent<CHealth>().current < m_player->getComponent<CHealth>().max)
            {
                m_player->getComponent<CHealth>().current += 1.0f / m_frameRate; // HPS = 1 (num / framerate)
                float heathPercentage = m_player->getComponent<CHealth>().current / m_player->getComponent<CHealth>().max;
                m_healthBar.setScale(sf::Vector2f(heathPercentage, 1));
                m_healthBar.setFillColor(heathBarColorBlend(heathPercentage));
            }
        }
    }
    for (auto& p : m_entityManager.getEntities("pickup"))
    {
        if (BasicCollision(p, m_player) != Vec2(0, 0))
        {
            // Replace this with something
            sPickUp(p->getComponent<CPickUp>().effect);
            m_game->playSound("GetItem");
            m_game->volSound("GetItem", options::m_optionSettings[1]);

            //move pickup out of game world upon collision
            p->getComponent<CTransform>().pos = Vec2(-5000, -5000);
            // respawn timer
            p->addComponent<CLifeSpan>(600, m_currentFrame);
        }
    }
    SetGrounded(groundedFlag);

    for (auto& g : m_entityManager.getEntities("goal"))
    {
        if (Physics::IsInside(m_player->getComponent<CTransform>().pos, g))
        {
            finishLevel();
        }
    }

    if (playerAdjustment != Vec2(0, 0))
    {
        // Adjust player pos
        m_player->getComponent<CTransform>().pos += playerAdjustment;
    }
    else if (m_player->getComponent<CState>().state == "Slide" && m_player->getComponent<CInput>().down) 
    {
        futureState = "Slide";
    }
    m_player->getComponent<CState>().state = futureState;

    // Reset player pos if player goes beneath the screen
    if (m_player->getComponent<CTransform>().pos.y - m_player->getComponent<CBoundingBox>().halfSize.y > 768)//m_game->window().getView().getSize().y) changed for multi views
    {
        playerDie();
    }
    // Keep the player from running off the left side of the map
    if (m_player->getComponent<CTransform>().pos.x - m_player->getComponent<CBoundingBox>().halfSize.x < 0)
    {
        m_player->getComponent<CTransform>().pos.x = m_player->getComponent<CBoundingBox>().halfSize.x;
    }

    for (auto& droid : m_entityManager.getEntities("Enemy"))
    {
        if (BasicCollision(droid, m_player) != Vec2(0, 0))
        {
            if (!m_player->hasComponent<CInvincibility>())
            {
                if (droid->hasComponent<CDamage>())
                {
                    m_game->playSound("PlayerHurt");
                    m_game->volSound("PlayerHurt", options::m_optionSettings[1]);
                    m_player->getComponent<CHealth>().current -= droid->getComponent<CDamage>().damage * options::m_optionSettings[0];
                    float heathPercentage = m_player->getComponent<CHealth>().current / m_player->getComponent<CHealth>().max;
                    m_healthBar.setScale(sf::Vector2f(heathPercentage, 1));
                    m_healthBar.setFillColor(heathBarColorBlend(heathPercentage));
                    m_player->addComponent<CInvincibility>(20);
                    if (m_player->getComponent<CHealth>().current <= 0) playerDie();
                }
            }
        }
        
        if (droid->tag() == "BossAtk") continue;
        if (droid->getComponent<CTransform>().pos.y > height()) //destroy droid if below the map
        {
            droid->destroy();
            continue;
        }

        // Keep the droid from running off the left side of the map
        if (droid->getComponent<CTransform>().pos.x - droid->getComponent<CBoundingBox>().halfSize.x < 0)
        {
            droid->getComponent<CTransform>().pos.x = droid->getComponent<CBoundingBox>().halfSize.x;
        }

        for (auto& bullet : m_entityManager.getEntities("bullet"))
        {
            Vec2 overlap = Physics::GetOverlap(droid, bullet);
            if (overlap != Vec2(0, 0))
            {
                if (bullet->hasComponent<CDamage>())
                {
                    m_game->playSound("EnemyHit");
                    m_game->volSound("EnemyHit", options::m_optionSettings[1]);
                    droid->getComponent<CHealth>().current -= bullet->getComponent<CDamage>().damage;
                    if (droid->getComponent<CHealth>().current <= 0)
                    {
                        droid->getComponent<CState>().state = "Death";
                        m_game->playSound("EnemyDie");
                        m_game->volSound("EnemyDie", options::m_optionSettings[1]);
                    }
                    bullet->destroy();
                }
            }
        }
        for (auto& tile : m_entityManager.getEntities("tile"))
        {
            droid->getComponent<CTransform>().velocity.y = 0;
            droid->getComponent<CTransform>().pos += BasicCollision(tile, droid);
        }
    }

    for (auto& missle : m_entityManager.getEntities("Missle"))
    {
        for (auto& tile : m_entityManager.getEntities("tile"))
        {
            missle->getComponent<CTransform>().pos += BasicCollision(tile, missle)/4;
        }
        if (Physics::GetOverlap(missle, m_player) != Vec2(0, 0))
        {
            missle->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Explosion");
            missle->getComponent<CAnimation>().repeat = false;
            missle->removeComponent<CBoundingBox>();
            missle->getComponent<CTransform>().velocity = Vec2(0, 0);
            m_player->getComponent<CHealth>().current -= 3.0f;
            float heathPercentage = m_player->getComponent<CHealth>().current / m_player->getComponent<CHealth>().max;
            m_healthBar.setScale(sf::Vector2f(heathPercentage, 1));
            m_healthBar.setFillColor(heathBarColorBlend(heathPercentage));
            m_player->addComponent<CInvincibility>(20);
            if (m_player->getComponent<CHealth>().current <= 0) playerDie();
        }
    }
}

Vec2 Scene_Play::BasicCollision(std::shared_ptr<Entity> tile, std::shared_ptr<Entity> character)
{
    // Set up the adjustment variable
    Vec2 adjustment = Vec2(0, 0);
    // See if the things overlap 
    Vec2 overlap = Physics::GetOverlap(tile, character);
    // If so
    if (overlap != Vec2(0, 0))
    {
        // Get the previous overlap
        Vec2 prevOverlap = Physics::GetPreviousOverlap(tile, character);
        // Set the adjustment correctly
        if (prevOverlap.x > 0)
        {
            Vec2 yCompare = Vec2(character->getComponent<CTransform>().prevPos.y, tile->getComponent<CTransform>().prevPos.y);
            int adjustmentDir = (yCompare.x > yCompare.y) ? 1 : -1;
            adjustment.y += overlap.y * adjustmentDir;
        }
        else if (prevOverlap.y > 0)
        {
            Vec2 xCompare = Vec2(character->getComponent<CTransform>().prevPos.x, tile->getComponent<CTransform>().prevPos.x);
            int adjustmentDir = (xCompare.x > xCompare.y) ? 1 : -1;
            adjustment.x += overlap.x * adjustmentDir;
        }
    }
    // Return the adjustment
    return adjustment;
}

void Scene_Play::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "TOGGLE_TEXTURE") { m_drawTextures = !m_drawTextures; }
        else if (action.name() == "TOGGLE_COLLISION") { m_drawCollision = !m_drawCollision; }
        else if (action.name() == "TOGGLE_GRID") { m_drawGrid = !m_drawGrid; }
        else if (action.name() == "TOGGLE_FANCY_LIGHTING") { m_useFancyLight = !m_useFancyLight; }
        else if (action.name() == "SWITCH") { switchWeapon(); }
        
        else if (action.name() == "PAUSE")
        {
            sf::Texture texture;
            texture.create(m_game->window().getSize().x, m_game->window().getSize().y);
            texture.update(m_game->window());
            m_game->stopSound("Play");
            m_game->changeScene("PAUSEMENU", std::make_shared<Scene_Pause>(m_game, m_weaponConfig.ammo, texture), false);
        }
        else if (action.name() == "QUIT") { onEnd(); }
        else if (action.name() == "LEFT" && !m_control->getComponent<CInput>().left) {
            m_control->getComponent<CInput>().left = true;
        }
        else if (action.name() == "RIGHT" && !m_control->getComponent<CInput>().right) {
            m_control->getComponent<CInput>().right = true;
        }
        else if (action.name() == "UP" && !m_control->getComponent<CInput>().up) {
            m_control->getComponent<CInput>().up = true;
            m_control->getComponent<CInput>().tryJump = true;
            //std::cout << "Up" << std::endl;
        }
        else if (action.name() == "DOWN" && !m_control->getComponent<CInput>().down) {
            m_control->getComponent<CInput>().down = true;
        }
        else if (action.name() == "SPACE" && !m_control->getComponent<CInput>().shoot) {
            // Shoot
            attack(m_player);
            m_player->getComponent<CInput>().shoot = true;
        }
        else if (action.name() == "SMOOTH") {
            m_smoothedCamera = !m_smoothedCamera;
        }
    }
    // Reset the input variables in general
    else if (action.type() == "END")
    {
        if (action.name() == "LEFT" && m_control->getComponent<CInput>().left) {
            m_control->getComponent<CInput>().left = false;
        }
        else if (action.name() == "RIGHT" && m_control->getComponent<CInput>().right) {
            m_control->getComponent<CInput>().right = false;
        }
        else if (action.name() == "UP" && m_control->getComponent<CInput>().up) {
            m_control->getComponent<CInput>().up = false;
            //m_player->getComponent<CAdvancedMovement>().hasJumped = false;
        }
        else if (action.name() == "DOWN" && m_control->getComponent<CInput>().down) {
            m_control->getComponent<CInput>().down = false;
            m_slideAmount = 0;
        }
        else if (action.name() == "SPACE" && m_control->getComponent<CInput>().shoot) {
            m_control->getComponent<CInput>().shoot = false;
        }
    }
}

void Scene_Play::sDoMouseAction(const Action& action)
{
}

void Scene_Play::sPickUp(int index) 
{
    switch (index) {
    case 0: //invisibility
        std::cout << "Invisibility" << std::endl;
        m_playerIsInvis = true;
        m_statusTimer = m_currentFrame + 300;
        //TURN OFF PLAYER DETECTION
        break;
    case 1: //speed up
        std::cout << "SPEED" << std::endl;
        m_playerIsFast = true;
        m_statusTimer = m_currentFrame + 300;
        m_playerConfig.SPEED *= 1.75;
        m_playerConfig.JUMP *= 1.2;
        break;
    case 2: //slow mo
        std::cout << "BEEG" << std::endl;
        m_playerIsBig = true;
        m_statusTimer = m_currentFrame + 300;
        m_playerEnlargementCount++;
        enlarge(true);
        break;
    case 3:
        m_weaponConfig.ammo[1]++;
        m_ammoCount.setString(ammoCountStringFormatter(m_weaponConfig.ammo[m_weaponConfig.currentWeapon]));
        break;
    case 4:
        m_weaponConfig.ammo[2]++;
        m_ammoCount.setString(ammoCountStringFormatter(m_weaponConfig.ammo[m_weaponConfig.currentWeapon]));
        break;
    case 5:
        m_weaponConfig.ammo[3]++;
        m_ammoCount.setString(ammoCountStringFormatter(m_weaponConfig.ammo[m_weaponConfig.currentWeapon]));
        break;
    default:
        std::cout << "ERROR" << std::endl;
        break;
    }
}

void Scene_Play::sAnimation()
{
    // Set the players animation correctly
    if (m_player->getComponent<CAnimation>().animation.getOverride())
    {
        if (m_player->getComponent<CAnimation>().animation.hasEnded())
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation(m_player->getComponent<CState>().state);
        }
    }
    else
    {
        if (m_player->getComponent<CAnimation>().animation.getName() != m_game->assets().getAnimation(m_player->getComponent<CState>().state).getName())
        {
            m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation(m_player->getComponent<CState>().state);
        }
    }

    // Destroy the entity if a non-looping animation finishes
    for (auto e : m_entityManager.getEntities())
    {
        if (e->hasComponent<CAnimation>())
        {
            e->getComponent<CAnimation>().animation.update();
            if (e->getComponent<CAnimation>().animation.hasEnded() && !e->getComponent<CAnimation>().repeat)
            {
                if (e == m_player)
                {
                    m_player->getComponent<CAnimation>().animation = m_game->assets().getAnimation("Air");
                    m_player->getComponent<CAnimation>().repeat = true;
                    m_player->getComponent<CTransform>().pos = gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player);
                }
                else if (e->getComponent<CState>().state == "Tele0")
                {
                    e->getComponent<CState>().state = "Tele1";
                    e->getComponent<CAnimation>().animation = m_game->assets().getAnimation(e->tag() + e->getComponent<CState>().state);
                    //Warp behind and face player
                    e->getComponent<CTransform>().pos = Vec2(m_player->getComponent<CTransform>().pos.x + -50 * m_player->getComponent<CTransform>().scale.x, m_player->getComponent<CTransform>().pos.y);
                    auto eDir = m_player->getComponent<CTransform>().pos.x - e->getComponent<CTransform>().pos.x;
                    if (eDir > 0)
                    {
                        e->getComponent<CTransform>().scale.x = abs(e->getComponent<CTransform>().scale.x);
                    }
                    else
                    {
                        e->getComponent<CTransform>().scale.x = abs(e->getComponent<CTransform>().scale.x) * -1;
                    }
                }
                else if (e->getComponent<CState>().state == "Tele1")
                {
                    e->getComponent<CState>().state = "Atk";
                    e->getComponent<CAnimation>().animation = m_game->assets().getAnimation(e->tag() + e->getComponent<CState>().state);
                    if (m_entityManager.getEntities("BossAtk").size() == 0)
                    {
                        auto atk = m_entityManager.addEntity("BossAtk");
                        atk->addComponent<CTransform>(Vec2(e->getComponent<CTransform>().pos.x + 250, e->getComponent<CTransform>().pos.y + 10));
                        atk->addComponent<CBoundingBox>(Vec2(700, 70));
                        atk->getComponent<CBoundingBox>().blockVision = false;
                        atk->addComponent<CLifeSpan>(30, m_currentFrame);
                    }
                }
                else if (std::find(std::begin(m_entityManager.enemies), std::end(m_entityManager.enemies), e->tag()) != std::end(m_entityManager.enemies) && e->getComponent<CState>().state == "Atk")
                {
                    //if its an enemy and attacking
                    e->getComponent<CState>().state = "Walk";
                    e->getComponent<CAnimation>().repeat = true;
                }
                else
                {
                    if (e->tag() == "Boss") finishLevel();
                    e->destroy();
                }
            }
        }
    }
}

std::string Scene_Play::ammoCountStringFormatter(int ammoCount)
{
    if (ammoCount <= -1)
    {
        return "99";
    }
    else if (ammoCount < 10)
    {
        return "0" + std::to_string(ammoCount);
    }
    else
    {
        return std::to_string(ammoCount);
    }
}

void Scene_Play::onEnd()
{
    m_game->changeScene("OVERWORLD", nullptr, true);
    m_game->stopSound("Play");
    m_game->stopSound("Win");
    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", options::m_optionSettings[2]);
}

void Scene_Play::drawLine(const Vec2 & p1, const Vec2 & p2)
{
    sf::Vertex line[] = { sf::Vector2f(p1.x, p1.y), sf::Vector2f(p2.x, p2.y) };
    m_game->window().draw(line, 2, sf::Lines);
}

void Scene_Play::sCamera()
{
         if (m_won)                      sWinCam();
    else if (m_cameraTarget == m_player) sPlayerCamera();
    else                                 sBulletCamera();
}

void Scene_Play::sPlayerCamera()
{
    // color the background darker so you know that the game is paused
    if (!m_paused) { m_game->window().clear(sf::Color(0, 0, 0)); }
    else { m_game->window().clear(sf::Color(0, 0, 0)); }

    // set the viewport of the window to be centered on the player if it's far enough right
    auto& pPos = m_player->getComponent<CTransform>().pos;
    sf::View view = m_game->window().getView();

    float idk = pPos.x;
    if (m_smoothedCamera) 
    {
        float scale = 0.025;
        idk = (pPos.x - view.getCenter().x) * scale;
        //std::cout << "Smooth" << std::endl;
        //if ((idk < 1 && idk > 0) || (idk < 0 && idk > -1)) { idk *= 40; }
        idk += view.getCenter().x;
    }

    m_xCenter = std::max(m_game->window().getSize().x / 2.0f, idk);
    m_gradientShader.setParameter("center", m_goalPos.x - (m_xCenter - 640), m_goalPos.y);
    m_timerText.setPosition(m_xCenter - (m_game->window().getSize().x / 2.0f) + 10, 10);
    m_deathCountText.setPosition(m_xCenter + (m_game->window().getSize().x / 2.0f) - 220, 10);
    m_healthBar.setPosition(sf::Vector2f(m_xCenter - 230, 768 - 20));
    m_healthBarBackground.setPosition(sf::Vector2f(m_xCenter - 230, 768 - 20));
    m_ammoIconRect.setPosition(sf::Vector2f(m_xCenter + 540, 768 - 15 - 32));
    m_ammoCount.setPosition(sf::Vector2f(m_xCenter + 590, 768 - 32 - 8));
    view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    view.setCenter(m_xCenter, 384);
    view.setSize(sf::Vector2f(1280, 768));

    m_game->window().setView(view);
    sScreenShake();
    sRender();
}

void Scene_Play::sBulletCamera()
{
    // color the background darker so you know that the game is paused
    if (!m_paused) { m_game->window().clear(sf::Color(0, 0, 0)); }
    else { m_game->window().clear(sf::Color(0, 0, 0)); }

    if (!m_cameraTarget || !m_cameraTarget->isActive()) m_cameraTarget = m_player;
    // set the viewport of the window to be centered on the player if it's far enough right

    // main cam
    sf::View mainView = m_game->window().getView();
    mainView.setSize(sf::Vector2f(1280, 768));
    auto& pPos = m_cameraTarget->getComponent<CTransform>().pos;
    

    m_xCenter = std::max(m_game->window().getSize().x / 2.0f, pPos.x);
    m_gradientShader.setParameter("center", m_goalPos.x - (m_xCenter - 640), m_goalPos.y);
    m_timerText.setPosition(m_xCenter - (m_game->window().getSize().x / 2.0f) + 10, 10);
    m_deathCountText.setPosition(m_xCenter + (m_game->window().getSize().x / 2.0f) - 220, 10);
    m_healthBar.setPosition(sf::Vector2f(m_xCenter - 230, 768 - 20));
    m_healthBarBackground.setPosition(sf::Vector2f(m_xCenter - 230, 768 - 20));
    m_ammoIconRect.setPosition(sf::Vector2f(m_xCenter + 540, 768 - 15 - 32));
    m_ammoCount.setPosition(sf::Vector2f(m_xCenter + 600, 768 - 15 - 32));
    mainView.setCenter(m_xCenter, 384);
    mainView.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    mainView.setSize(sf::Vector2f(1280, 768));
    m_game->window().setView(mainView);
    sScreenShake();
    sRender();
    

    //Background for mini cam
    sf::RectangleShape rect;
    auto xPos = m_control->getComponent<CTransform>().pos.x - 640;
    if (xPos < 0) xPos = 0;
    rect.setSize(sf::Vector2f(m_game->window().getSize().x / 4.0f, m_game->window().getSize().y / 4.0f));
    rect.setPosition(xPos, 576);
    rect.setFillColor(sf::Color(0, 0, 0, 0));
    rect.setOutlineColor(sf::Color(50, 75, 50));
    rect.setOutlineThickness(5);
    
    m_game->window().draw(rect);

    // mini cam
    auto& pos = m_player->getComponent<CTransform>().pos;
    float m_xCenter2 = std::max(m_game->window().getSize().x / 2.0f, pos.x);
    sf::View miniView = m_game->window().getView();
    m_timerText.setPosition(m_xCenter2 - (m_game->window().getSize().x / 2.0f) + 10, 10);
    m_deathCountText.setPosition(m_xCenter2 + (m_game->window().getSize().x / 2.0f) - 220, 10);
    miniView.setCenter(m_player->getComponent<CTransform>().pos.x, m_player->getComponent<CTransform>().pos.y);
    miniView.setViewport(sf::FloatRect(0.f, 0.75f, 0.25f, 0.25f));
    miniView.setSize(sf::Vector2f(320, 192));
    m_game->window().setView(miniView);
    sRender();
}

void Scene_Play::sWinCam()
{
    m_game->window().clear(sf::Color(0, 0, 0));
    auto nesWrestler = m_entityManager.getEntities("wrestler")[0];
    nesWrestler->getComponent<CTransform>().pos = Vec2(640, 200);

    auto& transform = nesWrestler->getComponent<CTransform>();

    auto& animation = nesWrestler->getComponent<CAnimation>().animation;
    animation.getSprite().setRotation(transform.angle);
    animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
    animation.getSprite().setScale(transform.scale.x, transform.scale.y);

    m_game->window().draw(animation.getSprite());

    m_gridText.setString("A WINNER IS YOU");
    m_gridText.setPosition(560, 300);
    m_gridText.setScale(2, 2);
    m_game->window().draw(m_gridText);
}

void Scene_Play::sRender()
{
    //TODO: Make this stuff global stop using so much computing power idiot
    //

    /*sf::Texture parallax4;
    parallax4.loadFromFile("images/parallax/cave4.png");
    parallax4.setRepeated(true);*/

    std::string level = m_levelPath.substr(0, m_levelPath.length() - 4);

    m_backText4.back()->loadFromFile("images/" + level + "/back4.png");
    m_backText4.back()->setRepeated(true);
    sf::Sprite par4;
    par4.setTexture(*m_backText4.back());
    par4.setScale(768 / 64, 768 / 64);
    par4.setOrigin(0, 0);

    m_backText3.back()->loadFromFile("images/" + level + "/back3.png");
    m_backText3.back()->setRepeated(true);
    sf::Sprite par3;
    par3.setTexture(*m_backText3.back());
    par3.setScale(768 / 64, 768 / 64);
    par3.setOrigin(0, 0);

    m_backText2.back()->loadFromFile("images/" + level + "/back2.png");
    m_backText2.back()->setRepeated(true);
    sf::Sprite par2;
    par2.setTexture(*m_backText2.back());
    par2.setScale(768 / 64, 768 / 64);
    par2.setOrigin(0, 0);

    m_backText1.back()->loadFromFile("images/" + level + "/back1.png");
    m_backText1.back()->setRepeated(true);
    sf::Sprite par1;
    par1.setTexture(*m_backText1.back());
    par1.setScale(768 / 64, 768 / 64);
    par1.setOrigin(0, 0);

    for (int i = 0; i < 3; i++)
    {
        m_background4.push_back(par4);
        m_background3.push_back(par3);
        m_background2.push_back(par2);
        m_background1.push_back(par1);
    }

    updateBackground(m_background4, 1, 3);
    updateBackground(m_background3, 0.75, 2);
    updateBackground(m_background2, 0.5, 1);
    updateBackground(m_background1, 0.25, 0);

    for (int i = 0; i < 3; i++)
    {
        m_game->window().draw(m_background4[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        m_game->window().draw(m_background3[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        m_game->window().draw(m_background2[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        m_game->window().draw(m_background1[i]);
    }

    if (m_drawTextures)
    {
        for (auto& e : m_onScreenText)
        {
            m_game->window().draw(e);
        }
        for (auto e : m_entityManager.getEntities())
        {
            auto & transform = e->getComponent<CTransform>();

            if (e->hasComponent<CAnimation>())
            {
                auto& animation = e->getComponent<CAnimation>().animation;
                animation.getSprite().setRotation(transform.angle);
                animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
                animation.getSprite().setScale(transform.scale.x, transform.scale.y);
                if (e->hasComponent<CAdvancedMovement>() && m_playerIsInvis) // if player
                {
                    m_game->window().draw(animation.getSprite(), & m_invisShader);
                }
                else if (e->hasComponent<CAdvancedMovement>() && m_playerIsFast)
                {
                    m_game->window().draw(animation.getSprite(), & m_fastShader);
                }
                else
                {
                    m_game->window().draw(animation.getSprite());
                }
            }
        }
    }

    if (m_useFancyLight)
    {
        updateFancyLighting(false);
        for (int i = 0; i < m_lightPolygons.size(); i++)
        {
            m_game->window().draw(m_lightPolygons[i], &m_gradientShader);
        }
    }
    

    float currentTime = (m_currentFrame - m_timerOffset) / 60.0f;
    std::string timerString = std::to_string(currentTime);
    timerString = timerString.substr(0, timerString.find(".")+4);
    m_timerText.setString(timerString);
    m_game->window().draw(m_timerText);
    m_game->window().draw(m_deathCountText);
    m_game->window().draw(m_healthBarBackground);
    m_game->window().draw(m_healthBar);
    m_game->window().draw(m_ammoIconRect);
    m_game->window().draw(m_ammoCount);

    // draw all Entity collision bounding boxes with a rectangleshape
    if (m_drawCollision)
    {
        for (auto e : m_entityManager.getEntities())
        {
            if (e->hasComponent<CBoundingBox>())
            {
                auto & box = e->getComponent<CBoundingBox>();
                auto & transform = e->getComponent<CTransform>();
                sf::RectangleShape rect;
                rect.setSize(sf::Vector2f(box.size.x-1, box.size.y-1));
                rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
                rect.setPosition(transform.pos.x, transform.pos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 0));
                rect.setOutlineColor(sf::Color(255, 255, 255, 255));
                rect.setOutlineThickness(2);
                m_game->window().draw(rect);
            }
        }
    }

    // draw the grid so that students can easily debug
    if (m_drawGrid)
    {
        float leftX = m_game->window().getView().getCenter().x - width() / 2;
        float rightX = leftX + width() + m_gridSize.x;
        float nextGridX = leftX - ((int)leftX % (int)m_gridSize.x);

        for (float x = nextGridX; x < rightX; x += m_gridSize.x)
        {
            drawLine(Vec2(x, 0), Vec2(x, height()));
        }

        for (float y = 0; y < height(); y += m_gridSize.y)
        {
            drawLine(Vec2(leftX, height() - y), Vec2(rightX, height() - y));

            for (float x = nextGridX; x < rightX; x += m_gridSize.x)
            {
                std::string xCell = std::to_string((int)x / (int)m_gridSize.x);
                std::string yCell = std::to_string((int)y / (int)m_gridSize.y);
                m_gridText.setString("(" + xCell + "," + yCell + ")");
                m_gridText.setPosition(x + 3, height() - y - m_gridSize.y + 2);
                m_game->window().draw(m_gridText);
            }
        }
    }
}

//https://www.youtube.com/watch?v=zit45k6CUMk&ab_channel=Dani
void Scene_Play::updateBackground(std::vector<sf::Sprite>& background, float parallaxVal, int camStartIndex)
{
    float temp = (m_xCenter * (1 - parallaxVal));
    float dist = (m_xCenter * parallaxVal);
    float length = background[0].getTexture()->getSize().x * 12;

    for (int i = 0; i < 3; i++)
    {
        background[i].setPosition(length * (i - 1) + dist + m_camStartX[camStartIndex], 0);
    }

    if (temp > m_camStartX[camStartIndex] + length) { m_camStartX[camStartIndex] += length; }
    else if (temp < m_camStartX[camStartIndex] - length) { m_camStartX[camStartIndex] -= length; }
}
