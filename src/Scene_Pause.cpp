#include "Scene_Pause.h"
#include "Common.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"
#include "Scene_Play.h"
#include "Scene_Menu.h"


Scene_Pause::Scene_Pause(GameEngine* gameEngine, std::vector<int> arrows, sf::Texture background)
    : Scene(gameEngine)
{
    init();
    m_arrows = arrows;
    m_background = background;
}

void Scene_Pause::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::Enter, "ENTER");
    registerAction(sf::Keyboard::Space, "SPACE");

    m_mainMenu = true;
    m_optionsMenu = false;
    m_inventoryMenu = false;
    m_menuStrings.push_back("Resume");
    m_menuStrings.push_back("Inventory");
    m_menuStrings.push_back("Options");
    m_menuStrings.push_back("Exit");   

    m_optionStrings.push_back("Difficulty");
    m_optionStrings.push_back("SFX Volume");
    m_optionStrings.push_back("Music Volume");
    m_optionStrings.push_back("Go Back");

    m_inventoryStrings.push_back("Buster Arrow");
    m_inventoryStrings.push_back("Brick Arrow");
    m_inventoryStrings.push_back("Homing Arrow");
    m_inventoryStrings.push_back("Healing Arrow");
    m_inventoryStrings.push_back("Go Back");

    m_menuText.setFont(m_game->assets().getFont("Metal"));
    m_menuText.setCharacterSize(64);
    m_selectedMenuIndex = 0;

    m_backgroundShader.loadFromFile("shaders/pixelate.frag", sf::Shader::Fragment);

    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", options::m_optionSettings[2]);
}

void Scene_Pause::playMenuSound()
{
    m_game->playSound("menuBlip");
    m_game->volSound("menuBlip", options::m_optionSettings[1]);
}

void Scene_Pause::playSelectSound()
{
    m_game->playSound("startSound");
    m_game->volSound("startSound",options::m_optionSettings[1]);
}

void Scene_Pause::upKeystroke(std::vector<std::string> arr)
{
    if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
    else { m_selectedMenuIndex = arr.size() - 1; }
}

void Scene_Pause::downKeystroke(std::vector<std::string> arr)
{
    m_selectedMenuIndex = (m_selectedMenuIndex + 1) % arr.size();
}

void Scene_Pause::renderBackground()
{
    //m_backgroundShader.setUniform("blur_radius", 3.0f);
    m_backgroundShader.setUniform("pixel_threshold", 0.008f);
    sf::Sprite sprite;
    sprite.setTexture(m_background);
    sprite.setOrigin(0, 0);
    m_game->window().draw(sprite, &m_backgroundShader);
}

void Scene_Pause::renderMenu()
{
    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();
    
    renderBackground();

    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setCharacterSize(58);
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

        //center text
        sf::FloatRect textRect = m_menuText.getLocalBounds();
        m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 110 + i * 144));//sf::Vector2f(540, 110 + i * 144)
        m_game->window().draw(m_menuText);
    }
}

void Scene_Pause::renderOptions()
{
    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    renderBackground();

    // draw all of the menu options
    for (size_t i = 0; i < m_optionStrings.size(); i++)
    {
        m_menuText.setCharacterSize(58);
        m_menuText.setString(m_optionStrings[i]);

        if (m_optionStrings[i] == "Go Back") //only highlight the text for rebind keys and go back
        {
            m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);
        }
        else
        {
            m_menuText.setFillColor(sf::Color::White);
        }

        sf::FloatRect textRect = m_menuText.getLocalBounds();


        //center text
        m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 110 + i * 144));
        m_game->window().draw(m_menuText);
        


        if (m_optionStrings[i] != "Go Back")
        {
            m_menuText.setString(std::to_string(options::m_optionSettings[i]));
            m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

            //center text
            textRect = m_menuText.getLocalBounds();
            m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 175 + i * 144));
            m_game->window().draw(m_menuText);

            //right triangle
            sf::CircleShape triangle(15, 3);
            triangle.rotate(90);
            triangle.setPosition(750, 155 + i * 144);
            m_game->window().draw(triangle);

            //left triangle
            sf::CircleShape triangle1(15, 3);
            triangle1.rotate(-90);
            triangle1.setPosition(530, 185 + i * 144);
            m_game->window().draw(triangle1);
        }
    }
}

void Scene_Pause::renderInventory()
{
    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    renderBackground();

    // draw all of the menu options
    for (size_t i = 0; i < m_inventoryStrings.size(); i++)
    {
        if (m_inventoryStrings[i] == "Go Back")
        {
            m_menuText.setCharacterSize(58);
            m_menuText.setString(m_inventoryStrings[i]);
        }
        else if (m_arrows[i] < 0)
        {
            m_menuText.setCharacterSize(58);
            m_menuText.setString(m_inventoryStrings[i] + " xInfinity");
        }
        else 
        {
            m_menuText.setCharacterSize(58);
            m_menuText.setString(m_inventoryStrings[i] + " x" + std::to_string(m_arrows[i]));
        }

        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

        sf::FloatRect textRect = m_menuText.getLocalBounds();

        //center text
        m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 110 + i * 144));
        m_game->window().draw(m_menuText);
    }
}

void Scene_Pause::update()
{
    sRender();
}

