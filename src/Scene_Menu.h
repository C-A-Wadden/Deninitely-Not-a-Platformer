#pragma once

#include "Common.h"
#include "Scene.h"
#include <map>
#include <memory>
#include <deque>

#include "EntityManager.h"

class Scene_Menu : public Scene
{

protected:

    std::string                 m_title;
    std::vector<std::string>    m_menuStrings;
    std::vector<std::string>    m_optionMenuStrings;
    std::vector<std::string>    m_rebindMenuStrings;
    std::vector<std::string>    m_editMenuStrings;
    std::vector<std::string>    m_difficultyStrings;
    std::vector<std::string>    m_optionPaths;
    sf::Text                    m_menuText;
    size_t                      m_selectedMenuIndex = 0;
    sf::Keyboard::Key           m_rebindKey;
    bool                        m_startMenu;
    bool                        m_optionsMenu;
    bool                        m_rebindMenu;
    bool                        m_editMenu;
    bool                        m_rebind;
    int                         m_difficulty;
    int                         m_sfxVol;
    int                         m_musicVol;
    int                         m_frame;
    int                         m_lastFrame;

    void init();
    void update();
    void onEnd();
    void sDoAction(const Action& action);
    void sDoMouseAction(const Action& action);
    void setBackground();
    void displayMain();
    void displayStart();
    void displayOptions();
    void displayRebind();
    void displayEdit();
    void displayControls();
    void playMenuSound();
    void playSelectSound();
    void updateOptions();
    void upKeystroke(std::vector<std::string> arr);
    void downKeystroke(std::vector<std::string> arr);
    void startMenuActions(const Action& action);
    void mainMenuActions(const Action& action);
    void optionMenuActions(const Action& action);
    void rebindMenuActions(const Action& action);
    void editMenuActions(const Action& action);
    std::string keyString(const sf::Keyboard::Key& k);
    bool shouldContinue();
    void passKey(const sf::Keyboard::Key& k);

public:

    Scene_Menu(GameEngine* gameEngine = nullptr);
    void sRender();
};

namespace options
{
    extern std::vector<int>            m_optionSettings;
    extern std::vector<sf::Keyboard::Key>   m_rebindKeys;
    extern std::vector<bool>                 m_beatenLevels;
}