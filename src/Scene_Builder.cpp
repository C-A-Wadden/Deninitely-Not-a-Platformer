#include "Scene_Overworld.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "Scene_Menu.h"
#include "Scene_Builder.h"

Scene_Builder::Scene_Builder(GameEngine* gameEngine, std::string level)
    : Scene(gameEngine)
{
    init(level);
}

void Scene_Builder::init(std::string level)
{
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Enter, "ENTER");

    registerAction(sf::Keyboard::Y, "PICKUP");
    registerAction(sf::Keyboard::T, "TILE");
    registerAction(sf::Keyboard::R, "DEC");
    registerAction(sf::Keyboard::E, "ENEMY");

    registerAction(sf::Keyboard::S, "SAVE");

    registerAction(sf::Keyboard::F, "FOLLOW");
    registerAction(sf::Keyboard::P, "PATROL");

    registerAction(sf::Keyboard::H, "HEALTH");
    registerAction(sf::Keyboard::G, "DAMAGE");
    registerAction(sf::Keyboard::J, "SPEED");
    registerAction(sf::Keyboard::BackSpace, "DELETE");
    registerAction(sf::Keyboard::Right, "FORWARD");
    registerAction(sf::Keyboard::Left, "BACKWARD");
    registerMouseAction(sf::Mouse::Button::Left, "LEFTCLICK");
    registerMouseAction(sf::Mouse::Button::Right, "RIGHTCLICK");

    m_gridText.setCharacterSize(12);
    m_gridText.setFont(m_game->assets().getFont("Arial"));
    m_noticeText.setCharacterSize(30);
    m_noticeText.setFont(m_game->assets().getFont("Metal"));
    
    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    m_animations = { "Float1","Float2","Float3","Float4","Float5","Float6","Float7","Float8","Float9","FloatBL",
                    "FloatBM","FloatBR","FloatML","FloatMM","FloatMR","FloatTL","FloatTM", "FloatTR", "Bridge", "Ground2",
                    "Ground3", "lWall1","lWall2", "lWall3", "rWall1","rWall2", "rWall3","mColumn1", "mColumn2", "mColumn3",
                    "mColumn4", "Ceiling"};

    m_decorations = { "bFloat1","bFloat2","bFloat3","bFloat4","bFloat5","bFloat6","FloatBL",
                    "FloatBM","FloatBR","FloatML","FloatMM","FloatMR","FloatTL","FloatTM", "FloatTR", "bGrass1",
                    "bGrass2", "bGrass3", "BushL", "BushM", "BushR", "Fence1", "Fence2", "Fence3", "Fence4", "PostL",
                    "PostR",  "Grass1", "Grass2", "Grass3", "Grass4", "GrassL", "GrassR", "Ground1", "Ground4", "AC1",
                    "AC2", "AC3", "AC4", "bPipe1L", "bPipe1M", "bPipe1R", "bPipe2L", "bPipe2M", "bPipe2R", "cWindow1L",
                    "cWindow1M","cWindow1R","cWindow2L","cWindow2M","cWindow2R","cWindow3L","cWindow3M", "cWindow3R",
                    "cWindow4L","cWindow4M","cWindow4R","Grate1","Grate2","hWindow1L","hWindow1M","hWindow1R","hWindow2L",
                    "hWindow2M","hWindow2R","hWindow3L","hWindow3M","hWindow3R","hWindow4L","hWindow4M","hWindow4R",
                    "Ladder1","Ladder2","lColumn1L","lColumn1R","lColumn2L","lColumn2R","lColumn3L","lColumn3R","lColumn4L",
                    "lColumn4R","oWindow1L","oWindow1M","oWindow1R","oWindow2L","oWindow2M","oWindow2R","oWindow3L","oWindow3M",
                    "oWindow3R","oWindow4L","oWindow4M","oWindow4R","PC1","PC2","rColumn1L","rColumn1R","rColumn2L","rColumn2R",
                    "rColumn3L","rColumn3R","rColumn4L","rColumn4R","sPipe1","sPipe2","Antenna1","Antenna2","Antenna3",
                    "Antenna4","Antenna5","sWindow1L","sWindow1M","sWindow1R","sWindow2L","sWindow2M","sWindow2R","sWindow3L",
                    "sWindow3M","sWindow3R","sWindow4L","sWindow4M","sWindow4R","Pipe1","Pipe2","Pipe3","Wire1","Wire2"};

    m_pickups = { "Invisible", "Fast", "Fridge", "Bridge", "ArrowIcon", "HeartIcon" };

    m_enemies = {"DroidWalk", "GuardWalk", "GoomBotWalk"};
    m_enemyNames = { "Droid", "Guard", "GoomBot" };

    m_screenOffset = 0;
    m_level = level;
    m_speedChange = false;
    m_damageChange = false;
    m_healthChange = false;
    m_dragging = false;
    m_aiSelection = false;
    m_patrolSelection = false;
    m_addingTile = false;
    m_addingDec = false;
    m_addingEnemies = false;
    m_addingPickup = false;
    m_makeMoving = false;
    readLevel();
    SpawnPlayer();
}

