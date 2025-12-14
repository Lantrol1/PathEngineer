#include "ConstructionSystem.hpp"
#include "RandomEvent.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

ConstructionSystem::ConstructionSystem()
    : mapSystem(nullptr), resourceManager(nullptr), randomEvent(nullptr) {

    // 初始化回调为空函数
    onTaskStarted = [](const ConstructionTask&) {};
    onTaskProgress = [](const ConstructionTask&) {};
    onTaskCompleted = [](const ConstructionTask&) {};
    onConstructionError = [](const std::string&) {};
}

void ConstructionSystem::initialize(MapSystem* mapSys, ResourceManager* resMgr, RandomEvent* eventSys) {
    mapSystem = mapSys;
    resourceManager = resMgr;
    randomEvent = eventSys;
}

bool ConstructionSystem::validateConstruction(int x, int y, Constants::ConstructionType type) const {
    if (!mapSystem || !resourceManager) {
        onConstructionError("系统未初始化");
        return false;
    }

    // 检查位置有效性
    if (!mapSystem->isValidPosition(x, y)) {
        onConstructionError("无效的位置");
        return false;
    }

    // 检查建设可行性
    if (!mapSystem->canBuildAt(x, y, type)) {
        onConstructionError("无法在此位置进行建设");
        return false;
    }

    // 检查是否已有任务
    if (hasActiveTask(x, y)) {
        onConstructionError("该位置已有建设任务");
        return false;
    }

    // 检查资金是否足够
    int materialCost = calculateMaterialCost(x, y, type);
    if (!resourceManager->canAfford(materialCost)) {
        onConstructionError("资金不足");
        return false;
    }

    return true;
}

int ConstructionSystem::calculateMaterialCost(int x, int y, Constants::ConstructionType type) const {
    if (!mapSystem) return 0;

    int cost = 0;

    switch (type) {
    case Constants::ConstructionType::BUILD_ROAD:
        cost = mapSystem->getTerrainBuildCost(x, y, type);
        break;

    case Constants::ConstructionType::BUILD_BRIDGE:
        cost = Constants::COST_BRIDGE;
        break;

    case Constants::ConstructionType::BUILD_TUNNEL:
        cost = Constants::COST_TUNNEL;
        break;

    case Constants::ConstructionType::CLEAR_OBSTACLE:
        cost = mapSystem->getObstacleClearCost(x, y);
        break;

    case Constants::ConstructionType::REINFORCE_MOUNTAIN:
        cost = Constants::COST_REINFORCE;
        break;

    default:
        cost = 0;
        break;
    }

    return cost;
}

bool ConstructionSystem::startConstruction(int x, int y, Constants::ConstructionType type) {
    // 验证建设可行性
    if (!validateConstruction(x, y, type)) {
        return false;
    }

    // 计算并支付材料费
    int materialCost = calculateMaterialCost(x, y, type);
    if (!resourceManager->spendBudget(materialCost)) {
        onConstructionError("支付材料费失败");
        return false;
    }

    // 创建建设任务
    activeTasks.emplace_back(std::make_unique<ConstructionTask>(x, y, type));
    ConstructionTask* newTask = activeTasks.back().get();
    taskLookup[{x, y}] = newTask;

    // 触发回调
    onTaskStarted(*newTask);

    std::cout << "开始建设: " << newTask->description
        << " 在位置 (" << x << ", " << y << ")"
        << " 材料费: " << materialCost << std::endl;

    return true;
}

bool ConstructionSystem::assignWorkersToTask(int x, int y, int workerCount) {
    if (workerCount <= 0) {
        return false;
    }

    ConstructionTask* task = findTask(x, y);
    if (!task) {
        onConstructionError("未找到指定的建设任务");
        return false;
    }

    // 检查是否有足够可用工人
    if (!resourceManager->canAssignWorkers(workerCount)) {
        onConstructionError("可用工人不足");
        return false;
    }

    // 分配工人到任务
    if (resourceManager->assignWorkersToLocation(x, y, workerCount)) {
        task->assignedWorkers += workerCount;

        std::cout << "分配 " << workerCount << " 名工人到 "
            << task->description << " 在 (" << x << ", " << y << ")" << std::endl;

        return true;
    }

    return false;
}

bool ConstructionSystem::reassignWorkers(int fromX, int fromY, int toX, int toY, int workerCount) {
    ConstructionTask* fromTask = findTask(fromX, fromY);
    if (!fromTask || fromTask->assignedWorkers < workerCount) {
        onConstructionError("源任务工人不足");
        return false;
    }

    // 先释放源任务的工人
    resourceManager->freeWorkersFromLocation(fromX, fromY);
    fromTask->assignedWorkers -= workerCount;

    // 分配到新任务
    return assignWorkersToTask(toX, toY, workerCount);
}

void ConstructionSystem::cancelConstruction(int x, int y) {
    ConstructionTask* task = findTask(x, y);
    if (!task) return;

    // 释放分配的工人
    if (task->assignedWorkers > 0) {
        resourceManager->freeWorkersFromLocation(x, y);
    }

    // 从查找表中移除
    taskLookup.erase({ x, y });

    // 从活动任务列表中移除
    auto it = std::find_if(activeTasks.begin(), activeTasks.end(),
        [x, y](const std::unique_ptr<ConstructionTask>& t) {
            return t->x == x && t->y == y;
        });

    if (it != activeTasks.end()) {
        activeTasks.erase(it);
    }

    std::cout << "取消建设任务在位置 (" << x << ", " << y << ")" << std::endl;
}

