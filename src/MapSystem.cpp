#include "MapSystem.hpp"
#include <iostream>
#include <queue>
#include <algorithm>
#include <sstream>

MapSystem::MapSystem()
    : grid(Constants::MAP_HEIGHT,
        std::vector<Cell>(Constants::MAP_WIDTH, Cell(0, 0))) {

    // 初始化网格坐标
    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            grid[y][x] = Cell(x, y);
        }
    }
}

void MapSystem::initialize() {
    // 设置起点和终点
    startPoint = sf::Vector2i(0, Constants::MAP_HEIGHT / 2);
    endPoint = sf::Vector2i(Constants::MAP_WIDTH - 1, Constants::MAP_HEIGHT / 2);

    // 标记起点和终点
    getCell(startPoint.x, startPoint.y).isStartPoint = true;
    getCell(endPoint.x, endPoint.y).isEndPoint = true;

    // 设置一些基础地形
    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            // 创建一条河流
            if (x == Constants::MAP_WIDTH / 2 && y > 2 && y < Constants::MAP_HEIGHT - 3) {
                grid[y][x].terrain = Constants::TerrainType::RIVER;
            }
            // 创建山脉区域
            else if (x > 5 && x < 10 && y > 5 && y < 10) {
                grid[y][x].terrain = Constants::TerrainType::MOUNTAIN;
            }
            // 创建沼泽区域
            else if (x > 12 && x < 18 && y > 3 && y < 8) {
                grid[y][x].terrain = Constants::TerrainType::SWAMP;
            }
        }
    }

    // 添加一些障碍物
    getCell(3, 4).obstacle = Constants::ObstacleType::BOULDER;
    getCell(7, 7).obstacle = Constants::ObstacleType::FOREST;
    getCell(15, 5).obstacle = Constants::ObstacleType::LAKE;
    getCell(8, 12).obstacle = Constants::ObstacleType::CLIFF;
    getCell(12, 10).obstacle = Constants::ObstacleType::RUINS;
}

void MapSystem::loadLevel(const std::string& levelFile) {
    // TODO: 从JSON文件加载关卡数据
    // 这里先使用随机生成的地图
    generateRandomMap();
}

void MapSystem::generateRandomMap() {
    // 简单的随机地图生成逻辑
    // 在实际实现中，这里会有更复杂的地形生成算法
    initialize(); // 暂时使用初始化地图
}

bool MapSystem::isValidPosition(int x, int y) const {
    return x >= 0 && x < Constants::MAP_WIDTH &&
        y >= 0 && y < Constants::MAP_HEIGHT;
}

bool MapSystem::isAdjacent(int x1, int y1, int x2, int y2) const {
    int dx = std::abs(x1 - x2);
    int dy = std::abs(y1 - y2);
    return (dx == 1 && dy == 0) || (dx == 0 && dy == 1);
}

Cell& MapSystem::getCell(int x, int y) {
    if (isValidPosition(x, y)) {
        return grid[y][x];
    }
    throw std::out_of_range("Invalid cell position: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
}

const Cell& MapSystem::getCell(int x, int y) const {
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
    // 不能在有遗迹的地方修路
    if (cell.obstacle == Constants::ObstacleType::RUINS) {
        return false;
    }

    // 不能重复修路
    if (cell.hasRoad) {
        return false;
    }

    // 检查是否与现有道路相邻（起点除外）
    /*if (!cell.isStartPoint && !cell.isEndPoint) {
        bool hasAdjacentRoad = false;
        if (x > 0 && grid[y][x - 1].isPassable()) hasAdjacentRoad = true;
        if (x < Constants::MAP_WIDTH - 1 && grid[y][x + 1].isPassable()) hasAdjacentRoad = true;
        if (y > 0 && grid[y - 1][x].isPassable()) hasAdjacentRoad = true;
        if (y < Constants::MAP_HEIGHT - 1 && grid[y + 1][x].isPassable()) hasAdjacentRoad = true;

        if (!hasAdjacentRoad) {
            return false;
        }
    }*/

    return true;
}

bool MapSystem::canBuildBridge(int x, int y) const {
    const Cell& cell = grid[y][x];

    // 只能在河流上建桥
    if (cell.terrain != Constants::TerrainType::RIVER) {
        return false;
    }

    // 不能重复建桥
    if (cell.hasBridge) {
        return false;
    }

    return true;
}

bool MapSystem::canBuildTunnel(int x, int y) const {
    const Cell& cell = grid[y][x];

    // 只能在山脉上建隧道
    if (cell.terrain != Constants::TerrainType::MOUNTAIN) {
        return false;
    }

    // 不能重复建隧道
    if (cell.hasTunnel) {
        return false;
    }

    // 检查隧道方向规则（简化实现）
    // 实际游戏中应该有更复杂的隧道方向检查
    return true;
}

bool MapSystem::canClearObstacle(int x, int y) const {
    const Cell& cell = grid[y][x];

    // 只能清除特定类型的障碍物
    return cell.obstacle != Constants::ObstacleType::NONE &&
        cell.obstacle != Constants::ObstacleType::RUINS;
}

bool MapSystem::canReinforce(int x, int y) const {
    const Cell& cell = grid[y][x];

    // 只能加固山脉
    if (cell.terrain != Constants::TerrainType::MOUNTAIN) {
        return false;
    }

    // 不能重复加固
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
    // 使用BFS检查起点到终点是否连通
    std::vector<std::vector<bool>> visited(
        Constants::MAP_HEIGHT,
        std::vector<bool>(Constants::MAP_WIDTH, false)
    );

    std::queue<sf::Vector2i> queue;
    queue.push(startPoint);
    visited[startPoint.y][startPoint.x] = true;

    // 四个方向：上、右、下、左
    const int dx[4] = { 0, 1, 0, -1 };
    const int dy[4] = { -1, 0, 1, 0 };

    while (!queue.empty()) {
        sf::Vector2i current = queue.front();
        queue.pop();

        // 如果到达终点，返回true
        if (current == endPoint) {
            return true;
        }

        // 检查四个方向
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
    // 检查是否有直接连接起点和终点的路径
    return isPathConnected();
}

std::vector<sf::Vector2i> MapSystem::findPath() const {
    // 使用BFS找到从起点到终点的路径
    std::vector<std::vector<bool>> visited(
        Constants::MAP_HEIGHT,
        std::vector<bool>(Constants::MAP_WIDTH, false)
    );

    std::vector<std::vector<sf::Vector2i>> parent(
        Constants::MAP_HEIGHT,
        std::vector<sf::Vector2i>(Constants::MAP_WIDTH, sf::Vector2i(-1, -1))
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
            // 重建路径
            std::vector<sf::Vector2i> path;
            sf::Vector2i step = endPoint;

            while (step != startPoint) {
                path.push_back(step);
                step = parent[step.y][step.x];
            }
            path.push_back(startPoint);
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (int i = 0; i < 4; ++i) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (isValidPosition(nx, ny) && !visited[ny][nx]) {
                const Cell& neighbor = grid[ny][nx];
                if (neighbor.isPassable()) {
                    visited[ny][nx] = true;
                    parent[ny][nx] = current;
                    queue.push(sf::Vector2i(nx, ny));
                }
            }
        }
    }

    return {}; // 没有找到路径
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
    // 简单的反序列化实现
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