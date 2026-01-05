#include "RandomEvent.hpp"
#include "GameEngine.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

RandomEvent::RandomEvent()
    : resourceManager(nullptr), mapSystem(nullptr),
    constructionSystem(nullptr), gameEngine(nullptr),
    rng(std::random_device{}()),
    currentSeason(GameEnums::Season::SPRING), currentWeather(GameEnums::Weather::CLEAR),
    seasonTurnCounter(0) {
    onEventTriggered = [](const GameEvent&) {};
    onEventEnded = [](const GameEvent&) {};
    onWeatherChanged = [](GameEnums::Season, GameEnums::Weather) {};

    initializeProbabilities();
}

void RandomEvent::initialize(ResourceManager* resMgr, MapSystem* mapSys,
    ConstructionSystem* constSys, GameEngine* engine) {
    resourceManager = resMgr;
    mapSystem = mapSys;
    constructionSystem = constSys;
    gameEngine = engine;
}

void RandomEvent::initializeProbabilities() {
    baseEventProbabilities = {
        {EventType::WEATHER_RAIN, 0.15f},
        {EventType::WEATHER_SNOW, 0.08f},
        {EventType::WEATHER_STORM, 0.05f},
        {EventType::SAFETY_ACCIDENT, 0.10f},
        {EventType::SAFETY_COLLAPSE, 0.03f},
        {EventType::ENVIRONMENT_PENALTY, 0.12f},
        {EventType::BONUS_TREASURE, 0.04f},
        {EventType::BONUS_SPONSOR, 0.02f},
        {EventType::TERRAIN_HAZARD, 0.07f}
    };
}

void RandomEvent::processTurn() {
    updateSeasonAndWeather();
    for (auto it = activeEvents.begin(); it != activeEvents.end(); ) {
        it->remainingTurns--;

        if (it->remainingTurns <= 0) {
            onEventEnded(*it);
            it = activeEvents.erase(it);
        }
        else {
            applyEventEffects(*it);
            ++it;
        }
    }
    checkEvents();
    applyWeatherEffects();
    checkEnvironmentalConditions();
}

void RandomEvent::updateSeasonAndWeather() {
    seasonTurnCounter++;

    // 每10回合更换季节
    GameEnums::Season oldSeason = currentSeason;
    currentSeason = getSeasonByTurn(seasonTurnCounter / 10);
    if (oldSeason != currentSeason) {
        std::cout << "季节变化: ";
        switch (currentSeason) {
        case GameEnums::Season::SPRING: std::cout << "春季"; break;
        case GameEnums::Season::SUMMER: std::cout << "夏季"; break;
        case GameEnums::Season::AUTUMN: std::cout << "秋季"; break;
        case GameEnums::Season::WINTER: std::cout << "冬季"; break;
        }
        std::cout << std::endl;
    }
    std::uniform_real_distribution<float> weatherDist(0.0f, 1.0f);
    float weatherRoll = weatherDist(rng);
    GameEnums::Weather oldWeather = currentWeather;

    // 天气概率
    switch (currentSeason) {
    case GameEnums::Season::SPRING:
        if (weatherRoll < 0.4f) currentWeather = GameEnums::Weather::RAIN;
        else if (weatherRoll < 0.7f) currentWeather = GameEnums::Weather::CLOUDY;
        else currentWeather = GameEnums::Weather::CLEAR;
        break;

    case GameEnums::Season::SUMMER:
        if (weatherRoll < 0.1f) currentWeather = GameEnums::Weather::STORM;
        else if (weatherRoll < 0.3f) currentWeather = GameEnums::Weather::RAIN;
        else currentWeather = GameEnums::Weather::CLEAR;
        break;

    case GameEnums::Season::AUTUMN:
        if (weatherRoll < 0.3f) currentWeather = GameEnums::Weather::RAIN;
        else if (weatherRoll < 0.6f) currentWeather = GameEnums::Weather::CLOUDY;
        else currentWeather = GameEnums::Weather::CLEAR;
        break;

    case GameEnums::Season::WINTER:
        if (weatherRoll < 0.5f) currentWeather = GameEnums::Weather::SNOW;
        else if (weatherRoll < 0.7f) currentWeather = GameEnums::Weather::CLOUDY;
        else currentWeather = GameEnums::Weather::CLEAR;
        break;
    }
    if (oldWeather != currentWeather) {
        onWeatherChanged(currentSeason, currentWeather);
    }
}