Vec2 Scene_Builder::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
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

void Scene_Builder::midPixelToGrid(std::shared_ptr<Entity> entity)
{
    int outX = entity->getComponent<CTransform>().pos.x/m_gridSize.x;
    int outY = abs((entity->getComponent<CTransform>().pos.y/m_gridSize.y) - 12);

    Vec2 positions = gridToMidPixel(outX, outY, entity);

    entity->getComponent<CTransform>().pos = positions;
}

Vec2 Scene_Builder::saveGrid(std::shared_ptr<Entity> entity)
{
    int outX = entity->getComponent<CTransform>().pos.x / m_gridSize.x;
    int outY = abs((entity->getComponent<CTransform>().pos.y / m_gridSize.y) - 12);

    return Vec2(outX, outY);
}


void Scene_Builder::readLevel()
{
    // Reset the entity manager when the level loads
    m_entityManager = EntityManager();

    // Set up some variables for later
    std::string name, lineLabel;
    float GX, GY;

    // Open the specified level file
    std::ifstream fin(m_level);

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
            Vec2 goalPos = gridToMidPixel(GX + 0.5, GY + 1, brick);
            brick->addComponent<CTransform>(goalPos);
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
        }
        else if (lineLabel == "NPC")
        {
            int H, D, R, S, scale = 3;
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
                    pos.push_back(Vec2(PX,PY));
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
}

