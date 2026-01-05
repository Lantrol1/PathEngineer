#ifndef RANDOMEVENT_HPP
#define RANDOMEVENT_HPP

#include <random>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include "Constants.hpp"
#include "ResourceManager.hpp"
#include "MapSystem.hpp"
#include "ConstructionSystem.hpp"
#include "GameEngine.hpp"
// 前向声明
class GameEngine;

// 事件类型枚举
enum class EventType {
    WEATHER_RAIN,
    WEATHER_SNOW,
    WEATHER_STORM,
    SAFETY_ACCIDENT,
    SAFETY_COLLAPSE,
    ENVIRONMENT_PENALTY,
    BONUS_TREASURE,
    BONUS_SPONSOR,
    SEASONAL_EVENT,
    TERRAIN_HAZARD
};

// 事件严重程度
enum class EventSeverity {
    MINOR,      // 轻微影响
    MODERATE,   // 中等影响
    MAJOR,      // 严重影响
    CRITICAL    // 致命影响
};

// 事件结构体
struct GameEvent {
    EventType type;
    EventSeverity severity;
    std::string title;
    std::string description;
    int budgetEffect;
    int workerEffect;
    int environmentEffect;
    int duration; // 事件持续回合数
    int remainingTurns; // 剩余回合数
    bool isActive;
    bool isPositive;

    // 事件影响的位置（可选）
    std::vector<std::pair<int, int>> affectedCells;

    GameEvent(EventType t, EventSeverity s, const std::string& tit, const std::string& desc)
        : type(t), severity(s), title(tit), description(desc),
        budgetEffect(0), workerEffect(0), environmentEffect(0),
        duration(1), remainingTurns(1), isActive(true), isPositive(false) {
    }
};
class RandomEvent {
private:
    ResourceManager* resourceManager;
    MapSystem* mapSystem;
    ConstructionSystem* constructionSystem;
    GameEngine* gameEngine;

    mutable std::mt19937 rng;

    // 当前季节和天气
    GameEnums::Season currentSeason;
    GameEnums::Weather currentWeather;
    int seasonTurnCounter;

    // 活跃事件
    std::vector<GameEvent> activeEvents;

    // 事件概率表
    std::map<EventType, float> baseEventProbabilities;

    // 回调函数
    std::function<void(const GameEvent&)> onEventTriggered;
    std::function<void(const GameEvent&)> onEventEnded;
    std::function<void(GameEnums::Season, GameEnums::Weather)> onWeatherChanged;

    // 内部方法
    void initializeProbabilities();
    void updateSeasonAndWeather();
    GameEnums::Season getSeasonByTurn(int turn) const;
    float getWeatherModifier() const;
    float getSeasonalModifier() const;

    // 事件生成
    GameEvent generateWeatherEvent();
    GameEvent generateSafetyEvent();
    GameEvent generateEnvironmentalEvent();
    GameEvent generateBonusEvent();
    GameEvent generateTerrainHazard();

    // 事件效果应用
    void applyWeatherEffects();
    void checkEnvironmentalConditions();
    void applyEventEffects(const GameEvent& event);

    // 辅助方法
    std::vector<std::pair<int, int>> findVulnerableCells() const;
    std::vector<std::pair<int, int>> findConstructionSites() const;

public:
    RandomEvent();

    void initialize(ResourceManager* resMgr, MapSystem* mapSys,
        ConstructionSystem* constSys = nullptr, GameEngine* engine = nullptr);

    // 主更新方法
    void processTurn();

    // 事件检查
    void checkEvents();
    bool triggerEvent(EventType type, EventSeverity severity = EventSeverity::MODERATE);
    GameEnums::Season getCurrentSeason() const { return currentSeason; }
    GameEnums::Weather getCurrentWeather() const { return currentWeather; }
    const std::vector<GameEvent>& getActiveEvents() const { return activeEvents; }
    bool hasActiveEvents() const { return !activeEvents.empty(); }
    float getWeatherRisk() const;
    void setEventTriggeredCallback(std::function<void(const GameEvent&)> callback);
    void setEventEndedCallback(std::function<void(const GameEvent&)> callback);
    void setWeatherChangedCallback(std::function<void(GameEnums::Season, GameEnums::Weather)> callback);
    void printActiveEvents() const;
    void forceEvent(EventType type);
};

#endif