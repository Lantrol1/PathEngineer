#include "MapSystem.hpp"
#include <iostream>
#include <queue>
#include <algorithm>
#include <sstream>

MapSystem::MapSystem():grid(Constants::MAP_HEIGHT,std::vector<Cell>(Constants::MAP_WIDTH, Cell(0, 0))) {
    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            grid[y][x] = Cell(x, y);
        }
    }
}

void MapSystem::initialize() {
    startPoint = sf::Vector2i(0, Constants::MAP_HEIGHT / 2);
    endPoint = sf::Vector2i(Constants::MAP_WIDTH - 1, Constants::MAP_HEIGHT / 2);
    getCell(startPoint.x, startPoint.y).isStartPoint = true;
    getCell(endPoint.x, endPoint.y).isEndPoint = true;
    std::cout << startPoint.x << " " << startPoint.y << std::endl;
    std::cout << endPoint.x << " " << endPoint.y << std::endl;
    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            if (x == Constants::MAP_WIDTH / 2 && y > 2 && y < Constants::MAP_HEIGHT - 3) {
                grid[y][x].terrain = Constants::TerrainType::RIVER;
            }
            else if (x > 5 && x < 10 && y > 5 && y < 10) {
                grid[y][x].terrain = Constants::TerrainType::MOUNTAIN;
            }
            else if (x > 12 && x < 18 && y > 3 && y < 8) {
                grid[y][x].terrain = Constants::TerrainType::SWAMP;
            }
        }
    }
    getCell(3, 4).obstacle = Constants::ObstacleType::BOULDER;
    getCell(7, 7).obstacle = Constants::ObstacleType::FOREST;
    getCell(15, 5).obstacle = Constants::ObstacleType::LAKE;
    getCell(8, 12).obstacle = Constants::ObstacleType::CLIFF;
    getCell(12, 10).obstacle = Constants::ObstacleType::RUINS;
}

void MapSystem::loadLevel(const std::string& levelFile) {
    generateRandomMap();
}

void MapSystem::generateRandomMap() {
    initialize();
}

bool MapSystem::isValidPosition(int x, int y) const {
    return x >= 0 && x < Constants::MAP_WIDTH &&
        y >= 0 && y < Constants::MAP_HEIGHT;
}

