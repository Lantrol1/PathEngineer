#include "ResourceManager.hpp"
#include <iostream>
#include <sstream>

ResourceManager::ResourceManager() {
    initialize();
}

void ResourceManager::initialize() {
    currentState = ResourceState();

    // 初始化回调为空函数
    onBudgetChange = [](int, const std::string&) {};
    onWorkersChange = [](int, int) {};
    onEnvironmentChange = [](int) {};
    onTurnChange = [](int) {};
}

void ResourceManager::reset() {
    initialize();
}

bool ResourceManager::canAfford(int cost) const {
    return currentState.budget >= cost;
}

bool ResourceManager::spendBudget(int amount) {
    if (canAfford(amount)) {
        int oldBudget = currentState.budget;
        currentState.budget -= amount;

        // 触发回调
        std::stringstream reason;
        reason << "支出: " << amount;
        onBudgetChange(currentState.budget, reason.str());

        return true;
    }
    return false;
}

void ResourceManager::addBudget(int amount) {
    int oldBudget = currentState.budget;
    currentState.budget += amount;

    // 触发回调
    std::stringstream reason;
    reason << "收入: " << amount;
    onBudgetChange(currentState.budget, reason.str());
}

void ResourceManager::applyPenalty(int amount, const std::string& reason) {
    spendBudget(amount);
    updateEnvironment(-5); // 罚款也会影响环境评分

    std::cout << "罚款: " << amount << " - " << reason << std::endl;
}

bool ResourceManager::canAssignWorkers(int count) const {
    return currentState.availableWorkers >= count;
}

bool ResourceManager::assignWorkers(int count) {
    if (canAssignWorkers(count)) {
        int oldAvailable = currentState.availableWorkers;
        currentState.availableWorkers -= count;

        // 触发回调
        onWorkersChange(currentState.availableWorkers, currentState.totalWorkers);

        return true;
    }
    return false;
}

bool ResourceManager::assignWorkersToLocation(int x, int y, int count) {
    if (assignWorkers(count)) {
        currentState.workerAssignments[{x, y}] += count;
        return true;
    }
    return false;
}

void ResourceManager::freeWorkers(int count) {
    int oldAvailable = currentState.availableWorkers;
    currentState.availableWorkers += count;

    if (currentState.availableWorkers > currentState.totalWorkers) {
        currentState.availableWorkers = currentState.totalWorkers;
    }

    // 触发回调
    if (oldAvailable != currentState.availableWorkers) {
        onWorkersChange(currentState.availableWorkers, currentState.totalWorkers);
    }
}

void ResourceManager::freeWorkersFromLocation(int x, int y) {
    auto it = currentState.workerAssignments.find({ x, y });
    if (it != currentState.workerAssignments.end()) {
        freeWorkers(it->second);
        currentState.workerAssignments.erase(it);
    }
}

void ResourceManager::permanentlyReduceWorkers(int count) {
    if (count > 0) {
        int oldTotal = currentState.totalWorkers;
        currentState.totalWorkers -= count;

        if (currentState.totalWorkers < 0) {
            currentState.totalWorkers = 0;
        }

        // 同时减少可用工人，但不能少于0
        if (currentState.availableWorkers > currentState.totalWorkers) {
            currentState.availableWorkers = currentState.totalWorkers;
        }

        // 触发回调
        onWorkersChange(currentState.availableWorkers, currentState.totalWorkers);

        std::cout << "永久减少工人: " << count << "名" << std::endl;
    }
}

void ResourceManager::increaseWorkers(int count) {
    if (count > 0) {
        int oldTotal = currentState.totalWorkers;
        currentState.totalWorkers += count;
        currentState.availableWorkers += count;
        onWorkersChange(currentState.availableWorkers, currentState.totalWorkers);
        std::cout << "增加工人: " << count << "名" << std::endl;
    }
}

void ResourceManager::updateEnvironment(int change) {
    int oldScore = currentState.environmentScore;
    currentState.environmentScore += change;
    if (currentState.environmentScore < 0) {
        currentState.environmentScore = 0;
    }
    else if (currentState.environmentScore > 100) {
        currentState.environmentScore = 100;
    }
    if (oldScore != currentState.environmentScore) {
        onEnvironmentChange(currentState.environmentScore);
    }
}

