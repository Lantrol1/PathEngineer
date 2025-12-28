#include "Render.hpp"
#include <iostream>
#include <sstream>
#include <cmath>

Render::Render()
    : mapSystem(nullptr), resourceManager(nullptr),
    constructionSystem(nullptr), randomEvent(nullptr),
    inputHandler(nullptr),
    hoveredCell(-1, -1), selectedCell(-1, -1),
    showGrid(true), showCoordinates(false), showWorkers(true),
    zoomLevel(1.0f) {

    terrainColors = {
        {Constants::TerrainType::PLAIN, Constants::COLOR_PLAIN},
        {Constants::TerrainType::RIVER, Constants::COLOR_RIVER},
        {Constants::TerrainType::MOUNTAIN, Constants::COLOR_MOUNTAIN},
        {Constants::TerrainType::SWAMP, Constants::COLOR_SWAMP}
    };

    obstacleColors = {
        {Constants::ObstacleType::NONE, sf::Color::Transparent},
        {Constants::ObstacleType::BOULDER, sf::Color(120, 120, 120)},
        {Constants::ObstacleType::FOREST, sf::Color(34, 139, 34)},
        {Constants::ObstacleType::LAKE, sf::Color(65, 105, 225)},
        {Constants::ObstacleType::CLIFF, sf::Color(101, 67, 33)},
        {Constants::ObstacleType::RUINS, sf::Color(160, 82, 45)}
    };

    constructionColors = {
        {Constants::ConstructionType::BUILD_ROAD, Constants::COLOR_ROAD},
        {Constants::ConstructionType::BUILD_BRIDGE, Constants::COLOR_BRIDGE},
        {Constants::ConstructionType::BUILD_TUNNEL, Constants::COLOR_TUNNEL},
        {Constants::ConstructionType::CLEAR_OBSTACLE, sf::Color::Yellow},
        {Constants::ConstructionType::REINFORCE_MOUNTAIN, sf::Color::Magenta}
    };
}

void Render::initialize(MapSystem* mapSys, ResourceManager* resMgr, ConstructionSystem* constSys, RandomEvent* eventSys, InputHandler* inputHndlr) {
    mapSystem = mapSys;
    resourceManager = resMgr;
    constructionSystem = constSys;
    randomEvent = eventSys;
    inputHandler = inputHndlr;

    // 初始化视图
    gameView = sf::View(sf::FloatRect(0, 0, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    uiView = sf::View(sf::FloatRect(0, 0, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));

    // 加载字体
    if (!loadFont("assets/fonts/arial.ttf")) {
        std::cerr << "警告: 无法加载字体，使用默认字体" << std::endl;
    }

    // 加载纹理（在实际项目中这里会加载图片纹理）
    loadTextures();
}

void Render::draw(sf::RenderWindow& window) {
    window.setView(gameView);
    drawMap(window);
    drawGrid(window);
    drawCoordinates(window);
    window.setView(uiView);
    drawUI(window);
}

void Render::drawMap(sf::RenderWindow& window) {
    if (!mapSystem) return;

    const auto& grid = mapSystem->getGrid();

    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            drawCell(window, x, y, grid[y][x]);
        }
    }
}

void Render::drawCell(sf::RenderWindow& window, int x, int y, const Cell& cell) {
    drawTerrain(window, x, y, cell);
    drawObstacles(window, x, y, cell);
    drawRoads(window, x, y, cell);
    drawConstruction(window, x, y, cell);
    // 渲染覆盖层
    drawOverlay(window, x, y, cell);
}