void::Scene_Builder::save()
{
    // Open the specified level file
    std::ofstream fin(m_level, std::ofstream::out | std::ofstream::trunc);
    
    for (auto e : m_entityManager.getEntities())
    {
        Vec2 pos = saveGrid(e);
        if (e->tag() == "player") //write player
        {
            auto bound = e->getComponent<CBoundingBox>().size;
            fin << "Player" << " " << pos.x << " " << pos.y << " " << bound.x / 3 << " " << bound.y / 3 << " " << m_playerConfig.SPEED << " " << m_playerConfig.JUMP << " " << m_playerConfig.MAXSPEED << " " << m_playerConfig.GRAVITY << "\n";
        }
        else if (e->tag() == "tile") //write tiles
        {
            if (e->hasComponent<CPatrol>()) //moving tiles
            {
                auto ai = e->getComponent<CPatrol>();
                fin << "MovingTile" << " " + e->getComponent<CAnimation>().animation.getName() + " " << pos.x << " " << pos.y << " " << ai.trigger << " " << ai.speed << " " << ai.positions.size();

                for (int i = 0; i < ai.positions.size(); i++)
                {
                    fin << " " << ai.positions[i].x << " " << ai.positions[i].y;
                }
                fin << "\n";
            }
            else
            {
                fin << "Tile" << " " + e->getComponent<CAnimation>().animation.getName() << " " << pos.x << " " << pos.y << "\n" ;
            }
        }
        else if (e->tag() == "dec") //write decorations
        {
            fin << "Dec" << " " + e->getComponent<CAnimation>().animation.getName() << " " << pos.x << " " << pos.y << "\n";
        }
        else if (e->tag() == "enemy")
        {
            if (e->hasComponent<CPatrol>())
            {
                auto health = e->getComponent<CHealth>().current;
                auto damage = e->getComponent<CDamage>().damage;
                auto patrol = e->getComponent<CPatrol>();

                if (e->getComponent<CAnimation>().animation.getName() == "DroidWalk")
                {
                    fin << "NPC" << " " << "Droid" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 120 << " " << "Patrol" << " " << patrol.speed << " " << patrol.positions.size();
                }
                else if (e->getComponent<CAnimation>().animation.getName() == "GoomBotWalk")
                {
                    fin << "NPC" << " " << "GoomBot" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Patrol" << " " << patrol.speed << " " << patrol.positions.size();
                }
                else if (e->getComponent<CAnimation>().animation.getName() == "GuardWalk")
                {
                    fin << "NPC" << " " << "Guard" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Patrol" << " " << patrol.speed << " " << patrol.positions.size();
                }

                for (int i = 0; i < patrol.positions.size(); i++)
                {
                    fin << " " << patrol.positions[i].x << " " << patrol.positions[i].y;
                }
                fin << "\n";
            }
            else if (e->hasComponent<CFollowPlayer>()) 
            {
                auto health = e->getComponent<CHealth>().current;
                auto damage = e->getComponent<CDamage>().damage;
                auto follow = e->getComponent<CFollowPlayer>();
                if (e->getComponent<CAnimation>().animation.getName() == "DroidWalk")
                {
                    fin << "NPC" << " " << "Droid" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 120 << " " << "Follow" << " " << follow.speed << "\n";
                }
                else if (e->getComponent<CAnimation>().animation.getName() == "GoomBotWalk")
                {
                    fin << "NPC" << " " << "GoomBot" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Follow" << " " << follow.speed << "\n";
                }
                else if (e->getComponent<CAnimation>().animation.getName() == "GuardWalk")
                {
                    fin << "NPC" << " " << "Guard" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Follow" << " " << follow.speed << "\n";
                }
            }
        }
        else if (e->tag() == "pickup") //write pickups
        {
            fin << "PickUp" << " " << e->getComponent<CAnimation>().animation.getName() << " " << pos.x << " " << pos.y << " " << e->getComponent<CPickUp>().effect << "\n";
        }
        else if (e->tag() == "goal") //write goal
        {
            fin << "Goal" << " " << pos.x << " " << pos.y - 1 << "\n";
        }
        else if (e->tag() == "GoomBot") //write goomba
        {
            auto health = e->getComponent<CHealth>().current;
            auto damage = e->getComponent<CDamage>().damage;
            if (e->hasComponent<CFollowPlayer>())
            {
                auto follow = e->getComponent<CFollowPlayer>();
                fin << "NPC" << " " << "GoomBot" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Follow" << " " << follow.speed << "\n";
            }
            else if (e->hasComponent<CPatrol>())
            {
                auto patrol = e->getComponent<CPatrol>();
                fin << "NPC" << " " << "GoomBot" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Patrol" << " " << patrol.speed << " " << patrol.positions.size();

                for (int i = 0; i < patrol.positions.size(); i++)
                {
                    fin << " " << patrol.positions[i].x << " " << patrol.positions[i].y;
                }
                fin << "\n";
            }
        }
        else if (e->tag() == "Guard") //write guard
        {
            auto health = e->getComponent<CHealth>().current;
            auto damage = e->getComponent<CDamage>().damage;
            if (e->hasComponent<CFollowPlayer>())
            {
                auto follow = e->getComponent<CFollowPlayer>();
                fin << "NPC" << " " << "Guard" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Follow" << " " << follow.speed << "\n";
            }
            else if (e->hasComponent<CPatrol>())
            {
                auto patrol = e->getComponent<CPatrol>();
                fin << "NPC" << " " << "Guard" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 30 << " " << "Patrol" << " " << patrol.speed << " " << patrol.positions.size();

                for (int i = 0; i < patrol.positions.size(); i++)
                {
                    fin << " " << patrol.positions[i].x << " " << patrol.positions[i].y;
                }
                fin << "\n";
            }
        }
        else if (e->tag() == "Droid") //write droid
        {
            auto health = e->getComponent<CHealth>().current;
            auto damage = e->getComponent<CDamage>().damage;
            if (e->hasComponent<CFollowPlayer>())
            {
                auto follow = e->getComponent<CFollowPlayer>();
                fin << "NPC" << " " << "Droid" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 120 << " " << "Follow" << " " << follow.speed << "\n";
            }
            else if (e->hasComponent<CPatrol>())
            {
                auto patrol = e->getComponent<CPatrol>();
                fin << "NPC" << " " << "Droid" << " " << pos.x << " " << pos.y << " " << health << " " << damage << " " << 120 << " " << "Patrol" << " " << patrol.speed << " " << patrol.positions.size();

                for (int i = 0; i < patrol.positions.size(); i++)
                {
                    fin << " " << patrol.positions[i].x << " " << patrol.positions[i].y;
                }
                fin << "\n";
            }
        }
    }

    // Close the file
    fin.close();
}