Cell& MapSystem::getCell(int x, int y) {
    if (isValidPosition(x, y)) {
        return grid[y][x];
    }
    throw std::out_of_range("Invalid cell position: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}


bool MapSystem::canBuildAt(int x, int y, Constants::ConstructionType construction) const {
    if (!isValidPosition(x, y)) return false;

    const Cell& cell = grid[y][x];

    switch (construction) {
    case Constants::ConstructionType::BUILD_ROAD:
        return canPlaceRoad(x, y);

    case Constants::ConstructionType::BUILD_BRIDGE:
        return canBuildBridge(x, y);

    case Constants::ConstructionType::BUILD_TUNNEL:
        return canBuildTunnel(x, y);

    case Constants::ConstructionType::CLEAR_OBSTACLE:
        return canClearObstacle(x, y);

    case Constants::ConstructionType::REINFORCE_MOUNTAIN:
        return canReinforce(x, y);

    default:
        return false;
    }
}

bool MapSystem::canPlaceRoad(int x, int y) const {
    const Cell& cell = grid[y][x];
    if (cell.terrain == Constants::TerrainType::RIVER) {
        return false;
    }
    if (cell.terrain == Constants::TerrainType::MOUNTAIN) {
        return false;
    }
    if (cell.obstacle != Constants::ObstacleType::NONE) {
        return false;
    }
    if (cell.hasRoad) {
        return false;
    }
    return true;
}

bool MapSystem::canBuildBridge(int x, int y) const {
    const Cell& cell = grid[y][x];
    if (cell.terrain != Constants::TerrainType::RIVER) {
        return false;
    }
    if (cell.hasBridge) {
        return false;
    }
    return true;
}

bool MapSystem::canBuildTunnel(int x, int y) const {
    const Cell& cell = grid[y][x];
    if (cell.terrain != Constants::TerrainType::MOUNTAIN) {
        return false;
    }
    if (cell.hasTunnel) {
        return false;
    }
    return true;
}

bool MapSystem::canClearObstacle(int x, int y) const {
    const Cell& cell = grid[y][x];
    return cell.obstacle != Constants::ObstacleType::NONE &&
        cell.obstacle != Constants::ObstacleType::RUINS;
}

bool MapSystem::canReinforce(int x, int y) const {
    const Cell& cell = grid[y][x];
    if (cell.terrain != Constants::TerrainType::MOUNTAIN) {
        return false;
    }
    if (cell.isReinforced) {
        return false;
    }

    return true;
}

void MapSystem::setTerrain(int x, int y, Constants::TerrainType terrain) {
    if (isValidPosition(x, y)) {
        grid[y][x].terrain = terrain;
    }
}

void MapSystem::setObstacle(int x, int y, Constants::ObstacleType obstacle) {
    if (isValidPosition(x, y)) {
        grid[y][x].obstacle = obstacle;
    }
}

void MapSystem::buildRoad(int x, int y) {
    if (canPlaceRoad(x, y)) {
        grid[y][x].hasRoad = true;
        // 修路时会自动清除一些小型障碍物
        if (grid[y][x].obstacle == Constants::ObstacleType::BOULDER ||
            grid[y][x].obstacle == Constants::ObstacleType::FOREST) {
            grid[y][x].obstacle = Constants::ObstacleType::NONE;
        }
    }
}

void MapSystem::buildBridge(int x, int y) {
    if (canBuildBridge(x, y)) {
        grid[y][x].hasBridge = true;
        grid[y][x].hasRoad = true; // 桥上也有路
    }
}

void MapSystem::buildTunnel(int x, int y) {
    if (canBuildTunnel(x, y)) {
        grid[y][x].hasTunnel = true;
        grid[y][x].hasRoad = true; // 隧道内也有路
    }
}

void MapSystem::reinforceMountain(int x, int y) {
    if (canReinforce(x, y)) {
        grid[y][x].isReinforced = true;
    }
}

void MapSystem::clearObstacle(int x, int y) {
    if (canClearObstacle(x, y)) {
        grid[y][x].obstacle = Constants::ObstacleType::NONE;
    }
}

bool MapSystem::assignWorkers(int x, int y, int count) {
    if (isValidPosition(x, y) && count > 0) {
        grid[y][x].assignedWorkers += count;
        return true;
    }
    return false;
}

void MapSystem::freeWorkers(int x, int y) {
    if (isValidPosition(x, y)) {
        grid[y][x].assignedWorkers = 0;
    }
}

int MapSystem::getAssignedWorkers(int x, int y) const {
    if (isValidPosition(x, y)) {
        return grid[y][x].assignedWorkers;
    }
    return 0;
}

bool MapSystem::isPathConnected() const {
    std::vector<std::vector<bool>> visited(
        Constants::MAP_HEIGHT,
        std::vector<bool>(Constants::MAP_WIDTH, false)
    );
    std::queue<sf::Vector2i> queue;
    queue.push(startPoint);
    visited[startPoint.y][startPoint.x] = true;
    const int dx[4] = { 0, 1, 0, -1 };
    const int dy[4] = { -1, 0, 1, 0 };

    while (!queue.empty()) {
        sf::Vector2i current = queue.front();
        queue.pop();
        if (current == endPoint) {
            return true;
        }
        for (int i = 0; i < 4; ++i) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (isValidPosition(nx, ny) && !visited[ny][nx]) {
                const Cell& neighbor = grid[ny][nx];
                if (neighbor.isPassable()) {
                    visited[ny][nx] = true;
                    queue.push(sf::Vector2i(nx, ny));
                }
            }
        }
    }
    return false;
}

bool MapSystem::hasDirectConnection() const {
    return isPathConnected();
}

int MapSystem::getTerrainBuildCost(int x, int y, Constants::ConstructionType type) const {
    if (!isValidPosition(x, y)) return 0;

    const Cell& cell = grid[y][x];
    int baseCost = 0;

    switch (type) {
    case Constants::ConstructionType::BUILD_ROAD:
        switch (cell.terrain) {
        case Constants::TerrainType::PLAIN:
            baseCost = Constants::COST_ROAD_PLAIN;
            break;
        case Constants::TerrainType::SWAMP:
            baseCost = Constants::COST_ROAD_SWAMP;
            break;
        default:
            baseCost = Constants::COST_ROAD_PLAIN;
            break;
        }
        break;

    case Constants::ConstructionType::BUILD_BRIDGE:
        baseCost = Constants::COST_BRIDGE;
        break;

    case Constants::ConstructionType::BUILD_TUNNEL:
        baseCost = Constants::COST_TUNNEL;
        break;

    case Constants::ConstructionType::REINFORCE_MOUNTAIN:
        baseCost = Constants::COST_REINFORCE;
        break;

    default:
        baseCost = 0;
        break;
    }

    return baseCost;
}

int MapSystem::getObstacleClearCost(int x, int y) const {
    if (!isValidPosition(x, y)) return 0;

    const Cell& cell = grid[y][x];

    switch (cell.obstacle) {
    case Constants::ObstacleType::BOULDER:
        return Constants::COST_CLEAR_BOULDER;
    case Constants::ObstacleType::FOREST:
        return Constants::COST_CLEAR_FOREST;
    default:
        return 0;
    }
}

std::string MapSystem::serialize() const {
    // 简单的序列化实现
    std::stringstream ss;
    ss << startPoint.x << "," << startPoint.y << "|"
        << endPoint.x << "," << endPoint.y << "|";

    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            const Cell& cell = grid[y][x];
            ss << static_cast<int>(cell.terrain) << ","
                << static_cast<int>(cell.obstacle) << ","
                << cell.hasRoad << ","
                << cell.hasBridge << ","
                << cell.hasTunnel << ","
                << cell.isReinforced << ";";
        }
    }

    return ss.str();
}

void MapSystem::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string token;

    // 解析起点
    std::getline(ss, token, '|');
    size_t commaPos = token.find(',');
    startPoint.x = std::stoi(token.substr(0, commaPos));
    startPoint.y = std::stoi(token.substr(commaPos + 1));

    // 解析终点
    std::getline(ss, token, '|');
    commaPos = token.find(',');
    endPoint.x = std::stoi(token.substr(0, commaPos));
    endPoint.y = std::stoi(token.substr(commaPos + 1));

    // 标记起点和终点
    getCell(startPoint.x, startPoint.y).isStartPoint = true;
    getCell(endPoint.x, endPoint.y).isEndPoint = true;

    // 解析单元格数据
    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            std::getline(ss, token, ';');
            std::stringstream cellStream(token);
            std::string cellToken;

            std::vector<std::string> cellData;
            while (std::getline(cellStream, cellToken, ',')) {
                cellData.push_back(cellToken);
            }

            if (cellData.size() >= 6) {
                Cell& cell = grid[y][x];
                cell.terrain = static_cast<Constants::TerrainType>(std::stoi(cellData[0]));
                cell.obstacle = static_cast<Constants::ObstacleType>(std::stoi(cellData[1]));
                cell.hasRoad = std::stoi(cellData[2]);
                cell.hasBridge = std::stoi(cellData[3]);
                cell.hasTunnel = std::stoi(cellData[4]);
                cell.isReinforced = std::stoi(cellData[5]);
            }
        }
    }
}