void Render::drawTerrain(sf::RenderWindow& window, int x, int y, const Cell& cell) {
    sf::RectangleShape terrainShape(sf::Vector2f(Constants::GRID_SIZE, Constants::GRID_SIZE));
    terrainShape.setPosition(x * Constants::GRID_SIZE, y * Constants::GRID_SIZE);

    // 地形颜色
    auto it = terrainColors.find(cell.terrain);
    if (it != terrainColors.end()) {
        terrainShape.setFillColor(it->second);
    }
    else {
        terrainShape.setFillColor(sf::Color::White);
    }

    // 地形边框
    terrainShape.setOutlineThickness(1.0f);
    terrainShape.setOutlineColor(sf::Color(50, 50, 50, 100));

    window.draw(terrainShape);

    // 特殊地形标记
    if (cell.terrain == Constants::TerrainType::RIVER) {
        // 河流
        sf::RectangleShape wave(sf::Vector2f(Constants::GRID_SIZE - 4, 2));
        wave.setPosition(x * Constants::GRID_SIZE + 2, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 2);
        wave.setFillColor(sf::Color(200, 200, 255, 150));
        window.draw(wave);
    }
    else if (cell.terrain == Constants::TerrainType::MOUNTAIN) {
        // 山脉
        sf::ConvexShape mountain;
        mountain.setPointCount(3);
        mountain.setPoint(0, sf::Vector2f(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 2, y * Constants::GRID_SIZE + 5));
        mountain.setPoint(1, sf::Vector2f(x * Constants::GRID_SIZE + 10, y * Constants::GRID_SIZE + Constants::GRID_SIZE - 5));
        mountain.setPoint(2, sf::Vector2f(x * Constants::GRID_SIZE + Constants::GRID_SIZE - 10, y * Constants::GRID_SIZE + Constants::GRID_SIZE - 5));
        mountain.setFillColor(sf::Color(100, 100, 100, 200));
        window.draw(mountain);
    }
}

void Render::drawObstacles(sf::RenderWindow& window, int x, int y, const Cell& cell) {
    if (cell.obstacle == Constants::ObstacleType::NONE) return;

    auto it = obstacleColors.find(cell.obstacle);
    if (it == obstacleColors.end()) return;

    sf::Color obstacleColor = it->second;

    switch (cell.obstacle) {
    case Constants::ObstacleType::BOULDER: {
        // 绘制巨石
        sf::CircleShape boulder(Constants::GRID_SIZE / 3);
        boulder.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 6, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 6);
        boulder.setFillColor(obstacleColor);
        window.draw(boulder);
        break;
    }

    case Constants::ObstacleType::FOREST: {
        // 绘制森林
        for (int i = 0; i < 3; ++i) {
            sf::ConvexShape tree;
            tree.setPointCount(3);
            float offsetX = (i * Constants::GRID_SIZE / 3) + Constants::GRID_SIZE / 6;
            tree.setPoint(0, sf::Vector2f(x * Constants::GRID_SIZE + offsetX, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 3));
            tree.setPoint(1, sf::Vector2f(x * Constants::GRID_SIZE + offsetX - 5, y * Constants::GRID_SIZE + Constants::GRID_SIZE - 5));
            tree.setPoint(2, sf::Vector2f(x * Constants::GRID_SIZE + offsetX + 5, y * Constants::GRID_SIZE + Constants::GRID_SIZE - 5));
            tree.setFillColor(obstacleColor);
            window.draw(tree);
        }
        break;
    }

    case Constants::ObstacleType::LAKE: {
        // 湖泊
        sf::CircleShape lake(Constants::GRID_SIZE / 2);
        lake.setPosition(x * Constants::GRID_SIZE, y * Constants::GRID_SIZE);
        lake.setFillColor(obstacleColor);
        window.draw(lake);
        break;
    }

    case Constants::ObstacleType::CLIFF: {
        // 悬崖
        sf::VertexArray cliff(sf::LinesStrip, 5);
        for (int i = 0; i < 5; ++i) {
            float posX = x * Constants::GRID_SIZE + (i * Constants::GRID_SIZE / 4);
            float posY = y * Constants::GRID_SIZE + (i % 2 == 0 ? 10 : 20);
            cliff[i].position = sf::Vector2f(posX, posY);
            cliff[i].color = obstacleColor;
        }
        window.draw(cliff);
        break;
    }

    case Constants::ObstacleType::RUINS: {
        // 遗迹
        sf::RectangleShape ruins(sf::Vector2f(Constants::GRID_SIZE - 10, Constants::GRID_SIZE - 10));
        ruins.setPosition(x * Constants::GRID_SIZE + 5, y * Constants::GRID_SIZE + 5);
        ruins.setFillColor(obstacleColor);
        ruins.setOutlineThickness(2);
        ruins.setOutlineColor(sf::Color::Black);
        window.draw(ruins);
        break;
    }

    default:
        break;
    }
}

