#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <map>
#include <functional>
#include "Constants.hpp"

// 前向声明
class MapSystem;
class ConstructionSystem;

// 资源状态结构体
struct ResourceState {
    int budget;
    int totalWorkers;
    int availableWorkers;
    int environmentScore;
    int currentTurn;
    int totalTurns;

    // 劳动力分配记录 (位置 -> 工人数量)
    std::map<std::pair<int, int>, int> workerAssignments;

    ResourceState()
        : budget(Constants::INITIAL_BUDGET),
        totalWorkers(Constants::INITIAL_WORKERS),
        availableWorkers(Constants::INITIAL_WORKERS),
        environmentScore(100),
        currentTurn(1),
        totalTurns(Constants::MAX_TURNS) {
    }
};

// 劳动力成本结构体
struct LaborCost {
    int baseCost;           // 基础劳务费
    int terrainModifier;    // 地形调整
    int weatherModifier;    // 天气调整
    int riskModifier;       // 风险调整

    LaborCost() : baseCost(10), terrainModifier(0), weatherModifier(0), riskModifier(0) {}

    int getTotalCost() const {
        return baseCost + terrainModifier + weatherModifier + riskModifier;
    }
};

class ResourceManager {
private:
    ResourceState currentState;

    // 回调函数
    std::function<void(int, const std::string&)> onBudgetChange;
    std::function<void(int, int)> onWorkersChange;
    std::function<void(int)> onEnvironmentChange;
    std::function<void(int)> onTurnChange;

    // 劳动力成本计算
    LaborCost calculateLaborCost(int x, int y, Constants::ConstructionType type, int workerCount) const;

public:
    ResourceManager();

    void initialize();
    void reset();

    // 预算管理
    bool canAfford(int cost) const;
    bool spendBudget(int amount);
    void addBudget(int amount);
    void applyPenalty(int amount, const std::string& reason = "");

    // 劳动力管理
    bool canAssignWorkers(int count) const;
    bool assignWorkers(int count);
    bool assignWorkersToLocation(int x, int y, int count);
    void freeWorkers(int count);
    void freeWorkersFromLocation(int x, int y);
    void permanentlyReduceWorkers(int count);
    void increaseWorkers(int count);

    // 环境管理
    void updateEnvironment(int change);
    void applyEnvironmentalPenalty(int severity = 1);

    // 回合管理
    void startTurn();
    void processTurn();
    void endTurn();
    bool isGameOver() const;
    bool isTimeOut() const;

    // 成本计算
    int calculateConstructionCost(int x, int y, Constants::ConstructionType type) const;
    int calculateLaborCostForTurn() const;

    // 回调设置
    void setBudgetChangeCallback(std::function<void(int, const std::string&)> callback);
    void setWorkersChangeCallback(std::function<void(int, int)> callback);
    void setEnvironmentChangeCallback(std::function<void(int)> callback);
    void setTurnChangeCallback(std::function<void(int)> callback);

    // Getters
    int getBudget() const { return currentState.budget; }
    int getTotalWorkers() const { return currentState.totalWorkers; }
    int getAvailableWorkers() const { return currentState.availableWorkers; }
    int getEnvironmentScore() const { return currentState.environmentScore; }
    int getCurrentTurn() const { return currentState.currentTurn; }
    int getTotalTurns() const { return currentState.totalTurns; }
    int getRemainingTurns() const { return currentState.totalTurns - currentState.currentTurn; }

    const ResourceState& getState() const { return currentState; }
    const std::map<std::pair<int, int>, int>& getWorkerAssignments() const {
        return currentState.workerAssignments;
    }

    // 状态检查
    bool isBudgetCritical() const { return currentState.budget < 500; }
    bool isEnvironmentCritical() const { return currentState.environmentScore < 30; }
    bool isWorkersCritical() const { return currentState.totalWorkers < 3; }
};

#endif