GameEnums::Season RandomEvent::getSeasonByTurn(int turn) const {
    int seasonIndex = turn % 4;
    switch (seasonIndex) {
    case 0: return GameEnums::Season::SPRING;
    case 1: return GameEnums::Season::SUMMER;
    case 2: return GameEnums::Season::AUTUMN;
    case 3: return GameEnums::Season::WINTER;
    default: return GameEnums::Season::SPRING;
    }
}

float RandomEvent::getWeatherModifier() const {
    switch (currentWeather) {
    case GameEnums::Weather::RAIN: return 1.2f;  // 雨天成本增加20%
    case GameEnums::Weather::SNOW: return 1.5f;  // 雪天成本增加50%
    case GameEnums::Weather::STORM: return 2.0f; // 暴风雨成本增加100%
    default: return 1.0f;
    }
}

float RandomEvent::getSeasonalModifier() const {
    switch (currentSeason) {
    case GameEnums::Season::WINTER: return 1.3f; // 冬季成本增加30%
    case GameEnums::Season::SUMMER: return 0.9f; // 夏季成本减少10%
    default: return 1.0f;
    }
}
void RandomEvent::checkEvents() {
    if (!resourceManager || !mapSystem) return;
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    // 检查每种类型的事件
    for (const auto& [eventType, baseProbability] : baseEventProbabilities) {
        float actualProbability = baseProbability;
        // 根据游戏状态调整概率
        switch (eventType) {
        case EventType::SAFETY_ACCIDENT:
            // 施工地点越多，事故概率越高
            if (constructionSystem) {
                int activeTasks = constructionSystem->getActiveTaskCount();
                actualProbability *= (1.0f + activeTasks * 0.1f);
            }
            break;

        case EventType::TERRAIN_HAZARD:
            // 检查山体滑坡风险
            actualProbability *= getWeatherRisk();
            break;
        case EventType::ENVIRONMENT_PENALTY:
            // 环境分数低时容易被罚款
            if (resourceManager->getEnvironmentScore() < 50) {
                actualProbability *= 2.0f;
            }
            break;
        default:
            break;
        }
        if (dist(rng) < actualProbability) {
            triggerEvent(eventType);
        }
    }
}

bool RandomEvent::triggerEvent(EventType type, EventSeverity severity) {
    GameEvent event(EventType::WEATHER_RAIN, EventSeverity::MINOR, "", ""); 
    switch (type) {
    case EventType::WEATHER_RAIN:
    case EventType::WEATHER_SNOW:
    case EventType::WEATHER_STORM:
        event = generateWeatherEvent();
        break;

    case EventType::SAFETY_ACCIDENT:
    case EventType::SAFETY_COLLAPSE:
        event = generateSafetyEvent();
        break;

    case EventType::ENVIRONMENT_PENALTY:
        event = generateEnvironmentalEvent();
        break;

    case EventType::BONUS_TREASURE:
    case EventType::BONUS_SPONSOR:
        event = generateBonusEvent();
        break;

    case EventType::TERRAIN_HAZARD:
        event = generateTerrainHazard();
        break;

    default:
        return false;
    }

    event.severity = severity;
    activeEvents.push_back(event);
    applyEventEffects(event);
    onEventTriggered(event);
    std::cout << "事件触发: " << event.title << " - " << event.description << std::endl;

    return true;
}

GameEvent RandomEvent::generateWeatherEvent() {
    GameEvent event(EventType::WEATHER_RAIN, EventSeverity::MODERATE, "", "");

    switch (currentWeather) {
    case GameEnums::Weather::RAIN:
        event.type = EventType::WEATHER_RAIN;
        event.title = "Continuous rainfall";
        event.description = "Continuous rainfall has slowed down the construction progress and increased labor costs.";
        event.budgetEffect = -100;
        event.environmentEffect = 5;
        event.duration = 3;
        break;

    case GameEnums::Weather::SNOW:
        event.type = EventType::WEATHER_SNOW;
        event.title = "heavy snow";
        event.description = "Heavy snow caused road closures and construction to come to a complete halt.";
        event.budgetEffect = -200;
        event.duration = 2;
        break;

    case GameEnums::Weather::STORM:
        event.type = EventType::WEATHER_STORM;
        event.title = "Storm strikes";
        event.description = "A strong storm caused damage to the construction site.";
        event.budgetEffect = -500;
        event.environmentEffect = -10;
        event.severity = EventSeverity::MAJOR;
        break;

    default:
        // 默认天气事件
        event.type = EventType::WEATHER_RAIN;
        event.title = "Unexpected weather";
        event.description = "The sudden weather conditions have affected the construction";
        event.budgetEffect = -50;
        break;
    }

    event.remainingTurns = event.duration;
    return event;
}

