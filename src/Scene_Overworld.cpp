#include "Scene_Overworld.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "Scene_Play.h"
#include "Scene_Menu.h"

Scene_Overworld::Scene_Overworld(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}


void Scene_Overworld::init()
{
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Enter, "ENTER");
    registerAction(sf::Keyboard::G, "GODMODE");
    registerAction(sf::Keyboard::S, "SAVE");
    registerAction(sf::Keyboard::N, "NEW");

    m_locations.push_back(std::vector{ 470, 190 });
    m_locations.push_back(std::vector{ 470, 385 });
    m_locations.push_back(std::vector{ 672, 385 });
    m_locations.push_back(std::vector{ 672, 410 });
    m_locations.push_back(std::vector{ 807, 410 });
    m_locations.push_back(std::vector{ 807, 630 });

    // Open the specified level file
    std::ifstream fin("save.txt");
    int lineLabel;
    bool levelBeaten;

    // While there are still lines to read
    while (fin >> lineLabel) //read in the file to see the game progression
    {
        fin >> levelBeaten;
        options::m_beatenLevels[lineLabel] = levelBeaten;
    }

    // Close the file
    fin.close();

    m_levels.push_back(std::vector{ 470, 190 });
    m_levels.push_back(std::vector{ 470, 385 });
    m_levels.push_back(std::vector{ 807, 410 });
    m_levels.push_back(std::vector{ 807, 630 });

    m_paths.push_back("level1.txt");
    m_paths.push_back("level2.txt");
    m_paths.push_back("level3.txt");
    m_paths.push_back("bossBattle.txt");

    m_levelNames.push_back("Level 1");
    m_levelNames.push_back("Level 2");
    m_levelNames.push_back("Level 3");
    m_levelNames.push_back("Boss Battle");

    m_playerConfig.X = m_levels[0][0];
    m_playerConfig.Y = m_levels[0][1];
    m_playerConfig.SPEED = 3;

    m_currentLocation = 0;
    m_currentLevel = 0;
    m_godMode = false;

    // reset the entity manager every time we load a level
    m_entityManager = EntityManager();

    SpawnPlayer();
    m_movePlayerRight = false;
    m_movePlayerLeft = false;

    m_menuText.setFont(m_game->assets().getFont("Metal"));
}

void Scene_Overworld::save()
{
    // Open the specified level file
    std::ofstream fin("save.txt", std::ofstream::out | std::ofstream::trunc);
    
    for (int i = 0; i < options::m_beatenLevels.size(); i++)
    {
        fin << std::to_string(i) + " " + std::to_string(options::m_beatenLevels[i]) + "\n";
    }

    // Close the file
    fin.close();
}

void Scene_Overworld::newFile()
{
    // Open the specified level file
    std::ofstream fin("save.txt", std::ofstream::out | std::ofstream::trunc);

    for (int i = 0; i < options::m_beatenLevels.size(); i++)
    {
        fin << std::to_string(i) + " " + std::to_string(0) + "\n";
    }

    // Close the file
    fin.close();

    //update beaten levels
    for (int i = 0; i < options::m_beatenLevels.size(); i++)
    {
        options::m_beatenLevels[i] = false;
    }
}

void Scene_Overworld::drawDots()
{
    sf::CircleShape circle;
    for (int i = 0; i < m_levels.size(); i++) //draw a red circle at each level location
    {
        circle.setRadius(10);
        if (options::m_beatenLevels[i] == true) //draw beaten levels green, unbeaten red
        {
            circle.setFillColor(sf::Color::Green);
        }
        else
        {
            circle.setFillColor(sf::Color::Red);
        }
        circle.setPosition(m_levels[i][0] - 10, m_levels[i][1] - 7);
        m_game->window().draw(circle);
    }
}

void Scene_Overworld::SpawnPlayer()
{
    // Create player entity
    m_player = m_entityManager.addEntity("player");

    // Add components
    m_player->addComponent<CAnimation>(m_game->assets().getAnimation("Stand"), true);
    m_player->addComponent<CTransform>(Vec2(m_playerConfig.X, m_playerConfig.Y));
}

