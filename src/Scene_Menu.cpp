#include "Scene_Menu.h"
#include "Scene_Play.h"
#include "Scene_Overworld.h"
#include "Scene_Builder.h"
#include "Common.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

Scene_Menu::Scene_Menu(GameEngine* gameEngine)
    : Scene(gameEngine)
{
    init();
}

namespace options
{
    std::vector<int>            m_optionSettings;
    std::vector<sf::Keyboard::Key>   m_rebindKeys;
    std::vector<bool>            m_beatenLevels = { false,false,false,false };
}

void Scene_Menu::init()
{
    registerAction(sf::Keyboard::W, "UP");
    registerAction(sf::Keyboard::A, "LEFT");
    registerAction(sf::Keyboard::S, "DOWN");
    registerAction(sf::Keyboard::D, "RIGHT");
    registerAction(sf::Keyboard::Escape, "QUIT");
    registerAction(sf::Keyboard::Enter, "ENTER");

    m_startMenu = true;
    m_optionsMenu = false;
    m_rebindMenu = false;
    m_editMenu = false;
    m_rebind = false;

    //set options
    m_difficulty = 2;
    m_musicVol = 20;
    m_sfxVol = 50;
    m_lastFrame = 0;
    m_frame = 0; //count frames

    options::m_rebindKeys.push_back(sf::Keyboard::A);
    options::m_rebindKeys.push_back(sf::Keyboard::D);
    options::m_rebindKeys.push_back(sf::Keyboard::W);
    options::m_rebindKeys.push_back(sf::Keyboard::S);
    options::m_rebindKeys.push_back(sf::Keyboard::Space);
    options::m_rebindKeys.push_back(sf::Keyboard::E);

    m_title = "Definitely Not a Platformer";
    m_menuStrings.push_back("Play");
    m_menuStrings.push_back("Level Builder");
    m_menuStrings.push_back("Options");
    m_menuStrings.push_back("Exit");

    m_optionMenuStrings.push_back("Game Difficulty");
    m_optionMenuStrings.push_back("SFX Volume");
    m_optionMenuStrings.push_back("Music Volume");
    m_optionMenuStrings.push_back("Rebind Keys");
    m_optionMenuStrings.push_back("Go Back");

    m_rebindMenuStrings.push_back("Left");
    m_rebindMenuStrings.push_back("Right");
    m_rebindMenuStrings.push_back("Jump");
    m_rebindMenuStrings.push_back("Slide");
    m_rebindMenuStrings.push_back("Shoot");
    m_rebindMenuStrings.push_back("Switch Weapons");
    m_rebindMenuStrings.push_back("Go Back");

    m_editMenuStrings.push_back("level1.txt");
    m_editMenuStrings.push_back("level2.txt");
    m_editMenuStrings.push_back("level3.txt");
    m_editMenuStrings.push_back("Go Back");

    options::m_optionSettings.push_back(m_difficulty);
    options::m_optionSettings.push_back(m_sfxVol);
    options::m_optionSettings.push_back(m_musicVol);

    m_menuText.setFont(m_game->assets().getFont("Metal"));
    m_menuText.setCharacterSize(64);

    m_game->playSound("MainMenu");
    m_game->volSound("MainMenu", m_musicVol);
}

void Scene_Menu::updateOptions()
{
    options::m_optionSettings[0] = m_difficulty;
    options::m_optionSettings[1] = m_sfxVol;
    options::m_optionSettings[2] = m_musicVol;
}

void Scene_Menu::passKey(const sf::Keyboard::Key& k) //get the key pressed
{
    m_rebindKey = k;
    bool add = true;
    if (m_rebind) //update the keys
    {
        for (int i = 0; i < options::m_rebindKeys.size(); i++)
        {
            if (options::m_rebindKeys[i] == m_rebindKey)
            {
                add = false;
            }
        }

        if (add) //only add a key is its not already bound
        {
            options::m_rebindKeys[m_selectedMenuIndex] = m_rebindKey;
        }

        m_rebind = false;
    }
}


