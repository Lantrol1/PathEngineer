#include "GameState.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>

GameStateManager::GameStateManager()
    : currentState(GameState::MAIN_MENU),
    previousState(GameState::MAIN_MENU),
    currentSeason(GameEnums::Season::SPRING),
    currentWeather(GameEnums::Weather::CLEAR) {

    // 初始化默认配置
    currentConfig = GameConfig();
    currentStats = GameStats();

    // 默认状态转换回调
    GameStateManager::onStateChange = [](GameState, GameState) {};
}

void GameStateManager::setState(GameState newState) {
    // 验证状态转换是否有效
    if (!isValidTransition(currentState, newState)) {
        std::cerr << "无效的状态转换: " << stateToString(currentState)
            << " -> " << stateToString(newState) << std::endl;
        return;
    }

    // 更新状态
    previousState = currentState;
    currentState = newState;

    // 触发回调
    onStateChange(previousState, currentState);

    std::cout << "游戏状态变更: " << stateToString(previousState)
        << " -> " << stateToString(currentState) << std::endl;
}

void GameStateManager::returnToPreviousState() {
    setState(previousState);
}

GameState GameStateManager::getCurrentState() const {
    return currentState;
}

GameState GameStateManager::getPreviousState() const {
    return previousState;
}

void GameStateManager::setConfig(const GameConfig& config) {
    currentConfig = config;
}

GameConfig GameStateManager::getConfig() const {
    return currentConfig;
}

void GameStateManager::setDifficulty(Difficulty difficulty) {
    currentConfig.applyDifficulty(difficulty);
}

void GameStateManager::updateStats(const GameStats& stats) {
    currentStats = stats;
}

GameStats GameStateManager::getStats() const {
    return currentStats;
}

void GameStateManager::resetStats() {
    currentStats.reset();
}

void GameStateManager::setSeason(GameEnums::Season season) {
    currentSeason = season;
}

GameEnums::Season GameStateManager::getSeason() const {
    return currentSeason;
}

void GameStateManager::setWeather(GameEnums::Weather weather) {
    currentWeather = weather;
}

GameEnums::Weather GameStateManager::getWeather() const {
    return currentWeather;
}

bool GameStateManager::saveGame(const std::string& filename, const SaveData& data) const {
    // 在实际项目中，这里会实现完整的存档功能
    // 目前只输出存档信息
    std::cout << "保存游戏: " << filename << std::endl;
    std::cout << "存档名称: " << data.saveName << std::endl;
    std::cout << "时间戳: " << data.timestamp << std::endl;
    std::cout << "游戏状态: " << stateToString(data.gameState) << std::endl;
    std::cout << "当前回合: " << data.currentTurn << std::endl;

    return true; // 简化实现，总是返回成功
}

bool GameStateManager::loadGame(const std::string& filename, SaveData& data) const {
    // 在实际项目中，这里会实现完整的读档功能
    // 目前只输出读档信息
    std::cout << "加载游戏: " << filename << std::endl;

    // 设置一些示例数据
    data.saveName = "示例存档";

    // 生成当前时间戳
    std::time_t t = std::time(nullptr);
    //std::tm tm = *std::localtime(&t);
    std::stringstream ss;
    //ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    data.timestamp = ss.str();

    data.gameState = GameState::IN_GAME;
    data.currentTurn = 5;

    std::cout << "加载存档: " << data.saveName << " (回合 " << data.currentTurn << ")" << std::endl;

    return true; // 简化实现，总是返回成功
}