void Scene_Overworld::movePlayerRight()
{
    if (m_movePlayerRight && (m_currentLocation < m_locations.size() - 1)) //if not at the last position and the player should be moving
    {
        if (m_locations[m_currentLocation][0] < m_locations[m_currentLocation + 1][0]) //check if player needs to move in x direction
        {
            if (m_locations[m_currentLocation + 1] == m_levels[m_currentLevel + 1]) //check if player is arriving at level location
            {
                if (m_locations[m_currentLocation + 1][0] - m_player->getComponent<CTransform>().pos.x < 3) //if player is close enough to a location
                {
                    m_movePlayerRight = false;
                    m_currentLocation += 1;
                    m_currentLevel += 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.x += m_playerConfig.SPEED; //move player
                }
            }
            else
            {
                if (m_locations[m_currentLocation + 1][0] - m_player->getComponent<CTransform>().pos.x < 3) //if player is close enough to a location
                {
                    m_currentLocation += 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.x += m_playerConfig.SPEED; //move player
                }
            }
        }
        else if (m_locations[m_currentLocation][1] < m_locations[m_currentLocation + 1][1]) //check if player needs to move in y direction
        {
            if (m_locations[m_currentLocation + 1] == m_levels[m_currentLevel + 1])
            {
                if ((m_locations[m_currentLocation + 1][1] - m_player->getComponent<CTransform>().pos.y) < 3)
                {
                    m_movePlayerRight = false;
                    m_currentLocation += 1;
                    m_currentLevel += 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.y += m_playerConfig.SPEED;
                }
            }
            else
            {
                if ((m_locations[m_currentLocation + 1][1] - m_player->getComponent<CTransform>().pos.y) < 3)
                {
                    m_currentLocation += 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.y += m_playerConfig.SPEED;
                }
            }
        }
    }
}

void Scene_Overworld::movePlayerLeft()
{
    if (m_movePlayerLeft && (m_currentLocation > 0)) //if not at the first position and the player should be moving
    {
        if (m_locations[m_currentLocation][0] > m_locations[m_currentLocation - 1][0]) //check if player needs to move in x direction
        {
            if (m_locations[m_currentLocation - 1] == m_levels[m_currentLevel - 1]) //check if player is arriving at level location
            {
                if ((m_player->getComponent<CTransform>().pos.x - m_locations[m_currentLocation - 1][0]) < 3) //if player is close enough to a location
                {
                    m_movePlayerLeft = false;
                    m_currentLocation -= 1;
                    m_currentLevel -= 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.x -= m_playerConfig.SPEED; //move player
                }
            }
            else
            {
                if ((m_player->getComponent<CTransform>().pos.x - m_locations[m_currentLocation - 1][0]) < 3) //if player is close enough to a location
                {
                    m_currentLocation -= 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.x -= m_playerConfig.SPEED; //move player
                }
            }
        }
        else if (m_locations[m_currentLocation][1] > m_locations[m_currentLocation - 1][1]) //check if player needs to move in y direction
        {
            if (m_locations[m_currentLocation - 1] == m_levels[m_currentLevel - 1])
            {
                if ((m_player->getComponent<CTransform>().pos.y - m_locations[m_currentLocation - 1][1]) < 3)
                {
                    m_movePlayerLeft = false;
                    m_currentLocation -= 1;
                    m_currentLevel -= 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.y -= m_playerConfig.SPEED;
                }
            }
            else
            {
                if ((m_player->getComponent<CTransform>().pos.y - m_locations[m_currentLocation - 1][1]) < 3)
                {
                    m_currentLocation -= 1;
                }
                else
                {
                    m_player->getComponent<CTransform>().pos.y -= m_playerConfig.SPEED;
                }
            }
        }
    }
}

void Scene_Overworld::update()
{
    m_entityManager.update();
    m_currentFrame++;
    sRender();
    movePlayerRight();
    movePlayerLeft();
    
}