void ResourceManager::applyEnvironmentalPenalty(int severity) {
    int penalty = severity * 200; // 基础罚款乘以严重程度
    applyPenalty(penalty, "环境破坏罚款");

    std::cout << "环境破坏! 罚款: " << penalty << std::endl;
}

void ResourceManager::startTurn() {
    std::cout << "回合 " << currentState.currentTurn << " 开始" << std::endl;
    onTurnChange(currentState.currentTurn);
}

void ResourceManager::processTurn() {
    int laborCost = calculateLaborCostForTurn();
    if (laborCost > 0) {
        if (!spendBudget(laborCost)) {
            std::cout << "资金不足支付工资! 工人罢工!" << std::endl;
            int totalAssigned = 0;
            for (const auto& assignment : currentState.workerAssignments) {
                totalAssigned += assignment.second;
            }
            currentState.workerAssignments.clear();
            currentState.availableWorkers = currentState.totalWorkers;
            onWorkersChange(currentState.availableWorkers, currentState.totalWorkers);
        }
        else {
            std::cout << "支付劳动力成本: " << laborCost << std::endl;
        }
    }
}

void ResourceManager::endTurn() {
    currentState.currentTurn++;
    if (isGameOver()) {
        std::cout << "游戏结束!" << std::endl;
    }
    onTurnChange(currentState.currentTurn);
}

bool ResourceManager::isGameOver() const {
    return isTimeOut() || currentState.budget <= 0 || currentState.totalWorkers <= 0;
}

bool ResourceManager::isTimeOut() const {
    return currentState.currentTurn > currentState.totalTurns;
}

LaborCost ResourceManager::calculateLaborCost(int x, int y, Constants::ConstructionType type, int workerCount) const {
    LaborCost cost;
    switch (type) {
    case Constants::ConstructionType::BUILD_TUNNEL:
        cost.riskModifier += 5;
        cost.terrainModifier += 3; 
        break;
    case Constants::ConstructionType::BUILD_BRIDGE:
        cost.riskModifier += 3;
        cost.terrainModifier += 2;
        break;
    case Constants::ConstructionType::REINFORCE_MOUNTAIN:
        cost.riskModifier += 2;
        cost.terrainModifier += 2;
        break;
    default:
        break;
    }

    // 天气影响（简化实现，实际应该从RandomEvent获取）
    // cost.weatherModifier = getWeatherModifier();

    return cost;
}
int ResourceManager::calculateLaborCostForTurn() const {
    int totalCost = 0;

    for (const auto& assignment : currentState.workerAssignments) {
        int x = assignment.first.first;
        int y = assignment.first.second;
        int workerCount = assignment.second;

        // 简化计算：每个工人基础成本10，根据位置和任务类型可能有调整
        // 实际实现中应该根据具体任务类型计算
        LaborCost cost = calculateLaborCost(x, y, Constants::ConstructionType::BUILD_ROAD, workerCount);
        totalCost += workerCount * cost.getTotalCost();
    }
    return totalCost;
}

int ResourceManager::calculateConstructionCost(int x, int y, Constants::ConstructionType type) const {
    // 基础成本计算
    int baseCost = 0;

    switch (type) {
    case Constants::ConstructionType::BUILD_ROAD:
        baseCost = Constants::COST_ROAD_PLAIN;
        break;
    case Constants::ConstructionType::BUILD_BRIDGE:
        baseCost = Constants::COST_BRIDGE;
        break;
    case Constants::ConstructionType::BUILD_TUNNEL:
        baseCost = Constants::COST_TUNNEL;
        break;
    case Constants::ConstructionType::CLEAR_OBSTACLE:
        baseCost = Constants::COST_CLEAR_BOULDER; // 简化处理
        break;
    case Constants::ConstructionType::REINFORCE_MOUNTAIN:
        baseCost = Constants::COST_REINFORCE;
        break;
    default:
        break;
    }
    return baseCost;
}

void ResourceManager::setBudgetChangeCallback(std::function<void(int, const std::string&)> callback) {
    onBudgetChange = callback;
}

void ResourceManager::setWorkersChangeCallback(std::function<void(int, int)> callback) {
    onWorkersChange = callback;
}

void ResourceManager::setEnvironmentChangeCallback(std::function<void(int)> callback) {
    onEnvironmentChange = callback;
}

void ResourceManager::setTurnChangeCallback(std::function<void(int)> callback) {
    onTurnChange = callback;
}