void Scene_Menu::update()
{
    sRender();
    m_entityManager.update();
}

//used to determine if text should still be displayed (used for blinking text)
bool Scene_Menu::shouldContinue()
{
    bool cont;
    if ((m_frame - m_lastFrame) < 15) //display the frame for 15 frames before every blink
    {
        cont = true;
    }
    else
    {
        cont = false;
    }

    return cont;
}

void Scene_Menu::playMenuSound()
{
    m_game->playSound("menuBlip");
    m_game->volSound("menuBlip", m_sfxVol);
}

void Scene_Menu::playSelectSound()
{
    m_game->playSound("startSound");
    m_game->volSound("startSound", m_sfxVol);
}

void Scene_Menu::upKeystroke(std::vector<std::string> arr)
{
    if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
    else { m_selectedMenuIndex = arr.size() - 1; }
}

void Scene_Menu::downKeystroke(std::vector<std::string> arr)
{
    m_selectedMenuIndex = (m_selectedMenuIndex + 1) % arr.size();
}

void Scene_Menu::startMenuActions(const Action& action)
{
    if (action.name() == "ENTER")
    {
        playSelectSound();
        m_startMenu = false;
    }
}

void Scene_Menu::mainMenuActions(const Action& action)
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
            m_optionsMenu = true;
            m_selectedMenuIndex = 0;
        }
        else if (m_menuStrings[m_selectedMenuIndex] == "Play")
        {
            updateOptions();
            m_game->changeScene("OVERWORLD", std::make_shared<Scene_Overworld>(m_game));
        }
        else if (m_menuStrings[m_selectedMenuIndex] == "Level Builder")
        {
            m_editMenu = true;
            m_selectedMenuIndex = 0;
        }
        else if (m_menuStrings[m_selectedMenuIndex] == "Exit")
        {
            onEnd();
        }
    }
}

void Scene_Menu::editMenuActions(const Action& action)
{
    if (action.name() == "UP")
    {
        upKeystroke(m_editMenuStrings);
        playMenuSound();
    }
    else if (action.name() == "DOWN")
    {
        downKeystroke(m_editMenuStrings);
        playMenuSound();
    }
    else if (action.name() == "QUIT")
    {
        onEnd();
    }
    else if (action.name() == "ENTER")
    {
        playSelectSound();
        if (m_editMenuStrings[m_selectedMenuIndex] == "Go Back")
        {
            m_editMenu = false;
            m_selectedMenuIndex = 0;
        }
        else
        {
            updateOptions();
            m_game->changeScene("BUILDER", std::make_shared<Scene_Builder>(m_game,m_editMenuStrings[m_selectedMenuIndex]), false);
        }
    }
}

