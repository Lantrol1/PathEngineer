#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP
#include<SFML/Graphics.hpp>
#include <string>

namespace Constants {
    // 窗口设置
    const int WINDOW_WIDTH = 1920;
    const int WINDOW_HEIGHT = 1080;
    const std::string WINDOW_TITLE = "Path Engineer";

    // 地图设置
    const int GRID_SIZE = 50;
    const int MAP_WIDTH = 20;
    const int MAP_HEIGHT = 20;

    // 地形类型
    enum class TerrainType {
        PLAIN,
        RIVER,
        MOUNTAIN,
        SWAMP
    };

    // 障碍物类型
    enum class ObstacleType {
        NONE,
        BOULDER,
        FOREST,
        LAKE,
        CLIFF,
        RUINS
    };

    // 建设操作类型
    enum class ConstructionType {
        NONE,
        BUILD_ROAD,
        BUILD_BRIDGE,
        BUILD_TUNNEL,
        CLEAR_OBSTACLE,
        REINFORCE_MOUNTAIN
    };

    // 资源常量
    const int INITIAL_BUDGET = 10000;
    const int INITIAL_WORKERS = 10;
    const int MAX_TURNS = 50;

    // 建设成本
    const int COST_ROAD_PLAIN = 100;
    const int COST_ROAD_SWAMP = 150;
    const int COST_BRIDGE = 300;
    const int COST_TUNNEL = 500;
    const int COST_CLEAR_BOULDER = 200;
    const int COST_CLEAR_FOREST = 100;
    const int COST_REINFORCE = 150;

    // 颜色定义 (替代贴图)
    const sf::Color COLOR_PLAIN = sf::Color(100, 200, 100);
    const sf::Color COLOR_RIVER = sf::Color(100, 100, 255);
    const sf::Color COLOR_MOUNTAIN = sf::Color(150, 150, 150);
    const sf::Color COLOR_SWAMP = sf::Color(150, 200, 150);
    const sf::Color COLOR_ROAD = sf::Color(100, 100, 100);
    const sf::Color COLOR_BRIDGE = sf::Color(200, 200, 100);
    const sf::Color COLOR_TUNNEL = sf::Color(100, 50, 50);
}

#endif