void Scene_Overworld::sDoMouseAction(const Action& action)
{

}

void Scene_Overworld::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (action.name() == "RIGHT")
        {
            if (!m_movePlayerLeft && (m_currentLevel < m_levels.size() - 1)) //make sure player isn't moving left and isn't in the last position
            {
                if (options::m_beatenLevels[m_currentLevel] == true || m_godMode) //if the level has been beaten or godmode is activated the player can move right
                {
                    m_movePlayerRight = true;
                    m_player->getComponent<CTransform>().scale = Vec2(1, 1);
                }
            }
        }
        else if (action.name() == "LEFT")
        {
            if (!m_movePlayerRight && (m_currentLevel > 0)) //make sure player isn't moving right and isn't in the first position
            {
                m_movePlayerLeft = true;
                m_player->getComponent<CTransform>().scale = Vec2(-1, 1);
            }
        }
        else if (action.name() == "ENTER")
        {
            if (!m_movePlayerLeft && !m_movePlayerRight) //make sure the player isn't moving
            {
                m_game->stopSound("MainMenu");
                m_game->changeScene("PLAY", std::make_shared<Scene_Play>(m_game, m_paths[m_currentLevel]));
            }
        }
        else if (action.name() == "GODMODE")
        {
            m_godMode = !m_godMode; //set godmode to allow the player to move anywhere

            //reset to first level position
            m_currentLevel = 0;
            m_currentLocation = 0;
            m_player->getComponent<CTransform>().pos.x = m_levels[m_currentLevel][0];
            m_player->getComponent<CTransform>().pos.y = m_levels[m_currentLevel][1];
        }
        else if (action.name() == "SAVE")
        {
            save();
        }
        else if (action.name() == "NEW")
        {
            newFile();
            m_currentLocation = 0;
            m_currentLevel = 0;
            m_player->getComponent<CTransform>().pos.x = m_levels[0][0];
            m_player->getComponent<CTransform>().pos.y= m_levels[0][1];
        }
        else if (action.name() == "QUIT")
        {
            onEnd();
        }
    }
}

void Scene_Overworld::onEnd()
{
    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", options::m_optionSettings[2]);
    m_game->changeScene("MENU", nullptr, true);
}

void Scene_Overworld::sRender()
{
    m_game->window().clear(sf::Color::Color(41, 173, 255));

    //draw the backround
    sf::Texture background;
    background.loadFromFile("images/misc/overWorld720.png");
    sf::Sprite sprite;
    sprite.setTexture(background);
    sprite.setOrigin(-300, -20);
    m_game->window().draw(sprite);

    drawDots();

    // set the viewport of the window to be centered on the player
    auto& pPos = m_player->getComponent<CTransform>().pos;
    sf::View view = m_game->window().getView();
    view.setSize(sf::Vector2f(960, 540));
    float idk = pPos.x;
    view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));
    view.setCenter(pPos.x, pPos.y);
    m_game->window().setView(view);

    for (auto e : m_entityManager.getEntities()) //draw the player
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

    if (!m_movePlayerLeft && !m_movePlayerRight) //draw the select level message to the screen
    {
        m_menuText.setCharacterSize(32);
        m_menuText.setFillColor(sf::Color::Yellow);
        m_menuText.setString(m_levelNames[m_currentLevel] + " Press ENTER to Play");
        m_menuText.setPosition(sf::Vector2f(m_player->getComponent<CTransform>().pos.x - 250, m_player->getComponent<CTransform>().pos.y));
        m_game->window().draw(m_menuText);
    }

    //draw controls
    m_menuText.setCharacterSize(22);
    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setString("Forward: D     Backward: A     Save: S     New Game: N     Godmode: G     Play: Enter");
    m_menuText.setPosition(sf::Vector2f(m_player->getComponent<CTransform>().pos.x - 450, m_player->getComponent<CTransform>().pos.y + 200));
    m_game->window().draw(m_menuText);  
}