void Render::drawRoads(sf::RenderWindow& window, int x, int y, const Cell& cell) const{
    // 道路
    if (cell.hasRoad) {
        sf::RectangleShape road(sf::Vector2f(Constants::GRID_SIZE - 8, Constants::GRID_SIZE - 8));
        road.setPosition(x * Constants::GRID_SIZE + 4, y * Constants::GRID_SIZE + 4);

        if (cell.hasBridge) {
            road.setFillColor(Constants::COLOR_BRIDGE);
            // 桥梁
            sf::RectangleShape bridgeLine(sf::Vector2f(Constants::GRID_SIZE - 12, 2));
            bridgeLine.setPosition(x * Constants::GRID_SIZE + 6, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 2);
            bridgeLine.setFillColor(sf::Color::White);
            window.draw(bridgeLine);
        }
        else if (cell.hasTunnel) {
            road.setFillColor(Constants::COLOR_TUNNEL);
            // 隧道
            sf::RectangleShape tunnelMark(sf::Vector2f(4, Constants::GRID_SIZE - 12));
            tunnelMark.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 2 - 2,
                y * Constants::GRID_SIZE + 6);
            tunnelMark.setFillColor(sf::Color::Black);
            window.draw(tunnelMark);
        }
        else {
            road.setFillColor(Constants::COLOR_ROAD);
        }

        window.draw(road);
    }

    // 起点终点
    if (cell.isStartPoint) {
        sf::CircleShape startMarker(Constants::GRID_SIZE / 4);
        startMarker.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 4, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 4);
        startMarker.setFillColor(sf::Color::Green);
        window.draw(startMarker);

        // 起点文字
        sf::Text startText("S", font, 12);
        startText.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 2 - 3, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 2 - 6);
        startText.setFillColor(sf::Color::Black);
        window.draw(startText);
    }

    if (cell.isEndPoint) {
        sf::CircleShape endMarker(Constants::GRID_SIZE / 4);
        endMarker.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 4, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 4);
        endMarker.setFillColor(sf::Color::Red);
        window.draw(endMarker);

        // 终点文字
        sf::Text endText("E", font, 12);
        endText.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE / 2 - 3, y * Constants::GRID_SIZE + Constants::GRID_SIZE / 2 - 6);
        endText.setFillColor(sf::Color::Black);
        window.draw(endText);
    }
}

void Render::drawConstruction(sf::RenderWindow& window, int x, int y, const Cell& cell) {
    if (!constructionSystem) return;

    const ConstructionTask* task = constructionSystem->getTask(x, y);
    if (!task) return;

    // 绘制建设进度条
    float progress = task->getProgress();

    // 进度条背景
    sf::RectangleShape progressBg(sf::Vector2f(Constants::GRID_SIZE - 4, 6));
    progressBg.setPosition(x * Constants::GRID_SIZE + 2, y * Constants::GRID_SIZE + 2);
    progressBg.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(progressBg);

    // 进度条前景
    sf::RectangleShape progressBar(sf::Vector2f((Constants::GRID_SIZE - 4) * progress, 6));
    progressBar.setPosition(x * Constants::GRID_SIZE + 2, y * Constants::GRID_SIZE + 2);
    progressBar.setFillColor(sf::Color(0, 255, 0, 200));
    window.draw(progressBar);

    // 绘制工人数量（如果启用）
    if (showWorkers && task->assignedWorkers > 0) {
        sf::Text workerText(std::to_string(task->assignedWorkers), font, 10);
        workerText.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE - 12,
            y * Constants::GRID_SIZE + Constants::GRID_SIZE - 15);
        workerText.setFillColor(sf::Color::Yellow);
        window.draw(workerText);
    }

    // 建设类型图标
    auto it = constructionColors.find(task->type);
    if (it != constructionColors.end()) {
        sf::CircleShape icon(4);
        icon.setPosition(x * Constants::GRID_SIZE + Constants::GRID_SIZE - 10,
            y * Constants::GRID_SIZE + 5);
        icon.setFillColor(it->second);
        window.draw(icon);
    }
}

void Render::drawOverlay(sf::RenderWindow& window, int x, int y, const Cell& cell) {
    // 鼠标悬停
    if (hoveredCell.x == x && hoveredCell.y == y) {
        sf::RectangleShape hover(sf::Vector2f(Constants::GRID_SIZE, Constants::GRID_SIZE));
        hover.setPosition(x * Constants::GRID_SIZE, y * Constants::GRID_SIZE);
        hover.setFillColor(sf::Color(255, 255, 255, 50));
        window.draw(hover);
    }

    // 鼠标选中
    if (selectedCell.x == x && selectedCell.y == y) {
        sf::RectangleShape selected(sf::Vector2f(Constants::GRID_SIZE, Constants::GRID_SIZE));
        selected.setPosition(x * Constants::GRID_SIZE, y * Constants::GRID_SIZE);
        selected.setFillColor(sf::Color::Transparent);
        selected.setOutlineThickness(2.0f);
        selected.setOutlineColor(sf::Color::Yellow);
        window.draw(selected);
    }
    if (cell.isReinforced) {
        sf::RectangleShape reinforce(sf::Vector2f(Constants::GRID_SIZE, 3));
        reinforce.setPosition(x * Constants::GRID_SIZE, y * Constants::GRID_SIZE);
        reinforce.setFillColor(sf::Color::Blue);
        window.draw(reinforce);
    }
}