void Scene_Builder::changeTile(int indexChange, std::shared_ptr<Entity> entity)
{
    m_selectedIndex += indexChange;
    if (m_selectedIndex < 0)
    {
        m_selectedIndex = m_animations.size() - 1;
    }
    else if (m_selectedIndex > m_animations.size() - 1)
    {
        m_selectedIndex = 0;
    }
    
    std::string name = m_animations[m_selectedIndex];
    auto pos = entity->getComponent<CTransform>().pos;
    entity->getComponent<CAnimation>().animation = m_game->assets().getAnimation(name);
    entity->getComponent<CBoundingBox>().size = m_game->assets().getAnimation(name).getSize();
}

void Scene_Builder::changeDec(int indexChange, std::shared_ptr<Entity> entity)
{
    m_selectedIndex += indexChange;
    if (m_selectedIndex < 0)
    {
        m_selectedIndex = m_decorations.size() - 1;
    }
    else if (m_selectedIndex > m_decorations.size() - 1)
    {
        m_selectedIndex = 0;
    }

    std::string name = m_decorations[m_selectedIndex];
    auto pos = entity->getComponent<CTransform>().pos;
    entity->getComponent<CAnimation>().animation = m_game->assets().getAnimation(name);
}

void Scene_Builder::changePickup(int indexChange, std::shared_ptr<Entity> entity)
{
    m_selectedIndex += indexChange;
    if (m_selectedIndex < 0)
    {
        m_selectedIndex = m_pickups.size() - 1;
    }
    else if (m_selectedIndex > m_pickups.size() - 1)
    {
        m_selectedIndex = 0;
    }

    std::string name = m_pickups[m_selectedIndex];
    auto pos = entity->getComponent<CTransform>().pos;
    entity->getComponent<CAnimation>().animation = m_game->assets().getAnimation(name);
    entity->addComponent<CBoundingBox>(m_game->assets().getAnimation(name).getSize() * 0.4f);
    entity->getComponent<CPickUp>().startPos = pos;
    entity->getComponent<CPickUp>().effect = m_selectedIndex;
}

void Scene_Builder::changeEnemy(int indexChange, std::shared_ptr<Entity> entity)
{
    m_selectedIndex += indexChange;
    if (m_selectedIndex < 0)
    {
        m_selectedIndex = m_enemies.size() - 1;
    }
    else if (m_selectedIndex > m_enemies.size() - 1)
    {
        m_selectedIndex = 0;
    }

    std::string name = m_enemies[m_selectedIndex];
    auto pos = entity->getComponent<CTransform>().pos;
    entity->getComponent<CAnimation>().animation = m_game->assets().getAnimation(name);
}

void Scene_Builder::SpawnPlayer()
{
    // Create player entity
    m_player = m_entityManager.addEntity("player");
    // Add components
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
    m_player->addComponent<CTransform>(gridToMidPixel(m_playerConfig.X, m_playerConfig.Y, m_player));
    m_player->getComponent<CTransform>().scale *= 3;
    m_player->addComponent<CBoundingBox>(Vec2(m_playerConfig.CX, m_playerConfig.CY) * 3);
    m_player->addComponent<CGravity>(m_playerConfig.GRAVITY);
    m_player->addComponent<CState>();
    m_player->addComponent<CInput>();
    m_player->addComponent<CAdvancedMovement>();
}

void Scene_Builder::moveCamera(int xMove)
{
    // set the viewport of the window to be centered on the player
    sf::View view = m_game->window().getView();
    auto& pPos = view.getCenter().x;
    view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    view.setCenter(pPos + xMove, height()/2);
    m_game->window().setView(view);
    m_screenOffset += xMove;
}

void Scene_Builder::makeMovingTile()
{
    if (m_positions.size() != 0)
    {
        m_currentlyDragging->addComponent<CPatrol>(m_positions, 2, false);
    }
}

void Scene_Builder::removeAI()
{
    if (m_currentlyDragging->hasComponent<CFollowPlayer>()) //remove follow if it is already there
    {
        m_currentlyDragging->removeComponent<CFollowPlayer>();
    }
    else if (m_currentlyDragging->hasComponent<CPatrol>()) //remove patrol if its there
    {
        m_currentlyDragging->removeComponent<CPatrol>();
    }
}

void Scene_Builder::addFollow() 
{
    removeAI();
    m_currentlyDragging->addComponent<CFollowPlayer>(m_currentlyDragging->getComponent<CTransform>().pos, 2);
}

void Scene_Builder::addPatrol()
{
    if (m_positions.size() != 0)
    {
        m_currentlyDragging->addComponent<CPatrol>(m_positions, 2, false);
    }
}

void Scene_Builder::update()
{
    m_entityManager.update();
    sRender();
}

