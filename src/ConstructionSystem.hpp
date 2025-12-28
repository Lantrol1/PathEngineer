#ifndef CONSTRUCTIONSYSTEM_HPP
#define CONSTRUCTIONSYSTEM_HPP

#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "Constants.hpp"
#include "MapSystem.hpp"
#include "ResourceManager.hpp"

// 前向声明
class RandomEvent;

// 建设任务结构体
struct ConstructionTask {
    int x, y;
    Constants::ConstructionType type;
    int assignedWorkers;
    int requiredWorkPoints;
    int completedWorkPoints;
    bool isCompleted;
    std::string description;

    ConstructionTask(int x, int y, Constants::ConstructionType type)
        : x(x), y(y), type(type), assignedWorkers(0),
        requiredWorkPoints(calculateRequiredWorkPoints()),
        completedWorkPoints(0), isCompleted(false),
        description(generateDescription()) {
    }

    // 进度百分比
    float getProgress() const {
        if (requiredWorkPoints == 0) return 1.0f;
        return static_cast<float>(completedWorkPoints) / requiredWorkPoints;
    }

    // 检查是否完成
    bool checkCompletion() {
        if (!isCompleted && completedWorkPoints >= requiredWorkPoints) {
            isCompleted = true;
            return true;
        }
        return false;
    }

    // 添加工作点数
    void addWorkPoints(int points) {
        completedWorkPoints += points;
        if (completedWorkPoints > requiredWorkPoints) {
            completedWorkPoints = requiredWorkPoints;
        }
        checkCompletion();
    }

private:
    int calculateRequiredWorkPoints() const {
        switch (type) {
        case Constants::ConstructionType::BUILD_ROAD:
            return 10;
        case Constants::ConstructionType::BUILD_BRIDGE:
            return 30;
        case Constants::ConstructionType::BUILD_TUNNEL:
            return 50;
        case Constants::ConstructionType::CLEAR_OBSTACLE:
            return 20;
        case Constants::ConstructionType::REINFORCE_MOUNTAIN:
            return 25;
        default:
            return 0;
        }
    }

    std::string generateDescription() const {
        switch (type) {
        case Constants::ConstructionType::BUILD_ROAD:
            return "修建道路";
        case Constants::ConstructionType::BUILD_BRIDGE:
            return "修建桥梁";
        case Constants::ConstructionType::BUILD_TUNNEL:
            return "开凿隧道";
        case Constants::ConstructionType::CLEAR_OBSTACLE:
            return "清除障碍";
        case Constants::ConstructionType::REINFORCE_MOUNTAIN:
            return "加固山体";
        default:
            return "未知建设";
        }
    }
};

// 建设系统类
class ConstructionSystem {
private:
    MapSystem* mapSystem;
    ResourceManager* resourceManager;
    RandomEvent* randomEvent;

    std::vector<std::unique_ptr<ConstructionTask>> activeTasks;
    std::map<std::pair<int, int>, ConstructionTask*> taskLookup;

    // 回调函数
    std::function<void(const ConstructionTask&)> onTaskStarted;
    std::function<void(const ConstructionTask&)> onTaskProgress;
    std::function<void(const ConstructionTask&)> onTaskCompleted;
    std::function<void(const std::string&)> onConstructionError;

    // 内部方法
    bool validateConstruction(int x, int y, Constants::ConstructionType type) const;
    int calculateMaterialCost(int x, int y, Constants::ConstructionType type) const;
    void executeConstruction(const ConstructionTask& task);
    ConstructionTask* findTask(int x, int y);

public:
    ConstructionSystem();

    void initialize(MapSystem* mapSys, ResourceManager* resMgr, RandomEvent* eventSys = nullptr);

    // 建设操作
    bool startConstruction(int x, int y, Constants::ConstructionType type);
    bool assignWorkersToTask(int x, int y, int workerCount);

    // 回合处理
    void processTurn();
    void update();

    // 任务查询
    bool hasActiveTask(int x, int y) const;
    const ConstructionTask* getTask(int x, int y) const;
    const std::vector<std::unique_ptr<ConstructionTask>>& getActiveTasks() const { return activeTasks; }
    int getActiveTaskCount() const { return activeTasks.size(); }

    // 进度检查
    bool isPathComplete() const;
    float getOverallProgress() const;

    // 成本计算
    int getEstimatedCost(int x, int y, Constants::ConstructionType type) const;
    int getRemainingWorkPoints(int x, int y) const;

    // 回调设置
    void setTaskStartedCallback(std::function<void(const ConstructionTask&)> callback);
    void setTaskProgressCallback(std::function<void(const ConstructionTask&)> callback);
    void setTaskCompletedCallback(std::function<void(const ConstructionTask&)> callback);
    void setConstructionErrorCallback(std::function<void(const std::string&)> callback);

    // 调试工具
    void printActiveTasks() const;
};

#endif