void ConstructionSystem::processTurn() {
    if (!mapSystem || !resourceManager) return;

    // 处理每个任务的进度
    for (auto it = activeTasks.begin(); it != activeTasks.end(); ) {
        ConstructionTask& task = **it;

        if (task.isCompleted) {
            // 任务已完成，执行建设操作
            executeConstruction(task);

            // 释放工人
            if (task.assignedWorkers > 0) {
                resourceManager->freeWorkersFromLocation(task.x, task.y);
            }

            // 触发完成回调
            onTaskCompleted(task);

            // 从查找表中移除
            taskLookup.erase({ task.x, task.y });

            // 从活动任务列表中移除
            it = activeTasks.erase(it);

            std::cout << "完成建设: " << task.description
                << " 在位置 (" << task.x << ", " << task.y << ")" << std::endl;
        }
        else {
            // 处理任务进度
            if (task.assignedWorkers > 0) {
                int workPoints = task.assignedWorkers; // 每个工人贡献1个工作点

                // 考虑地形对工作效率的影响
                const Cell& cell = mapSystem->getCell(task.x, task.y);
                switch (cell.terrain) {
                case Constants::TerrainType::SWAMP:
                    workPoints = static_cast<int>(workPoints * 0.8f); // 沼泽效率降低
                    break;
                case Constants::TerrainType::MOUNTAIN:
                    if (task.type != Constants::ConstructionType::BUILD_TUNNEL) {
                        workPoints = static_cast<int>(workPoints * 0.7f); // 山地效率降低
                    }
                    break;
                default:
                    break;
                }

                task.addWorkPoints(workPoints);

                // 触发进度回调
                if (workPoints > 0) {
                    onTaskProgress(task);
                }

                // 检查施工安全（如果有随机事件系统）
                if (randomEvent && task.assignedWorkers > 0) {
                    // 这里可以添加安全检查逻辑
                    // 例如：randomEvent->checkConstructionSafety(task.x, task.y, task.assignedWorkers);
                }
            }

            ++it;
        }
    }
}

void ConstructionSystem::executeConstruction(const ConstructionTask& task) {
    if (!mapSystem) return;

    switch (task.type) {
    case Constants::ConstructionType::BUILD_ROAD:
        mapSystem->buildRoad(task.x, task.y);
        break;

    case Constants::ConstructionType::BUILD_BRIDGE:
        mapSystem->buildBridge(task.x, task.y);
        break;

    case Constants::ConstructionType::BUILD_TUNNEL:
        mapSystem->buildTunnel(task.x, task.y);
        break;

    case Constants::ConstructionType::CLEAR_OBSTACLE:
        mapSystem->clearObstacle(task.x, task.y);
        break;

    case Constants::ConstructionType::REINFORCE_MOUNTAIN:
        mapSystem->reinforceMountain(task.x, task.y);
        break;

    default:
        break;
    }
}

ConstructionTask* ConstructionSystem::findTask(int x, int y) {
    auto it = taskLookup.find({ x, y });
    if (it != taskLookup.end()) {
        return it->second;
    }
    return nullptr;
}

bool ConstructionSystem::hasActiveTask(int x, int y) const {
    return taskLookup.find({ x, y }) != taskLookup.end();
}

const ConstructionTask* ConstructionSystem::getTask(int x, int y) const {
    auto it = taskLookup.find({ x, y });
    if (it != taskLookup.end()) {
        return it->second;
    }
    return nullptr;
}

bool ConstructionSystem::isPathComplete() const {
    if (!mapSystem) return false;
    return mapSystem->isPathConnected();
}

float ConstructionSystem::getOverallProgress() const {
    if (activeTasks.empty()) return 0.0f;

    float totalProgress = 0.0f;
    for (const auto& task : activeTasks) {
        totalProgress += task->getProgress();
    }

    return totalProgress / activeTasks.size();
}

int ConstructionSystem::getEstimatedCost(int x, int y, Constants::ConstructionType type) const {
    return calculateMaterialCost(x, y, type);
}

int ConstructionSystem::getRemainingWorkPoints(int x, int y) const {
    const ConstructionTask* task = getTask(x, y);
    if (!task) return 0;

    return task->requiredWorkPoints - task->completedWorkPoints;
}

void ConstructionSystem::setTaskStartedCallback(std::function<void(const ConstructionTask&)> callback) {
    onTaskStarted = callback;
}

void ConstructionSystem::setTaskProgressCallback(std::function<void(const ConstructionTask&)> callback) {
    onTaskProgress = callback;
}

void ConstructionSystem::setTaskCompletedCallback(std::function<void(const ConstructionTask&)> callback) {
    onTaskCompleted = callback;
}

void ConstructionSystem::setConstructionErrorCallback(std::function<void(const std::string&)> callback) {
    onConstructionError = callback;
}

void ConstructionSystem::printActiveTasks() const {
    std::cout << "=== 活跃建设任务 ===" << std::endl;
    for (const auto& task : activeTasks) {
        std::cout << task->description << " 在 (" << task->x << ", " << task->y << ")"
            << " 进度: " << (task->getProgress() * 100) << "%"
            << " 工人: " << task->assignedWorkers
            << " 完成: " << (task->isCompleted ? "是" : "否") << std::endl;
    }
    std::cout << "===================" << std::endl;
}