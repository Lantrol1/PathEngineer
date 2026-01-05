#include "InputHandler.hpp"
#include "GameEngine.hpp"
#include <iostream>
#include <sstream>

InputHandler::InputHandler()
    : constructionSystem(nullptr), mapSystem(nullptr),
    resourceManager(nullptr), gameEngine(nullptr),
    currentMode(InputMode::NORMAL),
    selectedConstruction(Constants::ConstructionType::NONE) {
    onConstructionSelected = [](int, int, Constants::ConstructionType) {};
    onCellHovered = [](int, int) {};
    onCellSelected = [](int, int) {};
    onInputMessage = [](const std::string&) {};
}

void InputHandler::initialize(ConstructionSystem* constSys, MapSystem* mapSys,
    ResourceManager* resMgr, GameEngine* engine) {
    constructionSystem = constSys;
    mapSystem = mapSys;
    resourceManager = resMgr;
    gameEngine = engine;
}

void InputHandler::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    prevKeyStates = keyStates;
    switch (event.type) {
    case sf::Event::MouseMoved:
        handleMouseMove(event.mouseMove, window);
        break;

    case sf::Event::MouseButtonPressed:
        mouseState.leftPressed = (event.mouseButton.button == sf::Mouse::Left);
        mouseState.rightPressed = (event.mouseButton.button == sf::Mouse::Right);
        handleMouseClick(event.mouseButton, window);
        break;

    case sf::Event::MouseButtonReleased:
        if (event.mouseButton.button == sf::Mouse::Left) {
            mouseState.leftPressed = false;
        }
        else if (event.mouseButton.button == sf::Mouse::Right) {
            mouseState.rightPressed = false;
        }
        break;

    case sf::Event::KeyPressed:
        handleKeyPress(event.key);
        keyStates[event.key.code] = true;
        break;

    case sf::Event::KeyReleased:
        keyStates[event.key.code] = false;
        break;
    default:
        break;
    }
}
void InputHandler::update() {
    mouseState.leftClicked = false;
    mouseState.rightClicked = false;
}

void InputHandler::handleMouseMove(const sf::Event::MouseMoveEvent& event, sf::RenderWindow& window) {
    mouseState.screenPosition = sf::Vector2i(event.x, event.y);
    sf::Vector2f worldPos = window.mapPixelToCoords(mouseState.screenPosition);
    sf::Vector2i newGridPos = getCellAtPosition(worldPos);
    if (newGridPos != mouseState.gridPosition) {
        mouseState.gridPosition = newGridPos;
        if (mouseState.gridPosition.x != -1 && mouseState.gridPosition.y != -1) {
            onCellHovered(mouseState.gridPosition.x, mouseState.gridPosition.y);
        }
    }
}

void InputHandler::handleMouseClick(const sf::Event::MouseButtonEvent& event, sf::RenderWindow& window) {
    sf::Vector2f worldPos = window.mapPixelToCoords(sf::Vector2i(event.x, event.y));
    sf::Vector2i gridPos = getCellAtPosition(worldPos);
    if (gridPos.x == -1 || gridPos.y == -1) {
        return;
    }
    if (event.button == sf::Mouse::Left) {
        mouseState.leftClicked = true;
        switch (currentMode) {
        case InputMode::NORMAL:
            handleCellInspectionMode(gridPos.x, gridPos.y);
            break;

        case InputMode::CONSTRUCTION:
            handleConstructionMode(gridPos.x, gridPos.y);
            break;

        case InputMode::WORKER_ASSIGNMENT:
            handleWorkerAssignmentMode(gridPos.x, gridPos.y);
            break;
        }
        onCellSelected(gridPos.x, gridPos.y);
    }
}

void InputHandler::handleKeyPress(const sf::Event::KeyEvent& event) {
    switch (event.code) {
    case sf::Keyboard::Escape:
        setInputMode(InputMode::NORMAL);
        onInputMessage("返回正常模式");
        break;
    case sf::Keyboard::R:
        toggleConstructionMode(Constants::ConstructionType::BUILD_ROAD);
        break;

    case sf::Keyboard::B:
        toggleConstructionMode(Constants::ConstructionType::BUILD_BRIDGE);
        break;

    case sf::Keyboard::T:
        toggleConstructionMode(Constants::ConstructionType::BUILD_TUNNEL);
        break;

    case sf::Keyboard::C:
        toggleConstructionMode(Constants::ConstructionType::CLEAR_OBSTACLE);
        break;

    case sf::Keyboard::F:
        toggleConstructionMode(Constants::ConstructionType::REINFORCE_MOUNTAIN);
        break;

    case sf::Keyboard::W:
        setInputMode(InputMode::WORKER_ASSIGNMENT);
        onInputMessage("点击单元格分配工人");
        break;

    default:
        break;
    }
}

sf::Vector2i InputHandler::getCellAtPosition(const sf::Vector2f& position) const {
    if (!mapSystem) return sf::Vector2i(-1, -1);
    int cellX = static_cast<int>(position.x / Constants::GRID_SIZE);
    int cellY = static_cast<int>(position.y / Constants::GRID_SIZE);
    if (cellX >= 0 && cellX < mapSystem->getWidth() &&
        cellY >= 0 && cellY < mapSystem->getHeight()) {
        return sf::Vector2i(cellX, cellY);
    }
    return sf::Vector2i(-1, -1);
}

void InputHandler::handleConstructionMode(int x, int y) {
    if (attemptConstruction(x, y)) {
        onInputMessage("开始建设: " + getConstructionName(selectedConstruction));
    }
}

