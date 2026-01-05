#ifndef MAPSYSTEM_HPP
#define MAPSYSTEM_HPP

#include <vector>
#include <memory>
#include <stdexcept>
#include <SFML/System/Vector2.hpp>
#include "Constants.hpp"

// 前向声明
class ResourceManager;

struct Cell {
    Constants::TerrainType terrain;
    Constants::ObstacleType obstacle;
    bool hasRoad;
    bool hasBridge;
    bool hasTunnel;
    bool isReinforced;
    bool isStartPoint;
    bool isEndPoint;
    int x, y;

    // 劳动力分配
    int assignedWorkers;

    Cell(int x, int y)
        : terrain(Constants::TerrainType::PLAIN),
        obstacle(Constants::ObstacleType::NONE),
        hasRoad(false),
        hasBridge(false),
        hasTunnel(false),
        isReinforced(false),
        isStartPoint(false),
        isEndPoint(false),
        x(x), y(y),
        assignedWorkers(0) {
    }

    // 检查单元格是否可通过（有道路、桥梁或隧道）
    bool isPassable() const {
        if (obstacle == Constants::ObstacleType::RUINS) {
            return false; // 遗迹不可通过
        }
        return hasRoad || hasBridge || hasTunnel;
    }

    // 检查是否正在施工
    bool isUnderConstruction() const {
        return assignedWorkers > 0;
    }

    // 重置劳动力分配
    void resetWorkers() {
        assignedWorkers = 0;
    }
};

class MapSystem {
private:
    std::vector<std::vector<Cell>> grid;
    sf::Vector2i startPoint;
    sf::Vector2i endPoint;

public:
    MapSystem();

    void initialize();
    void loadLevel(const std::string& levelFile);
    void generateRandomMap();

    // 单元格访问
    Cell& getCell(int x, int y);
    const Cell& getCell(int x, int y) const;

    // 位置验证
    bool isValidPosition(int x, int y) const;

    // 建设可行性检查
    bool canBuildAt(int x, int y, Constants::ConstructionType construction) const;
    bool canPlaceRoad(int x, int y) const;
    bool canBuildBridge(int x, int y) const;
    bool canBuildTunnel(int x, int y) const;
    bool canClearObstacle(int x, int y) const;
    bool canReinforce(int x, int y) const;

    // 地形操作
    void setTerrain(int x, int y, Constants::TerrainType terrain);
    void setObstacle(int x, int y, Constants::ObstacleType obstacle);

    // 建设操作
    void buildRoad(int x, int y);
    void buildBridge(int x, int y);
    void buildTunnel(int x, int y);
    void reinforceMountain(int x, int y);
    void clearObstacle(int x, int y);

    // 劳动力管理
    bool assignWorkers(int x, int y, int count);
    void freeWorkers(int x, int y);
    int getAssignedWorkers(int x, int y) const;

    // 路径检查
    bool isPathConnected() const;
    bool hasDirectConnection() const;

    // 获取地形建设成本
    int getTerrainBuildCost(int x, int y, Constants::ConstructionType type) const;
    int getObstacleClearCost(int x, int y) const;

    // 获取ter
    //Getters
    sf::Vector2i getStartPoint() const { return startPoint; }
    sf::Vector2i getEndPoint() const { return endPoint; }
    const std::vector<std::vector<Cell>>& getGrid() const { return grid; }
    int getWidth() const { return Constants::MAP_WIDTH; }
    int getHeight() const { return Constants::MAP_HEIGHT; }

    // 序列化（用于保存/加载）
    std::string serialize() const;
    void deserialize(const std::string& data);
};

#endif