void Scene_Pause::mainMenuActions(const Action& action)
{
    if (action.name() == "UP")
    {
        upKeystroke(m_menuStrings);
        playMenuSound();
    }
    else if (action.name() == "DOWN")
    {
        downKeystroke(m_menuStrings);
        playMenuSound();
    }
    else if (action.name() == "QUIT")
    {
        onEnd();
    }
    else if (action.name() == "ENTER")
    {
        playSelectSound();
        if (m_menuStrings[m_selectedMenuIndex] == "Options")
        {
            m_mainMenu = false;
            m_optionsMenu = true;
            m_selectedMenuIndex = 0;
        }
        else if (m_menuStrings[m_selectedMenuIndex] == "Inventory")
        {
            m_mainMenu = false;
            m_inventoryMenu = true;
            m_selectedMenuIndex = 0;
        }
        else if (m_menuStrings[m_selectedMenuIndex] == "Resume")
        {
            m_game->changeScene("PLAY", nullptr, true);
            m_game->stopSound("MainMenu");
            m_game->playSound("Play");
            m_game->volSound("Play", options::m_optionSettings[2]);
        }
        else if (m_menuStrings[m_selectedMenuIndex] == "Exit")
        {
            onEnd();
        }
    } 
}

void Scene_Pause::optionMenuActions(const Action& action)
{
    if (action.name() == "ENTER")
    {
        playSelectSound();
        if (m_selectedMenuIndex == 3) //GO BACK
        {
            m_optionsMenu = false;
            m_selectedMenuIndex = 0;
            m_mainMenu = true;
        }
    }
    else if (action.name() == "UP")
    {
        upKeystroke(m_optionStrings);
        playMenuSound();
    }
    else if (action.name() == "DOWN")
    {
        downKeystroke(m_optionStrings);
        playMenuSound();
    }
    else if (action.name() == "RIGHT")
    {
        if (m_selectedMenuIndex == 0) //difficulty
        {
            if (options::m_optionSettings[m_selectedMenuIndex] < 3) //cap it for the 3 difficulties
            {
                options::m_optionSettings[m_selectedMenuIndex] += 1;
                playMenuSound();
            }
        }
        else if (m_selectedMenuIndex == 1) //sfx volume
        {
            if (options::m_optionSettings[m_selectedMenuIndex] < 100) //cap it for volume 0 to 100
            {
                options::m_optionSettings[m_selectedMenuIndex] += 1;
                playMenuSound();
            }
        }
        else if (m_selectedMenuIndex == 2) //main music volume
        {
            if (options::m_optionSettings[m_selectedMenuIndex] < 100) //cap it for volume 0 to 100
            {
                m_game->volSound("MainMenu", options::m_optionSettings[m_selectedMenuIndex]);
                options::m_optionSettings[m_selectedMenuIndex] += 1;
                playMenuSound();
            }
        }
    }
    else if (action.name() == "LEFT")
    {
        if (m_selectedMenuIndex == 0) //difficulty
        {
            if (options::m_optionSettings[m_selectedMenuIndex] > 1) //cap it for the 3 difficulties
            {
                options::m_optionSettings[m_selectedMenuIndex] -= 1;
                playMenuSound();
            }
        }
        else if (m_selectedMenuIndex == 1) //sfx volume
        {
            if (options::m_optionSettings[m_selectedMenuIndex] > 0) //cap it for volume 0 to 100
            {
                options::m_optionSettings[m_selectedMenuIndex] -= 1;
                playMenuSound();
            }

        }
        else if (m_selectedMenuIndex == 2) //main music volume
        {
            if (options::m_optionSettings[m_selectedMenuIndex] > 0) //cap it for volume 0 to 100
            {
                m_game->volSound("MainMenu", options::m_optionSettings[m_selectedMenuIndex]);
                options::m_optionSettings[m_selectedMenuIndex] -= 1;
                playMenuSound();
            }
        }
    }
}

void Scene_Pause::inventoryMenuActions(const Action& action)
{
    if (action.name() == "UP")
    {
        upKeystroke(m_inventoryStrings);
        playMenuSound();
    }
    else if (action.name() == "DOWN")
    {
        downKeystroke(m_inventoryStrings);
        playMenuSound();
    }
    else if (action.name() == "QUIT")
    {
        onEnd();
    }
    else if (action.name() == "ENTER")
    {
        playSelectSound();
        if (m_inventoryStrings[m_selectedMenuIndex] == "Go Back")
        {
            m_inventoryMenu = false;
            m_mainMenu = true;
            m_selectedMenuIndex = 0;
        }
    }
}

void Scene_Pause::sDoMouseAction(const Action& action)
{

}

void Scene_Pause::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (m_mainMenu)
        {
            mainMenuActions(action);
        }
        else if (m_optionsMenu)
        {
            optionMenuActions(action);
        }
        else if (m_inventoryMenu)
        {
            inventoryMenuActions(action);
        }
    }
}

void Scene_Pause::onEnd()
{
    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", options::m_optionSettings[2]);
    m_game->changeScene("OVERWORLD", nullptr, true);
}

void Scene_Pause::sRender()
{
    if (m_mainMenu) {
        renderMenu();
    }
    else if (m_optionsMenu)
    {
        renderOptions();
    }
    else if (m_inventoryMenu)
    {
        renderInventory();
    }
    
} 