void Render::drawUI(sf::RenderWindow& window) {
    drawResourcePanel(window);
    drawStatusPanel(window);
    drawConstructionPanel(window);
    drawEventPanel(window);
    drawTooltip(window);
}

void Render::drawResourcePanel(sf::RenderWindow& window) {
    if (!resourceManager) return;
    sf::RectangleShape panel(sf::Vector2f(200, 120));
    panel.setPosition(1280, 10);//10，10
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color::White);
    window.draw(panel);
    sf::Text title("Resource Status", font, 16);
    title.setPosition(1290, 15);
    title.setFillColor(sf::Color::White);
    window.draw(title);
    std::stringstream resources;
    resources << "Budget: " << resourceManager->getBudget() << "\n"
        << "Workers: " << resourceManager->getAvailableWorkers() << "/"
        << resourceManager->getTotalWorkers() << "\n"
        << "Environment: " << resourceManager->getEnvironmentScore() << "\n"
        << "Round: " << resourceManager->getCurrentTurn() << "/"
        << resourceManager->getTotalTurns();
    sf::Text resourceText(resources.str(), font, 14);
    resourceText.setPosition(1290, 40);
    resourceText.setFillColor(sf::Color::White);
    resourceText.setLineSpacing(1.2f);
    window.draw(resourceText);
    if (resourceManager->isBudgetCritical()) {
        sf::Text warning("Short on budget!", font, 12);
        warning.setPosition(1290, 105);
        warning.setFillColor(sf::Color::Red);
        window.draw(warning);
    }
}

void Render::drawStatusPanel(sf::RenderWindow& window) {
    // 状态面板背景
    sf::RectangleShape panel(sf::Vector2f(250, 100));
    panel.setPosition(Constants::WINDOW_WIDTH - 260, 10);
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color::White);
    window.draw(panel);

    // 状态标题
    sf::Text title("Status Info", font, 16);
    title.setPosition(Constants::WINDOW_WIDTH - 250, 15);
    title.setFillColor(sf::Color::White);
    window.draw(title);

    if (inputHandler) {
        sf::Text modeText("Mode: " + inputHandler->getInputModeName(), font, 12);
        modeText.setPosition(Constants::WINDOW_WIDTH - 250, 40);
        modeText.setFillColor(sf::Color::Yellow);
        window.draw(modeText);
    }

    if (constructionSystem) {
        std::string pathStatus = constructionSystem->isPathComplete() ?
            "Connected ✓" : "Not connected ✗";
        sf::Text pathText(pathStatus, font, 12);
        pathText.setPosition(Constants::WINDOW_WIDTH - 250, 60);
        pathText.setFillColor(constructionSystem->isPathComplete() ?
            sf::Color::Green : sf::Color::Red);
        window.draw(pathText);
    }

    // 悬停单元格信息
    if (mapSystem && hoveredCell.x != -1 && hoveredCell.y != -1) {
        const Cell& cell = mapSystem->getCell(hoveredCell.x, hoveredCell.y);
        std::stringstream cellInfo;
        cellInfo << "Position: (" << hoveredCell.x << ", " << hoveredCell.y << ")";

        sf::Text cellText(cellInfo.str(), font, 12);
        cellText.setPosition(Constants::WINDOW_WIDTH - 250, 80);
        cellText.setFillColor(sf::Color::Cyan);
        window.draw(cellText);
    }
}