std::string GameStateManager::stateToString(GameState state) const {
    static std::map<GameState, std::string> stateMap = {
        {GameState::MAIN_MENU, "MAIN_MENU"},
        {GameState::LEVEL_SELECTION, "LEVEL_SELECTION"},
        {GameState::IN_GAME, "IN_GAME"},
        {GameState::DEPLOYMENT, "DEPLOYMENT"},
        {GameState::TURN_PROCESSING, "TURN_PROCESSING"},
        {GameState::TURN_END, "TURN_END"},
        {GameState::GAME_OVER, "GAME_OVER"},
        {GameState::PAUSED, "PAUSED"},
        {GameState::SETTINGS, "SETTINGS"},
        {GameState::CREDITS, "CREDITS"}
    };

    auto it = stateMap.find(state);
    return it != stateMap.end() ? it->second : "Unknown";
}

std::string GameStateManager::seasonToString(GameEnums::Season season) const {
    static std::map<GameEnums::Season, std::string> seasonMap = {
        {GameEnums::Season::SPRING, "SPRING"},
        {GameEnums::Season::SUMMER, "SUMMER"},
        {GameEnums::Season::AUTUMN, "AUTUMN"},
        {GameEnums::Season::WINTER, "WINTER"}
    };

    auto it = seasonMap.find(season);
    return it != seasonMap.end() ? it->second : "Unknown";
}

std::string GameStateManager::weatherToString(GameEnums::Weather weather) const {
    static std::map<GameEnums::Weather, std::string> weatherMap = {
        {GameEnums::Weather::CLEAR, "CLEAR"},
        {GameEnums::Weather::CLOUDY, "CLOUDY"},
        {GameEnums::Weather::RAIN, "RAIN"},
        {GameEnums::Weather::STORM, "STORM"},
        {GameEnums::Weather::SNOW, "SNOW"}
    };

    auto it = weatherMap.find(weather);
    return it != weatherMap.end() ? it->second : "Unknown";
}

std::string GameStateManager::difficultyToString(Difficulty difficulty) const {
    static std::map<Difficulty, std::string> difficultyMap = {
        {Difficulty::EASY, "EASY"},
        {Difficulty::NORMAL, "NORMAL"},
        {Difficulty::HARD, "HARD"},
        {Difficulty::EXPERT, "EXPERT"}
    };

    auto it = difficultyMap.find(difficulty);
    return it != difficultyMap.end() ? it->second : "Unknown";
}

void GameStateManager::setStateChangeCallback(std::function<void(GameState, GameState)> callback) {
    onStateChange = callback;
}

bool GameStateManager::isValidTransition(GameState from, GameState to) const {
    // 定义有效的状态转换规则
    static std::map<GameState, std::vector<GameState>> validTransitions = {
        {GameState::MAIN_MENU, {
            GameState::LEVEL_SELECTION,
            GameState::SETTINGS,
            GameState::CREDITS
        }},
        {GameState::LEVEL_SELECTION, {
            GameState::MAIN_MENU,
            GameState::IN_GAME
        }},
        {GameState::IN_GAME, {
            GameState::PAUSED,
            GameState::GAME_OVER,
            GameState::DEPLOYMENT
        }},
        {GameState::DEPLOYMENT, {
            GameState::TURN_PROCESSING,
            GameState::PAUSED
        }},
        {GameState::TURN_PROCESSING, {
            GameState::TURN_END,
            GameState::PAUSED
        }},
        {GameState::TURN_END, {
            GameState::DEPLOYMENT,
            GameState::GAME_OVER,
            GameState::PAUSED
        }},
        {GameState::GAME_OVER, {
            GameState::MAIN_MENU
        }},
        {GameState::PAUSED, {
            GameState::IN_GAME,
            GameState::DEPLOYMENT,
            GameState::TURN_PROCESSING,
            GameState::TURN_END,
            GameState::MAIN_MENU
        }},
        {GameState::SETTINGS, {
            GameState::MAIN_MENU
        }},
        {GameState::CREDITS, {
            GameState::MAIN_MENU
        }}
    };

    auto it = validTransitions.find(from);
    if (it == validTransitions.end()) {
        return false; // 未知的起始状态
    }

    const auto& allowedStates = it->second;
    return std::find(allowedStates.begin(), allowedStates.end(), to) != allowedStates.end();
}