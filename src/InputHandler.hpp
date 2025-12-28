#ifndef INPUTHANDLER_HPP
#define INPUTHANDLER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <functional>
#include "Constants.hpp"
#include "ConstructionSystem.hpp"
#include "MapSystem.hpp"
#include "ResourceManager.hpp"

// 前向声明
class GameEngine;

// 输入模式枚举
enum class InputMode {
    NORMAL,
    CONSTRUCTION,
    WORKER_ASSIGNMENT,
    CELL_INSPECTION
};

// 鼠标状态结构体
struct MouseState {
    sf::Vector2i screenPosition;
    sf::Vector2i gridPosition;
    bool leftPressed;
    bool rightPressed;
    bool leftClicked;
    bool rightClicked;
    bool isDragging;

    MouseState()
        : screenPosition(-1, -1), gridPosition(-1, -1),
        leftPressed(false), rightPressed(false),
        leftClicked(false), rightClicked(false),
        isDragging(false) {
    }
};

class InputHandler {
private:
    ConstructionSystem* constructionSystem;
    MapSystem* mapSystem;
    ResourceManager* resourceManager;
    GameEngine* gameEngine;

    // 输入状态
    InputMode currentMode;
    Constants::ConstructionType selectedConstruction;
    MouseState mouseState;
    sf::Vector2i lastGridPosition;

    // 键盘状态
    std::map<sf::Keyboard::Key, bool> keyStates;
    std::map<sf::Keyboard::Key, bool> prevKeyStates;

    // 回调函数
    std::function<void(int, int, Constants::ConstructionType)> onConstructionSelected;
    std::function<void(int, int)> onCellHovered;
    std::function<void(int, int)> onCellSelected;
    std::function<void(const std::string&)> onInputMessage;

    // 内部方法
    sf::Vector2i getCellAtPosition(const sf::Vector2f& position) const;
    void handleMouseMove(const sf::Event::MouseMoveEvent& event, sf::RenderWindow& window);
    void handleMouseClick(const sf::Event::MouseButtonEvent& event, sf::RenderWindow& window);
    void handleKeyPress(const sf::Event::KeyEvent& event);
     
    // 模式特定的处理
    void handleNormalMode(int x, int y);
    void handleConstructionMode(int x, int y);
    void handleWorkerAssignmentMode(int x, int y);
    void handleCellInspectionMode(int x, int y);

    // 建设操作
    bool attemptConstruction(int x, int y);
    bool attemptWorkerAssignment(int x, int y, int count = 1);

public:
    InputHandler();

    void initialize(ConstructionSystem* constSys, MapSystem* mapSys,
        ResourceManager* resMgr = nullptr, GameEngine* engine = nullptr);

    // 主更新方法
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update();

    // 模式控制
    void setInputMode(InputMode mode);
    void setConstructionType(Constants::ConstructionType type);
    void toggleConstructionMode(Constants::ConstructionType type);

    // 状态获取
    InputMode getCurrentMode() const { return currentMode; }
    Constants::ConstructionType getSelectedConstruction() const { return selectedConstruction; }
    sf::Vector2i getHoveredCell() const { return mouseState.gridPosition; }
    MouseState getMouseState() const { return mouseState; }

    // 回调设置
    void setConstructionSelectedCallback(std::function<void(int, int, Constants::ConstructionType)> callback);
    void setCellHoveredCallback(std::function<void(int, int)> callback);
    void setCellSelectedCallback(std::function<void(int, int)> callback);
    void setInputMessageCallback(std::function<void(const std::string&)> callback);

    // 工具方法
    std::string getConstructionName(Constants::ConstructionType type) const;
    std::string getInputModeName() const;

    // 调试工具
    void printInputState() const;
};

#endif