void Render::drawConstructionPanel(sf::RenderWindow& window) {
    // 建设面板背景
    sf::RectangleShape panel(sf::Vector2f(150, 200));
    panel.setPosition(Constants::WINDOW_WIDTH - 160, Constants::WINDOW_HEIGHT - 210);
    panel.setFillColor(sf::Color(0, 0, 0, 180));
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color::White);
    window.draw(panel);

    // 建设标题
    sf::Text title("Construction operations", font, 16);
    title.setPosition(Constants::WINDOW_WIDTH - 150, Constants::WINDOW_HEIGHT - 200);
    title.setFillColor(sf::Color::White);
    window.draw(title);

    // 建设选项列表
    std::vector<std::pair<Constants::ConstructionType, std::string>> constructions = {
        {Constants::ConstructionType::BUILD_ROAD, "R - Build road"},
        {Constants::ConstructionType::BUILD_BRIDGE, "B - Build bridge"},
        {Constants::ConstructionType::BUILD_TUNNEL, "T - Build tunnel"},
        {Constants::ConstructionType::CLEAR_OBSTACLE, "C - Clear obstacle"},
        {Constants::ConstructionType::REINFORCE_MOUNTAIN, "F - Reinforce mountain"}
    };

    for (size_t i = 0; i < constructions.size(); ++i) {
        sf::Text consText(constructions[i].second, font, 12);
        consText.setPosition(Constants::WINDOW_WIDTH - 150,
            Constants::WINDOW_HEIGHT - 170 + i * 25);

        // 高亮当前选择的建设类型
        if (inputHandler && inputHandler->getSelectedConstruction() == constructions[i].first) {
            consText.setFillColor(sf::Color::Yellow);
            consText.setStyle(sf::Text::Bold);
        }
        else {
            consText.setFillColor(sf::Color::White);
        }

        window.draw(consText);
    }
}

void Render::drawEventPanel(sf::RenderWindow& window) {
    if (!randomEvent) return;

    const auto& activeEvents = randomEvent->getActiveEvents();
    if (activeEvents.empty()) return;

    // 事件面板背景
    sf::RectangleShape panel(sf::Vector2f(300, 80));
    panel.setPosition(Constants::WINDOW_WIDTH / 2 - 150, 10);
    panel.setFillColor(sf::Color(0, 0, 0, 200));
    panel.setOutlineThickness(1);
    panel.setOutlineColor(sf::Color::Red);
    window.draw(panel);

    // 事件标题
    sf::Text title("Current events", font, 14);
    title.setPosition(Constants::WINDOW_WIDTH / 2 - 140, 15);
    title.setFillColor(sf::Color::Red);
    window.draw(title);

    // 显示第一个活跃事件（简化显示）
    if (!activeEvents.empty()) {
        const auto& event = activeEvents[0];
        sf::Text eventText(event.title + ": " + event.description, font, 12);
        eventText.setPosition(Constants::WINDOW_WIDTH / 2 - 140, 35);
        eventText.setFillColor(event.isPositive ? sf::Color::Green : sf::Color::Red);
        eventText.setLineSpacing(1.2f);

        // 自动换行处理
        std::string displayText = event.title + ": " + event.description;
        if (displayText.length() > 40) {
            displayText = displayText.substr(0, 37) + "...";
        }
        eventText.setString(displayText);

        window.draw(eventText);
    }
}

void Render::drawTooltip(sf::RenderWindow& window) {
    if (hoveredCell.x == -1 || hoveredCell.y == -1 || !mapSystem) return;

    const Cell& cell = mapSystem->getCell(hoveredCell.x, hoveredCell.y);

    // 工具提示背景
    sf::RectangleShape tooltip(sf::Vector2f(180, 80));
    tooltip.setPosition(hoveredCell.x * Constants::GRID_SIZE + 20,
        hoveredCell.y * Constants::GRID_SIZE - 90);
    tooltip.setFillColor(sf::Color(0, 0, 0, 220));
    tooltip.setOutlineThickness(1);
    tooltip.setOutlineColor(sf::Color::White);
    window.draw(tooltip);

    // 工具提示内容
    std::stringstream tooltipText;
    tooltipText << "Terrain: " << getTerrainName(cell.terrain) << "\n";

    if (cell.obstacle != Constants::ObstacleType::NONE) {
        tooltipText << "Obstacle: " << getObstacleName(cell.obstacle) << "\n";
    }

    if (cell.hasRoad) tooltipText << "The road already exists\n";
    if (cell.hasBridge) tooltipText << "The bridge already exists\n";
    if (cell.hasTunnel) tooltipText << "The tunnel already exists\n";
    if (cell.isReinforced) tooltipText << "Reinforced\n";

    if (cell.assignedWorkers > 0) {
        tooltipText << "Workers: " << cell.assignedWorkers;
    }

    sf::Text text(tooltipText.str(), font, 12);
    text.setPosition(hoveredCell.x * Constants::GRID_SIZE + 25,
        hoveredCell.y * Constants::GRID_SIZE - 85);
    text.setFillColor(sf::Color::White);
    text.setLineSpacing(1.1f);
    window.draw(text);
}