GameEvent RandomEvent::generateSafetyEvent() {
    GameEvent event(EventType::SAFETY_ACCIDENT, EventSeverity::MODERATE, "Accident", "");

    std::uniform_int_distribution<int> severityDist(1, 3);
    int severityRoll = severityDist(rng);

    // 查找施工地点
    auto constructionSites = findConstructionSites();

    if (!constructionSites.empty()) {
        std::uniform_int_distribution<int> siteDist(0, constructionSites.size() - 1);
        auto site = constructionSites[siteDist(rng)];
        event.affectedCells.push_back(site);
    }

    switch (severityRoll) {
    case 1: 
        event.description = "Minor construction accidents resulting in minor losses.";
        event.budgetEffect = -200;
        event.workerEffect = -1; 
        event.severity = EventSeverity::MINOR;
        break;

    case 2:
        event.description = "Moderate construction accident, compensation needs to be paid.";
        event.budgetEffect = -500;
        event.workerEffect = -2;
        event.severity = EventSeverity::MODERATE;
        break;

    case 3:
        event.description = "Serious construction accidents resulting in significant losses and casualties.";
        event.budgetEffect = -1000;
        event.workerEffect = -3;
        event.severity = EventSeverity::MAJOR;
        break;
    }
    if (!constructionSites.empty()) {
        for (const auto& site : constructionSites) {
            if (mapSystem->getAssignedWorkers(site.first, site.second) < 2) {
                // 劳动力投入过少，增加事故严重性
                event.budgetEffect *= 2;
                event.description += " (Poor regulation has exacerbated losses.)";
                break;
            }
        }
    }

    return event;
}

GameEvent RandomEvent::generateEnvironmentalEvent() {
    GameEvent event(EventType::ENVIRONMENT_PENALTY, EventSeverity::MODERATE, "Environmental fines", "");

    if (resourceManager && resourceManager->getEnvironmentScore() < 40) {
        event.description = "Serious environmental damage, received large fines.";
        event.budgetEffect = -800;
        event.environmentEffect = -15;
        event.severity = EventSeverity::MAJOR;
    }
    else {
        event.description = "Environmental regulatory fines.";
        event.budgetEffect = -300;
        event.environmentEffect = -5;
        event.severity = EventSeverity::MODERATE;
    }

    return event;
}

GameEvent RandomEvent::generateBonusEvent() {
    GameEvent event(EventType::BONUS_TREASURE, EventSeverity::MODERATE, "", "");
    event.isPositive = true;

    std::uniform_int_distribution<int> bonusDist(1, 2);

    if (bonusDist(rng) == 1) {
        event.type = EventType::BONUS_TREASURE;
        event.title = "Discovering treasure";
        event.description = "Hidden treasures were discovered during the construction process!";
        event.budgetEffect = 1000;

        // 标记发现位置（如果有施工地点）
        auto sites = findConstructionSites();
        if (!sites.empty()) {
            std::uniform_int_distribution<int> siteDist(0, sites.size() - 1);
            event.affectedCells.push_back(sites[siteDist(rng)]);
        }
    }
    else {
        event.type = EventType::BONUS_SPONSOR;
        event.title = "Obtain sponsorship";
        event.description = "The project has received additional funding sponsorship.";
        event.budgetEffect = 500;
    }

    return event;
}

GameEvent RandomEvent::generateTerrainHazard() {
    GameEvent event(EventType::TERRAIN_HAZARD, EventSeverity::MODERATE, "Terrain disasters", "");
    auto vulnerableCells = findVulnerableCells();
    if (vulnerableCells.empty()) {
        event.description = "Minor terrain changes have affected construction";
        event.budgetEffect = -100;
        return event;
    }

    std::uniform_int_distribution<int> cellDist(0, vulnerableCells.size() - 1);
    auto& affectedCell = vulnerableCells[cellDist(rng)];
    event.affectedCells.push_back(affectedCell);
    const Cell& cell = mapSystem->getCell(affectedCell.first, affectedCell.second);
    if (cell.terrain == Constants::TerrainType::MOUNTAIN && !cell.isReinforced) {
        event.title = "landslide";
        event.description = "Unreinforced mountain landslide causing damage to roads";
        event.budgetEffect = -400;
        event.severity = EventSeverity::MAJOR;
        if (currentWeather == GameEnums::Weather::RAIN || currentWeather == GameEnums::Weather::STORM) {
            event.budgetEffect *= 2;
            event.description += " (Severe weather exacerbates disasters)";
        }
    }
    else if (cell.terrain == Constants::TerrainType::RIVER) {
        event.title = "Flood impact";
        event.description = "The rising water level of the river has washed away some facilities.";
        event.budgetEffect = -300;
        event.severity = EventSeverity::MODERATE;
    }
    else {
        event.description = "Unstable terrain leads to construction delays.";
        event.budgetEffect = -150;
    }

    return event;
}

