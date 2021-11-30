#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>

#include "EntityManager.h"

class Scene_Pause : public Scene
{

protected:

    std::vector<std::string>    m_menuStrings;
    std::vector<std::string>    m_optionStrings;
    std::vector<std::string>    m_inventoryStrings;
    std::vector<int>            m_arrows;
    sf::Text                    m_menuText;
    size_t                      m_selectedMenuIndex = 0;
    sf::Texture                 m_background;
    sf::Shader                  m_backgroundShader;
    bool                        m_mainMenu;
    bool                        m_optionsMenu;
    bool                        m_inventoryMenu;

    void init();
    void update();
    void onEnd();
    void sDoAction(const Action& action);
    void sDoMouseAction(const Action& action);
    void sRender();
    void renderMenu();
    void renderOptions();
    void renderInventory();
    void renderBackground();
    void mainMenuActions(const Action& action);
    void optionMenuActions(const Action& action);
    void inventoryMenuActions(const Action& action);
    void downKeystroke(std::vector<std::string> arr);
    void upKeystroke(std::vector<std::string> arr);
    void playSelectSound();
    void playMenuSound();

public:

    Scene_Pause(GameEngine* gameEngine, std::vector<int> arrows, sf::Texture background);
};