void Scene_Builder::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "TILE")
        {
            if (!m_dragging && !m_addingDec && !m_addingEnemies && !m_addingPickup) //spawn the first tile at the mouse position
            {
                m_addingTile = true;
                m_selectedIndex = 0;
                auto brick = m_entityManager.addEntity("tile");
                brick->addComponent<CAnimation>(m_game->assets().getAnimation(m_animations[0]), true);
                brick->addComponent<CTransform>(Vec2(sf::Mouse::getPosition(m_game->window()).x + m_screenOffset, sf::Mouse::getPosition(m_game->window()).y));
                brick->addComponent<CBoundingBox>(m_game->assets().getAnimation(m_animations[0]).getSize());
                m_currentlyAdding = brick;
            }
        }
        else if (action.name() == "DEC")
        {
            if (!m_dragging && !m_addingTile && !m_addingEnemies && !m_addingPickup) //select the first dec
            {
                m_addingDec = true;
                m_selectedIndex = 0;
                auto dec = m_entityManager.addEntity("dec");
                dec->addComponent<CAnimation>(m_game->assets().getAnimation(m_decorations[0]), true);
                dec->addComponent<CTransform>(Vec2(sf::Mouse::getPosition(m_game->window()).x + m_screenOffset, sf::Mouse::getPosition(m_game->window()).y));
                m_currentlyAdding = dec;
            }
        }
        else if (action.name() == "ENEMY")
        {
            if (!m_dragging && !m_addingTile && !m_addingDec && !m_addingPickup) //select the first enemy
            {
                m_addingEnemies = true;
                m_selectedIndex = 0;
                auto enm = m_entityManager.addEntity("enemy");
                enm->addComponent<CAnimation>(m_game->assets().getAnimation(m_enemies[0]), true);
                enm->addComponent<CTransform>(Vec2(sf::Mouse::getPosition(m_game->window()).x + m_screenOffset, sf::Mouse::getPosition(m_game->window()).y));
                enm->getComponent<CTransform>().scale *= 3;
                enm->addComponent<CBoundingBox>(Vec2(50, 90));
                enm->addComponent<CHealth>(3, 3);
                enm->addComponent<CDamage>(3);
                m_currentlyAdding = enm;
            }
        }
        else if (action.name() == "PICKUP")
        {
            if (!m_dragging && !m_addingTile && !m_addingDec && !m_addingEnemies) //select the first enemy
            {
                m_addingPickup = true;
                m_selectedIndex = 0;
                auto pk = m_entityManager.addEntity("pickup");
                pk->addComponent<CAnimation>(m_game->assets().getAnimation(m_pickups[0]), true);
                pk->addComponent<CTransform>(Vec2(sf::Mouse::getPosition(m_game->window()).x + m_screenOffset, sf::Mouse::getPosition(m_game->window()).y));
                pk->getComponent<CTransform>().scale *= 0.4;
                pk->addComponent<CPickUp>(Vec2(sf::Mouse::getPosition(m_game->window()).x + m_screenOffset, sf::Mouse::getPosition(m_game->window()).y), 0);
                pk->addComponent<CBoundingBox>(m_game->assets().getAnimation(m_enemies[0]).getSize() * 0.4f);
                m_currentlyAdding = pk;
            }
        }
        else if (action.name() == "RIGHT")
        {
            if (m_addingTile)
            {
                changeTile(1, m_currentlyAdding);
            }
            else if (m_addingDec)
            {
                changeDec(1, m_currentlyAdding);
            }
            else if (m_addingEnemies)
            {
                changeEnemy(1, m_currentlyAdding);
            }
            else if (m_addingPickup)
            {
                changePickup(1, m_currentlyAdding);
            }
            else if (m_healthChange) //increment health
            {
                m_currentlyDragging->getComponent<CHealth>().max += 1;
                m_currentlyDragging->getComponent<CHealth>().current += 1;
            }
            else if (m_damageChange) //increment damage
            {
                m_currentlyDragging->getComponent<CDamage>().damage += 1;
            }
            else if (m_speedChange) //increment speed
            {
                if (m_currentlyDragging->hasComponent<CFollowPlayer>())
                {
                    m_currentlyDragging->getComponent<CFollowPlayer>().speed += 1;
                }
                else if (m_currentlyDragging->hasComponent<CPatrol>())
                {
                    m_currentlyDragging->getComponent<CPatrol>().speed += 1;
                }
            }
        }
        else if (action.name() == "LEFT")
        {
            if (m_addingTile)
            {
                changeTile(-1, m_currentlyAdding);
            }
            else if (m_addingDec)
            {
                changeDec(-1, m_currentlyAdding);
            }
            else if (m_addingEnemies)
            {
                changeEnemy(-1, m_currentlyAdding);
            }
            else if (m_addingPickup)
            {
                changePickup(-1, m_currentlyAdding);
            }
            else if (m_healthChange) //decrement health
            {
                m_currentlyDragging->getComponent<CHealth>().max -= 1;
                m_currentlyDragging->getComponent<CHealth>().current -= 1;
            }
            else if (m_damageChange) //decrement damage
            {
                m_currentlyDragging->getComponent<CDamage>().damage -= 1;
            }
            else if (m_speedChange) //decrement speed
            {
                if (m_currentlyDragging->hasComponent<CFollowPlayer>())
                {
                    m_currentlyDragging->getComponent<CFollowPlayer>().speed -= 1;
                }
                else if (m_currentlyDragging->hasComponent<CPatrol>())
                {
                    m_currentlyDragging->getComponent<CPatrol>().speed -= 1;
                }
            }
        }
        else if (action.name() == "ENTER")
        {
            if (m_addingTile)
            {
                m_addingTile = false;
                midPixelToGrid(m_currentlyAdding);
            }
            else if (m_addingDec)
            {
                m_addingDec = false;
                midPixelToGrid(m_currentlyAdding);
            }
            else if (m_addingEnemies)
            {
                m_addingEnemies = false;
                midPixelToGrid(m_currentlyAdding);
            }
            else if (m_addingPickup)
            {
                m_addingPickup = false;
                midPixelToGrid(m_currentlyAdding);
            }
            else if (m_makeMoving)
            {
                m_makeMoving = false;
                makeMovingTile();
            }
            else if (m_aiSelection && m_patrolSelection)
            {
                m_aiSelection = false;
                m_patrolSelection = false;
                addPatrol();
            }
            else if (m_healthChange)
            {
                m_healthChange = false;
            }
            else if (m_damageChange)
            {
                m_damageChange = false;
            }
            else if (m_speedChange)
            {
                m_speedChange = false;
            }
        }
        else if (action.name() == "FORWARD")
        {
            if (!m_addingTile && !m_addingDec && !m_addingPickup)
            {
                moveCamera(600);
            }
        }
        else if (action.name() == "BACKWARD")
        {
            if (!m_addingTile && !m_addingDec && !m_addingEnemies)
            {
                moveCamera(-600);
            }
        }
        else if (action.name() == "HEALTH")
        {
            if (m_dragging && (m_currentlyDragging->tag() == "Guard" || m_currentlyDragging->tag() == "GoomBot" || m_currentlyDragging->tag() == "Droid" || m_currentlyDragging->tag() == "enemy"))
            {
                m_healthChange = true;
            }
        }
        else if (action.name() == "DAMAGE")
        {
            if (m_dragging && (m_currentlyDragging->tag() == "Guard" || m_currentlyDragging->tag() == "GoomBot" || m_currentlyDragging->tag() == "Droid" || m_currentlyDragging->tag() == "enemy"))
            {
                m_damageChange = true;
            }
        }
        else if (action.name() == "SPEED")
        {
            if (m_dragging && (m_currentlyDragging->hasComponent<CFollowPlayer>() || m_currentlyDragging->hasComponent<CPatrol>()))
            {
                m_speedChange = true;
            }
        }
        else if (action.name() == "DELETE")
        {
            if (m_dragging && m_currentlyDragging->tag() != "player" && m_currentlyDragging->tag() != "goal") //don't delete the player or the goal
            {
                m_currentlyDragging->destroy();
                m_dragging = false;
            }
        }
        else if (action.name() == "FOLLOW")
        {
            if (m_aiSelection)
            {
                m_aiSelection = false;
                addFollow();
            }
        }
        else if (action.name() == "PATROL")
        {
            if (m_aiSelection)
            {
                m_patrolSelection = true;
                addPatrol();
            }
        }
        else if (action.name() == "SAVE")
        {
            save();
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}