void RandomEvent::applyWeatherEffects() {
    if (!resourceManager) return;

    // 天气对劳动力成本的影响
    float costModifier = getWeatherModifier() * getSeasonalModifier();

    if (costModifier > 1.0f) {
        // 计算额外的天气成本
        int extraCost = static_cast<int>((costModifier - 1.0f) * 100);
        resourceManager->spendBudget(extraCost);
    }
}

void RandomEvent::checkEnvironmentalConditions() {
    if (!resourceManager || !constructionSystem) return;
    int environmentScore = resourceManager->getEnvironmentScore();
    if (environmentScore < 30 && !resourceManager->isEnvironmentCritical()) {
        triggerEvent(EventType::ENVIRONMENT_PENALTY, EventSeverity::MAJOR);
    }
    else if (environmentScore < 50) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        if (dist(rng) < 0.3f) {
            triggerEvent(EventType::ENVIRONMENT_PENALTY, EventSeverity::MODERATE);
        }
    }
}

void RandomEvent::applyEventEffects(const GameEvent& event) {
    if (!resourceManager) return;
    if (event.budgetEffect != 0) {
        if (event.budgetEffect > 0) {
            resourceManager->addBudget(event.budgetEffect);
        }
        else {
            resourceManager->spendBudget(-event.budgetEffect);
        }
    }
    if (event.workerEffect != 0) {
        if (event.workerEffect > 0) {
            resourceManager->increaseWorkers(event.workerEffect);
        }
        else {
            resourceManager->permanentlyReduceWorkers(-event.workerEffect);
        }
    }

    // 应用环境效果
    if (event.environmentEffect != 0) {
        resourceManager->updateEnvironment(event.environmentEffect);
    }
}

std::vector<std::pair<int, int>> RandomEvent::findVulnerableCells() const {
    std::vector<std::pair<int, int>> vulnerableCells;

    if (!mapSystem) return vulnerableCells;

    const auto& grid = mapSystem->getGrid();

    for (int y = 0; y < Constants::MAP_HEIGHT; ++y) {
        for (int x = 0; x < Constants::MAP_WIDTH; ++x) {
            const Cell& cell = grid[y][x];
            if (cell.terrain == Constants::TerrainType::MOUNTAIN && !cell.isReinforced) {
                vulnerableCells.emplace_back(x, y);
            }
            if (cell.terrain == Constants::TerrainType::RIVER) {
                vulnerableCells.emplace_back(x, y);
            }
        }
    }

    return vulnerableCells;
}

std::vector<std::pair<int, int>> RandomEvent::findConstructionSites() const {
    std::vector<std::pair<int, int>> constructionSites;

    if (!constructionSystem) return constructionSites;

    const auto& activeTasks = constructionSystem->getActiveTasks();

    for (const auto& task : activeTasks) {
        if (!task->isCompleted && task->assignedWorkers > 0) {
            constructionSites.emplace_back(task->x, task->y);
        }
    }

    return constructionSites;
}
float RandomEvent::getWeatherRisk() const {
    switch (currentWeather) {
    case GameEnums::Weather::STORM: return 2.0f;
    case GameEnums::Weather::RAIN: return 1.5f;
    case GameEnums::Weather::SNOW: return 1.3f;
    default: return 1.0f;
    }
}

void RandomEvent::setEventTriggeredCallback(std::function<void(const GameEvent&)> callback) {
    onEventTriggered = callback;
}

void RandomEvent::setEventEndedCallback(std::function<void(const GameEvent&)> callback) {
    onEventEnded = callback;
}

void RandomEvent::setWeatherChangedCallback(std::function<void(GameEnums::Season, GameEnums::Weather)> callback) {
    onWeatherChanged = callback;
}

void RandomEvent::printActiveEvents() const {
    std::cout << "=== 活跃事件 ===" << std::endl;
    for (const auto& event : activeEvents) {
        std::cout << event.title << " - " << event.description
            << " (剩余回合: " << event.remainingTurns << ")" << std::endl;
    }
    std::cout << "===============" << std::endl;
}

void RandomEvent::forceEvent(EventType type) {
    triggerEvent(type, EventSeverity::MODERATE);
}