void InputHandler::handleWorkerAssignmentMode(int x, int y) {
    if (constructionSystem && constructionSystem->hasActiveTask(x, y)) {
        attemptWorkerAssignment(x, y, 1);
    }
    else {
        onInputMessage("该位置没有活跃的建设任务");
    }
}

void InputHandler::handleCellInspectionMode(int x, int y) {
    if (mapSystem) {
        const Cell& cell = mapSystem->getCell(x, y);
        std::stringstream info;
        info << "=== 单元格详情 ===\n";
        info << "位置: (" << x << ", " << y << ")\n";
        info << "地形: ";
        switch (cell.terrain) {
        case Constants::TerrainType::PLAIN: info << "平原"; break;
        case Constants::TerrainType::RIVER: info << "河流"; break;
        case Constants::TerrainType::MOUNTAIN: info << "山脉"; break;
        case Constants::TerrainType::SWAMP: info << "沼泽"; break;
        }
        info << "\n";
        // 障碍物
        if (cell.obstacle != Constants::ObstacleType::NONE) {
            info << "障碍物: ";
            switch (cell.obstacle) {
            case Constants::ObstacleType::BOULDER: info << "巨石"; break;
            case Constants::ObstacleType::FOREST: info << "森林"; break;
            case Constants::ObstacleType::LAKE: info << "湖泊"; break;
            case Constants::ObstacleType::CLIFF: info << "悬崖"; break;
            case Constants::ObstacleType::RUINS: info << "遗迹"; break;
            default: break;
            }
            info << "\n";
        }

        // 建设状态
        if (cell.hasRoad) info << "有道路\n";
        if (cell.hasBridge) info << "有桥梁\n";
        if (cell.hasTunnel) info << "有隧道\n";
        if (cell.isReinforced) info << "已加固\n";

        // 劳动力
        if (cell.assignedWorkers > 0) {
            info << "分配工人: " << cell.assignedWorkers << "\n";
        }
        // 建设任务
        if (constructionSystem) {
            const ConstructionTask* task = constructionSystem->getTask(x, y);
            if (task) {
                info << "建设任务: " << task->description << "\n";
                info << "进度: " << static_cast<int>(task->getProgress() * 100) << "%\n";
                info << "工人: " << task->assignedWorkers << "\n";
            }
        }
        onInputMessage(info.str());
    }
}

bool InputHandler::attemptConstruction(int x, int y) {
    if (!constructionSystem) return false;

    return constructionSystem->startConstruction(x, y, selectedConstruction);
}

bool InputHandler::attemptWorkerAssignment(int x, int y, int count) {
    if (!constructionSystem || !resourceManager) return false;
    if (resourceManager->getAvailableWorkers() < count) {
        onInputMessage("可用工人不足");
        return false;
    }
    if (!constructionSystem->hasActiveTask(x, y)) {
        onInputMessage("该位置没有建设任务");
        return false;
    }
    if (constructionSystem->assignWorkersToTask(x, y, count)) {
        std::stringstream msg;
        msg << "分配 " << count << " 名工人到 (" << x << ", " << y << ")";
        onInputMessage(msg.str());
        return true;
    }
    return false;
}

void InputHandler::setInputMode(InputMode mode) {
    currentMode = mode;

    if (mode != InputMode::CONSTRUCTION) {
        selectedConstruction = Constants::ConstructionType::NONE;
    }
}

void InputHandler::setConstructionType(Constants::ConstructionType type) {
    selectedConstruction = type;
    currentMode = InputMode::CONSTRUCTION;
    std::stringstream msg;
    msg << "建设模式: " << getConstructionName(type);
    onInputMessage(msg.str());
}

void InputHandler::toggleConstructionMode(Constants::ConstructionType type) {
    setConstructionType(type);
}

std::string InputHandler::getConstructionName(Constants::ConstructionType type) const {
    switch (type) {
    case Constants::ConstructionType::BUILD_ROAD: return "Build Road";
    case Constants::ConstructionType::BUILD_BRIDGE: return "Build Bridge";
    case Constants::ConstructionType::BUILD_TUNNEL: return "Build Tunnel";
    case Constants::ConstructionType::CLEAR_OBSTACLE: return "Clear Obstacle";
    case Constants::ConstructionType::REINFORCE_MOUNTAIN: return "Reinforce Mountain";
    default: return "Unknown";
    }
}

std::string InputHandler::getInputModeName() const {
    switch (currentMode) {
    case InputMode::NORMAL: return "Normal";
    case InputMode::CONSTRUCTION: return "Construction - " + getConstructionName(selectedConstruction);
    case InputMode::WORKER_ASSIGNMENT: return "Worker Assignment";
    case InputMode::CELL_INSPECTION: return "Cell Inspection";
    default: return "Unknown";
    }
}

void InputHandler::setConstructionSelectedCallback(std::function<void(int, int, Constants::ConstructionType)> callback) {
    onConstructionSelected = callback;
}

void InputHandler::setCellHoveredCallback(std::function<void(int, int)> callback) {
    onCellHovered = callback;
}

void InputHandler::setCellSelectedCallback(std::function<void(int, int)> callback) {
    onCellSelected = callback;
}

void InputHandler::setInputMessageCallback(std::function<void(const std::string&)> callback) {
    onInputMessage = callback;
}

void InputHandler::printInputState() const {
    std::cout << "=== 输入状态 ===" << std::endl;
    std::cout << "模式: " << getInputModeName() << std::endl;
    std::cout << "鼠标位置: (" << mouseState.gridPosition.x << ", " << mouseState.gridPosition.y << ")" << std::endl;
    std::cout << "左键: " << (mouseState.leftPressed ? "按下" : "释放") << std::endl;
    std::cout << "右键: " << (mouseState.rightPressed ? "按下" : "释放") << std::endl;
    std::cout << "===============" << std::endl;
}