void Render::drawGrid(sf::RenderWindow& window) {
    sf::Color gridColor(255, 255, 255, 50);

    // 绘制垂直线
    for (int x = 0; x <= Constants::MAP_WIDTH; ++x) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x * Constants::GRID_SIZE, 0), gridColor),
            sf::Vertex(sf::Vector2f(x * Constants::GRID_SIZE, Constants::MAP_HEIGHT * Constants::GRID_SIZE), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }

    // 绘制水平线
    for (int y = 0; y <= Constants::MAP_HEIGHT; ++y) {
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0, y * Constants::GRID_SIZE), gridColor),
            sf::Vertex(sf::Vector2f(Constants::MAP_WIDTH * Constants::GRID_SIZE, y * Constants::GRID_SIZE), gridColor)
        };
        window.draw(line, 2, sf::Lines);
    }
}

void Render::drawCoordinates(sf::RenderWindow& window) const{
    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            std::string coord = std::to_string(x) + "," + std::to_string(y);
            sf::Text coordText(coord, font, 12);
            coordText.setPosition(x * Constants::GRID_SIZE + 2, y * Constants::GRID_SIZE + 2);
            coordText.setFillColor(sf::Color(0, 0, 0, 100));
            window.draw(coordText);
        }
    }
}

void Render::setViewCenter(const sf::Vector2f& center) {
    gameView.setCenter(center);
}

void Render::zoom(float factor) {
    zoomLevel *= factor;
    gameView.zoom(factor);
}

void Render::resetView() {
    gameView = sf::View(sf::FloatRect(0, 0, Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
    zoomLevel = 1.0f;
}
void Render::setHoveredCell(int x, int y) {
    hoveredCell = sf::Vector2i(x, y);
}
void Render::setSelectedCell(int x, int y) {
    selectedCell = sf::Vector2i(x, y);
}
bool Render::loadFont(const std::string& fontPath) {
    return font.loadFromFile(fontPath);
}
bool Render::loadTextures() {
    return true;
}

sf::Color Render::getCellColor(const Cell& cell) const {
    auto it = terrainColors.find(cell.terrain);
    if (it != terrainColors.end()) {
        return it->second;
    }
    return sf::Color::White;
}

sf::Color Render::getHoverColor(const sf::Color& baseColor) const {
    return sf::Color(
        std::min(baseColor.r + 50, 255),
        std::min(baseColor.g + 50, 255),
        std::min(baseColor.b + 50, 255),
        baseColor.a
    );
}

sf::Color Render::getSelectedColor(const sf::Color& baseColor) const {
    return sf::Color(
        baseColor.r,
        baseColor.g,
        std::min(baseColor.b + 100, 255),
        baseColor.a
    );
}

std::string Render::getTerrainName(Constants::TerrainType terrain) const {
    switch (terrain) {
    case Constants::TerrainType::PLAIN: return "Plain";
    case Constants::TerrainType::RIVER: return "River";
    case Constants::TerrainType::MOUNTAIN: return "Mountain";
    case Constants::TerrainType::SWAMP: return "Swamp";
    default: return "Default";
    }
}

std::string Render::getObstacleName(Constants::ObstacleType obstacle) const {
    switch (obstacle) {
    case Constants::ObstacleType::BOULDER: return "Boulder";
    case Constants::ObstacleType::FOREST: return "Forest";
    case Constants::ObstacleType::LAKE: return "Lake";
    case Constants::ObstacleType::CLIFF: return "Cliff";
    case Constants::ObstacleType::RUINS: return "Ruins";
    default: return "None";
    }
}

void Render::printRenderState() const {
    std::cout << "=== 渲染状态 ===" << std::endl;
    std::cout << "悬停单元格: (" << hoveredCell.x << ", " << hoveredCell.y << ")" << std::endl;
    std::cout << "选中单元格: (" << selectedCell.x << ", " << selectedCell.y << ")" << std::endl;
    std::cout << "显示网格: " << (showGrid ? "是" : "否") << std::endl;
    std::cout << "显示坐标: " << (showCoordinates ? "是" : "否") << std::endl;
    std::cout << "显示工人: " << (showWorkers ? "是" : "否") << std::endl;
    std::cout << "缩放级别: " << zoomLevel << std::endl;
    std::cout << "===============" << std::endl;
}