void Scene_Builder::sDoMouseAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "LEFTCLICK")
        {
            if (m_makeMoving) //make selections for moving tiles
            {
                sf::Vector2i position = sf::Mouse::getPosition(m_game->window());
                int outX = (position.x + m_screenOffset) / m_gridSize.x;
                int outY = abs((position.y / m_gridSize.y) - 12);

                Vec2 positions = gridToMidPixel(outX, outY, nullptr);

                m_positions.push_back(Vec2(outX, outY));
            }
            else if (m_patrolSelection) //make selections for patrol
            {
                sf::Vector2i position = sf::Mouse::getPosition(m_game->window());
                int outX = (position.x + m_screenOffset) / m_gridSize.x;
                int outY = abs((position.y / m_gridSize.y) - 12);

                Vec2 positions = gridToMidPixel(outX, outY, nullptr);

                m_positions.push_back(Vec2(outX, outY));
            }
            else if (m_dragging && !m_healthChange && !m_damageChange) //drop the entity
            {
                midPixelToGrid(m_currentlyDragging);
                m_dragging = false;
            }
            else if(!m_addingTile && !m_addingDec && !m_addingEnemies && !m_aiSelection && !m_addingPickup) //drag an entity
            {
                sf::Vector2i position = sf::Mouse::getPosition(m_game->window());
                for (auto e : m_entityManager.getEntities())
                {
                    bool ent = isMouseInside(e, position);
                    if (ent)
                    {
                        m_currentlyDragging = e;
                        m_dragging = true;
                    }
                }
            }
        }
        else if (action.name() == "RIGHTCLICK")
        {
            if (!m_dragging) //access the settings of a entity that was right clicked
            {
                sf::Vector2i position = sf::Mouse::getPosition(m_game->window());
                for (auto e : m_entityManager.getEntities())
                {
                    bool ent = isMouseInside(e, position);
                    if (ent)
                    {
                        m_currentlyDragging = e;
                    }
                }

                if (m_currentlyDragging->tag() == "tile") //if its a tile make a moving platform
                {
                    m_positions = {};
                    m_makeMoving = true;
                }
                else if (m_currentlyDragging->tag() == "GoomBot" || m_currentlyDragging->tag() == "Droid" || m_currentlyDragging->tag() == "enemy" || m_currentlyDragging->tag() == "Guard") //if its an enemy select ai
                {
                    m_positions = {};
                    m_aiSelection = true;
                    
                }
            }
        }
    }
}