void Scene_Menu::optionMenuActions(const Action& action)
{
    if (action.name() == "ENTER")
    {
        playSelectSound();
        if (m_selectedMenuIndex == 4) //GO BACK
        {
            m_optionsMenu = false;
            m_selectedMenuIndex = 0;
        }
        else if (m_selectedMenuIndex == 3) //Rebind Keys
        {
            m_rebindMenu = true;
            m_optionsMenu = false;
            m_selectedMenuIndex = 0;
        }
    }
    else if (action.name() == "UP")
    {
        upKeystroke(m_optionMenuStrings);
        playMenuSound();
    }
    else if (action.name() == "DOWN")
    {
        downKeystroke(m_optionMenuStrings);
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

void Scene_Menu::rebindMenuActions(const Action& action)
{
    if (m_rebind != true)
    {
        if (action.name() == "UP")
        {
            upKeystroke(m_rebindMenuStrings);
            playMenuSound();
        }
        else if (action.name() == "DOWN")
        {
            downKeystroke(m_rebindMenuStrings);
            playMenuSound();
        }
        else if (action.name() == "ENTER")
        {
            playSelectSound();
            if (m_rebindMenuStrings[m_selectedMenuIndex] == "Go Back") //go back to the options
            {
                m_rebindMenu = false;
                m_optionsMenu = true;
                m_selectedMenuIndex = 0;
            }
            else if (m_rebind == false)
            {
                m_rebind = true;
            }
        }
    }
}

void Scene_Menu::sDoAction(const Action& action)
{
    if (action.type() == "START")
    {
        if (!m_startMenu && !m_optionsMenu && !m_rebindMenu && !m_editMenu) //this is the main menu
        {
            mainMenuActions(action);
        }
        else if (m_startMenu)
        {
            startMenuActions(action);
        }
        else if (m_optionsMenu)
        {
            optionMenuActions(action);
        }
        else if (m_editMenu)
        {
            editMenuActions(action);
        }
        else if (m_rebindMenu)
        {
            rebindMenuActions(action);
        }
    }
}

void Scene_Menu::setBackground()
{
    //draw the foreground
    sf::Texture background2;
    background2.loadFromFile("images/misc/CityForeground.png");
    sf::Sprite sprite2;
    sprite2.setTexture(background2);
    sprite2.setOrigin(0, 0);
    m_game->window().draw(sprite2);
}

void Scene_Menu::displayControls()
{
    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(30);
    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setString("up  w     down  s    select  enter");
    m_menuText.setPosition(sf::Vector2f(70, 720));
    m_game->window().draw(m_menuText);
}

void Scene_Menu::displayMain()
{
    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    setBackground();

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
    displayControls();
}

void Scene_Menu::displayEdit()
{
    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    setBackground();

    // draw all of the menu options
    for (size_t i = 0; i < m_editMenuStrings.size(); i++)
    {
        m_menuText.setCharacterSize(58);
        m_menuText.setString(m_editMenuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

        //center text
        sf::FloatRect textRect = m_menuText.getLocalBounds();
        m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 110 + i * 144));//sf::Vector2f(540, 110 + i * 144)
        m_game->window().draw(m_menuText);
    }
}

void Scene_Menu::displayStart()
{
    bool cont = shouldContinue(); //bool that says if the text should be displayed even is time parameter has not been met

    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    setBackground();

    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(64);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::White); //Color(67,70,75)
    m_menuText.setPosition(sf::Vector2f(150, 180));
    m_game->window().draw(m_menuText);

    if ((m_frame - m_lastFrame) > 30) //determine the portion of time that has passed to decide if you should display
    {
        m_menuText.setCharacterSize(38);
        m_menuText.setFillColor(sf::Color::Red);
        m_menuText.setString("PRESS ENTER TO CONTINUE");
        m_menuText.setPosition(sf::Vector2f(400, 500));
        m_game->window().draw(m_menuText);
        m_lastFrame = m_frame;
    }
    else if (cont) //makes sure the text doesn't rapidly appear and disappear
    {
        m_menuText.setCharacterSize(38);
        m_menuText.setFillColor(sf::Color::Red);
        m_menuText.setString("PRESS ENTER TO CONTINUE");
        m_menuText.setPosition(sf::Vector2f(400, 500));
        m_game->window().draw(m_menuText);
    }
}

void Scene_Menu::displayOptions()
{
    // clear the window
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    setBackground();

    // draw all of the menu options
    for (size_t i = 0; i < m_optionMenuStrings.size(); i++)
    {
        m_menuText.setCharacterSize(58);
        m_menuText.setString(m_optionMenuStrings[i]);

        if (m_optionMenuStrings[i] == "Rebind Keys" || m_optionMenuStrings[i] == "Go Back") //only highlight the text for rebind keys and go back
        {
            m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);
        }
        else
        {
            m_menuText.setFillColor(sf::Color::White);
        }

        sf::FloatRect textRect = m_menuText.getLocalBounds();
        if (m_optionMenuStrings[i] == "Go Back")
        {
            //center text
            m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 50 + i * 144));
            m_game->window().draw(m_menuText);
        }
        else
        {
            //center text
            m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 110 + i * 144));
            m_game->window().draw(m_menuText);
        }


        if ((m_optionMenuStrings[i] != "Rebind Keys") && (m_optionMenuStrings[i] != "Go Back"))
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

        //update values
        m_difficulty = options::m_optionSettings[0];
        m_sfxVol = options::m_optionSettings[1];
        m_musicVol = options::m_optionSettings[2];
    }
}


