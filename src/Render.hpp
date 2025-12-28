#ifndef RENDER_HPP
#define RENDER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <map>
#include "Constants.hpp"
#include "MapSystem.hpp"
#include "ResourceManager.hpp"
#include "ConstructionSystem.hpp"
#include "RandomEvent.hpp"
#include "InputHandler.hpp"

// 渲染层类型
enum class RenderLayer {
    TERRAIN,
    OBSTACLES,
    CONSTRUCTION,
    ROADS,
    OVERLAY,
    UI
};

class Render {
private:
    MapSystem* mapSystem;
    ResourceManager* resourceManager;
    ConstructionSystem* constructionSystem;
    RandomEvent* randomEvent;
    InputHandler* inputHandler;

    // 渲染资源
    sf::Font font;
    sf::Texture terrainTextures;
    sf::Texture uiTextures;

    // 颜色映射
    std::map<Constants::TerrainType, sf::Color> terrainColors;
    std::map<Constants::ObstacleType, sf::Color> obstacleColors;
    std::map<Constants::ConstructionType, sf::Color> constructionColors;

    // 渲染状态
    sf::Vector2i hoveredCell;
    sf::Vector2i selectedCell;
    bool showGrid;
    bool showCoordinates;
    bool showWorkers;

    // 视图控制
    sf::View gameView;
    sf::View uiView;
    float zoomLevel;

    // 内部渲染方法
    void drawMap(sf::RenderWindow& window);
    void drawCell(sf::RenderWindow& window, int x, int y, const Cell& cell);
    void drawTerrain(sf::RenderWindow& window, int x, int y, const Cell& cell);
    void drawObstacles(sf::RenderWindow& window, int x, int y, const Cell& cell);
    void drawRoads(sf::RenderWindow& window, int x, int y, const Cell& cell) const;
    //void drawRoads(sf::RenderWindow& window, int x, int y, const Cell& cell);
    void drawConstruction(sf::RenderWindow& window, int x, int y, const Cell& cell);
    void drawOverlay(sf::RenderWindow& window, int x, int y, const Cell& cell);

    void drawUI(sf::RenderWindow& window);
    void drawResourcePanel(sf::RenderWindow& window);
    void drawStatusPanel(sf::RenderWindow& window);
    void drawConstructionPanel(sf::RenderWindow& window);
    void drawEventPanel(sf::RenderWindow& window);
    void drawTooltip(sf::RenderWindow& window);

    void drawGrid(sf::RenderWindow& window);
    void drawCoordinates(sf::RenderWindow& window) const;

    // 工具方法
    sf::Color getCellColor(const Cell& cell) const;
    sf::Color getHoverColor(const sf::Color& baseColor) const;
    sf::Color getSelectedColor(const sf::Color& baseColor) const;
    std::string getTerrainName(Constants::TerrainType terrain) const;
    std::string getObstacleName(Constants::ObstacleType obstacle) const;

public:
    Render();

    void initialize(MapSystem* mapSys, ResourceManager* resMgr,
        ConstructionSystem* constSys = nullptr,
        RandomEvent* eventSys = nullptr,
        InputHandler* inputHandler = nullptr);

    // 主渲染方法
    void draw(sf::RenderWindow& window);

    // 视图控制
    void setViewCenter(const sf::Vector2f& center);
    void zoom(float factor);
    void resetView();

    // 状态更新
    void setHoveredCell(int x, int y);
    void setSelectedCell(int x, int y);
    void toggleWorkers() { showWorkers = !showWorkers; }

    // 资源管理
    bool loadFont(const std::string& fontPath);
    bool loadTextures();
    const sf::Font& Getfont() const { return font; };
    // 调试工具
    void printRenderState() const;
};

#endif