bool Scene_Builder::isMouseInside(std::shared_ptr<Entity> a, sf::Vector2i mouse)
{
    //determines if a click was inside of an entity
    int x = a->getComponent<CTransform>().pos.x;
    int y = a->getComponent<CTransform>().pos.y;
    int xSize = a->getComponent<CAnimation>().animation.getSize().x;
    int ySize = a->getComponent<CAnimation>().animation.getSize().y;
    if ((mouse.x + m_screenOffset >= x - (xSize/2)) && (mouse.x + m_screenOffset <= x + (xSize/2)) && (mouse.y >= y - (ySize/2)) && (mouse.y <= y + (ySize/2)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Scene_Builder::drawLine(const Vec2& p1, const Vec2& p2)
{
    sf::Vertex line[] = { sf::Vector2f(p1.x, p1.y), sf::Vector2f(p2.x, p2.y) };
    m_game->window().draw(line, 2, sf::Lines);
}

void Scene_Builder::sRender()
{
    m_game->window().clear(sf::Color::Color(41, 173, 255));

    if (m_dragging && !m_healthChange && !m_damageChange)
    {
        m_currentlyDragging->getComponent<CTransform>().pos.x = sf::Mouse::getPosition(m_game->window()).x + m_screenOffset;
        m_currentlyDragging->getComponent<CTransform>().pos.y = sf::Mouse::getPosition(m_game->window()).y;
    }

    for (auto& e : m_onScreenText)
    {
        m_game->window().draw(e);
    }

    
    for (auto e : m_entityManager.getEntities()) //draw the entities
    {
        auto& transform = e->getComponent<CTransform>();

        if (e->hasComponent<CAnimation>())
        {
            auto& animation = e->getComponent<CAnimation>().animation;
            animation.getSprite().setRotation(transform.angle);
            animation.getSprite().setPosition(transform.pos.x, transform.pos.y);
            animation.getSprite().setScale(transform.scale.x, transform.scale.y);
            m_game->window().draw(animation.getSprite());
        }
    }


    for (auto e : m_entityManager.getEntities()) //draw bounding boxes
    {
        if (e->hasComponent<CBoundingBox>())
        {
            auto& box = e->getComponent<CBoundingBox>();
            auto& transform = e->getComponent<CTransform>();
            sf::RectangleShape rect;
            rect.setSize(sf::Vector2f(box.size.x - 1, box.size.y - 1));
            rect.setOrigin(sf::Vector2f(box.halfSize.x, box.halfSize.y));
            rect.setPosition(transform.pos.x, transform.pos.y);
            rect.setFillColor(sf::Color(0, 0, 0, 0));
            rect.setOutlineColor(sf::Color(255, 255, 255, 255));
            rect.setOutlineThickness(1);
            m_game->window().draw(rect);
        }
    }
    

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
            //m_gridText.setString("(" + xCell + "," + yCell + ")");
            //m_gridText.setPosition(x + 3, height() - y - m_gridSize.y + 2);
            //m_game->window().draw(m_gridText);
        }
    }

    if (m_addingDec || m_addingEnemies || m_addingPickup || m_addingTile)
    {
        std::string text = "Cycle through animations:   Left A   Right D   Select Enter";
        m_noticeText.setString(text);
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(50 + m_screenOffset, 700);
        m_noticeText.setFillColor(sf::Color::White);
        m_game->window().draw(m_noticeText);
    }
    else if (m_patrolSelection) //patrol position notice
    {
        std::string text = "Left Click the squares you want the Enemy to Patrol then Press Enter";
        m_noticeText.setString(text);
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(50 + m_screenOffset, 700);
        m_noticeText.setFillColor(sf::Color::White);
        m_game->window().draw(m_noticeText);
    }
    else if (m_aiSelection) //tell player to choose ai
    {
        std::string text = "Press F for Follow or P for Patrol";
        m_noticeText.setString(text);
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(50 + m_screenOffset, 700);
        m_noticeText.setFillColor(sf::Color::White);
        m_game->window().draw(m_noticeText);
    }
    else if (m_makeMoving) //making a tile a moving tile notice
    {
        std::string text = "Left Click the squares you want the tile to move to then Press Enter";
        m_noticeText.setString(text);
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(50 + m_screenOffset, 700);
        m_noticeText.setFillColor(sf::Color::White);
        m_game->window().draw(m_noticeText);
    }
    else if (m_healthChange) //show the changing health
    {
        m_noticeText.setString("Health: " + std::to_string(m_currentlyDragging->getComponent<CHealth>().max));
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(540 + m_screenOffset, 200);
        m_noticeText.setFillColor(sf::Color::Red);
        m_game->window().draw(m_noticeText);
    }
    else if (m_damageChange) //show the changing damage
    {
        m_noticeText.setString("Damage: " + std::to_string(m_currentlyDragging->getComponent<CDamage>().damage));
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(600 + m_screenOffset, 200);
        m_noticeText.setFillColor(sf::Color::Red);
        m_game->window().draw(m_noticeText);
    }
    else if (m_speedChange) //show the changing speed
    {
        if (m_currentlyDragging->hasComponent<CFollowPlayer>())
        {
            m_noticeText.setString("Speed: " + std::to_string(m_currentlyDragging->getComponent<CFollowPlayer>().speed));
            m_noticeText.setCharacterSize(30);
            m_noticeText.setPosition(600 + m_screenOffset, 200);
            m_noticeText.setFillColor(sf::Color::Red);
            m_game->window().draw(m_noticeText);
        }
        else if (m_currentlyDragging->hasComponent<CPatrol>())
        {
            m_noticeText.setString("Speed: " + std::to_string(m_currentlyDragging->getComponent<CPatrol>().speed));
            m_noticeText.setCharacterSize(30);
            m_noticeText.setPosition(600 + m_screenOffset, 200);
            m_noticeText.setFillColor(sf::Color::Red);
            m_game->window().draw(m_noticeText);
        }
    }
    else if (m_dragging)
    {
        std::string text = "Delete Entity: Backspace  Set Health: H  Set Speed: J  Set Damage: G";
        m_noticeText.setString(text);
        m_noticeText.setCharacterSize(30);
        m_noticeText.setPosition(50 + m_screenOffset, 700);
        m_noticeText.setFillColor(sf::Color::White);
        m_game->window().draw(m_noticeText);
    }
    else
    {
        std::string text = "Grab Entity: Left Click   Set Enemy/Tile AI: Right Click   Add Enemy: E   Add Decoration: R   Add Tile: T   Add Pickup: Y";
        m_noticeText.setString(text);
        m_noticeText.setCharacterSize(20);
        m_noticeText.setPosition(25 + m_screenOffset, 700);
        m_noticeText.setFillColor(sf::Color::White);
        m_game->window().draw(m_noticeText);
    }
}

void Scene_Builder::onEnd()
{
    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", options::m_optionSettings[2]);
    m_game->changeScene("MENU", nullptr, true);
}