void Scene_Menu::displayRebind()
{
    bool cont = shouldContinue(); //bool that says if the text should be displayed even is time parameter has not been met

    // clear the window to a blue
    m_game->window().setView(m_game->window().getDefaultView());
    m_game->window().clear();

    setBackground();

    sf::FloatRect textRect = m_menuText.getLocalBounds();
    // draw all of the menu options
    for (size_t i = 0; i < m_rebindMenuStrings.size(); i++)
    {
        if (m_rebindMenuStrings[i] == "Go Back") //only hightlight go back text
        {
            m_menuText.setCharacterSize(45);
            m_menuText.setString(m_rebindMenuStrings[i]);
            m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);
        }
        else
        {
            m_menuText.setCharacterSize(45);
            m_menuText.setString(m_rebindMenuStrings[i]);
            m_menuText.setFillColor(sf::Color::White);
        }

        //center text
        textRect = m_menuText.getLocalBounds();
        m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 100 + i * 100));//sf::Vector2f(540, 175 + i * 144)
        m_game->window().draw(m_menuText);

        if (m_rebindMenuStrings[i] != "Go Back")
        {
            if (m_rebind) //if you are currently rebinding keys
            {
                if ((m_frame - m_lastFrame) > 30) //determine if text should flash
                {
                    m_menuText.setString(keyString(options::m_rebindKeys[i]));
                    m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

                    //center text
                    textRect = m_menuText.getLocalBounds();
                    m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                    m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 150 + i * 100));
                    m_game->window().draw(m_menuText);
                    m_lastFrame = m_frame;
                }
                else if (cont) //continues displaying for flashing text
                {
                    m_menuText.setString(keyString(options::m_rebindKeys[i]));
                    m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

                    //center text
                    textRect = m_menuText.getLocalBounds();
                    m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                    m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 150 + i * 100));
                    m_game->window().draw(m_menuText);
                }
                else if (i != m_selectedMenuIndex) //make sure only the selected text is not displayed
                {
                    m_menuText.setString(keyString(options::m_rebindKeys[i]));
                    m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

                    //center text
                    textRect = m_menuText.getLocalBounds();
                    m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                    m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 150 + i * 100));
                    m_game->window().draw(m_menuText);
                }
            }
            else
            {
                m_menuText.setString(keyString(options::m_rebindKeys[i]));
                m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Red : sf::Color::White);

                //center text
                textRect = m_menuText.getLocalBounds();
                m_menuText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
                m_menuText.setPosition(sf::Vector2f(1280 / 2.0f, 150 + i * 100));
                m_game->window().draw(m_menuText);
            }
        }
    }
}

void Scene_Menu::sDoMouseAction(const Action& action){}

void Scene_Menu::sRender()
{
    m_frame += 1; //increment frame

    if (m_startMenu) //drawing the start menu
    {
        displayStart();
    }
    else if (m_optionsMenu) //drawing the options menu
    {
        displayOptions();
    }
    else if (m_editMenu)
    {
        displayEdit();
    }
    else if (m_rebindMenu) //drawing the rebind menu
    {
        displayRebind();
    }
    else //display the main menu
    {
        displayMain();
    }
}

void Scene_Menu::onEnd()
{
    m_hasEnded = true;
    m_game->quit();
}

// Source: https://en.sfml-dev.org/forums/index.php?topic=15226.0
std::string Scene_Menu::keyString(const sf::Keyboard::Key& k) {
    std::string ret;
    switch (k) {

    case sf::Keyboard::A:

        ret = "A";
        break;
    case sf::Keyboard::B:

        ret = "B";
        break;
    case sf::Keyboard::C:

        ret = "C";
        break;
    case sf::Keyboard::D:

        ret = "D";
        break;
    case sf::Keyboard::E:

        ret = "E";
        break;
    case sf::Keyboard::F:

        ret = "F";
        break;
    case sf::Keyboard::G:

        ret = "G";
        break;
    case sf::Keyboard::H:

        ret = "H";
        break;
    case sf::Keyboard::I:

        ret = "I";
        break;
    case sf::Keyboard::J:

        ret = "J";
        break;
    case sf::Keyboard::K:

        ret = "K";
        break;
    case sf::Keyboard::L:

        ret = "L";
        break;
    case sf::Keyboard::M:

        ret = "M";
        break;
    case sf::Keyboard::N:

        ret = "N";
        break;
    case sf::Keyboard::O:

        ret = "O";
        break;
    case sf::Keyboard::P:

        ret = "P";
        break;
    case sf::Keyboard::Q:

        ret = "Q";
        break;
    case sf::Keyboard::R:

        ret = "R";
        break;
    case sf::Keyboard::S:

        ret = "S";
        break;
    case sf::Keyboard::T:

        ret = "T";
        break;
    case sf::Keyboard::U:

        ret = "U";
        break;
    case sf::Keyboard::V:

        ret = "V";
        break;
    case sf::Keyboard::W:

        ret = "W";
        break;
    case sf::Keyboard::X:

        ret = "X";
        break;
    case sf::Keyboard::Y:

        ret = "Y";
        break;
    case sf::Keyboard::Z:

        ret = "Z";
        break;
    case sf::Keyboard::Num0:

        ret = "Num0";
        break;
    case sf::Keyboard::Num1:

        ret = "Num1";
        break;
    case sf::Keyboard::Num2:

        ret = "Num2";
        break;
    case sf::Keyboard::Num3:

        ret = "Num3";
        break;
    case sf::Keyboard::Num4:

        ret = "Num4";
        break;
    case sf::Keyboard::Num5:

        ret = "Num5";
        break;
    case sf::Keyboard::Num6:

        ret = "Num6";
        break;
    case sf::Keyboard::Num7:

        ret = "Num7";
        break;
    case sf::Keyboard::Num8:

        ret = "Num8";
        break;
    case sf::Keyboard::Num9:

        ret = "Num9";
        break;
    case sf::Keyboard::Escape:

        ret = "Escape";
        break;
    case sf::Keyboard::LControl:

        ret = "LControl";
        break;
    case sf::Keyboard::LShift:

        ret = "LShift";
        break;
    case sf::Keyboard::LAlt:

        ret = "LAlt";
        break;
    case sf::Keyboard::LSystem:

        ret = "LSystem";
        break;
    case sf::Keyboard::RControl:

        ret = "RControl";
        break;
    case sf::Keyboard::RShift:

        ret = "RShift";
        break;
    case sf::Keyboard::RAlt:

        ret = "RAlt";
        break;
    case sf::Keyboard::RSystem:

        ret = "RSystem";
        break;
    case sf::Keyboard::Menu:

        ret = "Menu";
        break;
    case sf::Keyboard::LBracket:

        ret = "LBracket";
        break;
    case sf::Keyboard::RBracket:

        ret = "RBracket";
        break;
    case sf::Keyboard::SemiColon:

        ret = "SemiColon";
        break;
    case sf::Keyboard::Comma:

        ret = "Comma";
        break;
    case sf::Keyboard::Period:

        ret = "Period";
        break;
    case sf::Keyboard::Quote:

        ret = "Quote";
        break;
    case sf::Keyboard::Slash:

        ret = "Slash";
        break;
    case sf::Keyboard::BackSlash:

        ret = "BackSlash";
        break;
    case sf::Keyboard::Tilde:

        ret = "Tilde";
        break;
    case sf::Keyboard::Equal:

        ret = "Equal";
        break;
    case sf::Keyboard::Dash:

        ret = "Dash";
        break;
    case sf::Keyboard::Space:

        ret = "Space";
        break;
    case sf::Keyboard::Return:

        ret = "Return";
        break;
    case sf::Keyboard::BackSpace:

        ret = "BackSpace";
        break;
    case sf::Keyboard::Tab:

        ret = "Tab";
        break;
    case sf::Keyboard::PageUp:

        ret = "PageUp";
        break;
    case sf::Keyboard::PageDown:

        ret = "PageDown";
        break;
    case sf::Keyboard::End:

        ret = "End";
        break;
    case sf::Keyboard::Home:

        ret = "Home";
        break;
    case sf::Keyboard::Insert:

        ret = "Insert";
        break;
    case sf::Keyboard::Delete:

        ret = "Delete";
        break;
    case sf::Keyboard::Add:

        ret = "Add";
        break;
    case sf::Keyboard::Subtract:

        ret = "Subtract";
        break;
    case sf::Keyboard::Multiply:

        ret = "Multiply";
        break;
    case sf::Keyboard::Divide:

        ret = "Divide";
        break;
    case sf::Keyboard::Left:

        ret = "Left";
        break;
    case sf::Keyboard::Right:

        ret = "Right";
        break;
    case sf::Keyboard::Up:

        ret = "Up";
        break;
    case sf::Keyboard::Down:

        ret = "Down";
        break;
    case sf::Keyboard::Numpad0:

        ret = "Numpad0";
        break;
    case sf::Keyboard::Numpad1:

        ret = "Numpad1";
        break;
    case sf::Keyboard::Numpad2:

        ret = "Numpad2";
        break;
    case sf::Keyboard::Numpad3:

        ret = "Numpad3";
        break;
    case sf::Keyboard::Numpad4:

        ret = "Numpad4";
        break;
    case sf::Keyboard::Numpad5:

        ret = "Numpad5";
        break;
    case sf::Keyboard::Numpad6:

        ret = "Numpad6";
        break;
    case sf::Keyboard::Numpad7:

        ret = "Numpad7";
        break;
    case sf::Keyboard::Numpad8:

        ret = "Numpad8";
        break;
    case sf::Keyboard::Numpad9:

        ret = "Numpad9";
        break;
    case sf::Keyboard::F1:

        ret = "F1";
        break;
    case sf::Keyboard::F2:

        ret = "F2";
        break;
    case sf::Keyboard::F3:

        ret = "F3";
        break;
    case sf::Keyboard::F4:

        ret = "F4";
        break;
    case sf::Keyboard::F5:

        ret = "F5";
        break;
    case sf::Keyboard::F6:

        ret = "F6";
        break;
    case sf::Keyboard::F7:

        ret = "F7";
        break;
    case sf::Keyboard::F8:

        ret = "F8";
        break;
    case sf::Keyboard::F9:

        ret = "F9";
        break;
    case sf::Keyboard::F10:

        ret = "F10";
        break;
    case sf::Keyboard::F11:

        ret = "F11";
        break;
    case sf::Keyboard::F12:

        ret = "F12";
        break;
    case sf::Keyboard::F13:

        ret = "F13";
        break;
    case sf::Keyboard::F14:

        ret = "F14";
        break;
    case sf::Keyboard::F15:

        ret = "F15";
        break;
    case sf::Keyboard::Pause:

        ret = "Pause";
        break;
    case sf::Keyboard::KeyCount:

        ret = "KeyCount";
        break;


    default:
        ret = "Unknow";
        break;
    }
    return ret;
}
