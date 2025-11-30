#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP

#include <string>
#include <map>
#include <functional>

// 游戏状态枚举
enum class GameState {
    MAIN_MENU,          // 主菜单
    LEVEL_SELECTION,     // 关卡选择
    IN_GAME,            // 游戏进行中
    DEPLOYMENT,         // 部署劳动力阶段
    TURN_PROCESSING,    // 回合处理中
    TURN_END,           // 回合结束
    GAME_OVER,          // 游戏结束
    PAUSED,             // 游戏暂停
    SETTINGS,           // 设置界面
    CREDITS             // 制作人员
};
namespace GameEnums {
    // 季节枚举
    enum class Season {
        SPRING,
        SUMMER,
        AUTUMN,
        WINTER
    };

    // 天气枚举
    enum class Weather {
        CLEAR,
        CLOUDY,
        RAIN,
        STORM,
        SNOW
    };
}


// 游戏难度枚举
enum class Difficulty {
    EASY,
    NORMAL,
    HARD,
    EXPERT
};

// 游戏配置结构体
struct GameConfig {
    Difficulty difficulty;
    int initialBudget;
    int initialWorkers;
    int maxTurns;
    int environmentTarget;

    GameConfig()
        : difficulty(Difficulty::NORMAL),
        initialBudget(10000),
        initialWorkers(10),
        maxTurns(50),
        environmentTarget(60) {
    }

    // 根据难度调整配置
    void applyDifficulty(Difficulty diff) {
        difficulty = diff;

        switch (diff) {
        case Difficulty::EASY:
            initialBudget = 15000;
            initialWorkers = 12;
            maxTurns = 60;
            environmentTarget = 50;
            break;

        case Difficulty::NORMAL:
            initialBudget = 10000;
            initialWorkers = 10;
            maxTurns = 50;
            environmentTarget = 60;
            break;

        case Difficulty::HARD:
            initialBudget = 7000;
            initialWorkers = 8;
            maxTurns = 40;
            environmentTarget = 70;
            break;

        case Difficulty::EXPERT:
            initialBudget = 5000;
            initialWorkers = 6;
            maxTurns = 30;
            environmentTarget = 80;
            break;
        }
    }
};

// 游戏统计信息
struct GameStats {
    int totalTurns;
    int totalCost;
    int environmentScore;
    int workersLost;
    int eventsTriggered;
    bool victory;
    std::string victoryType;

    GameStats()
        : totalTurns(0), totalCost(0), environmentScore(0),
        workersLost(0), eventsTriggered(0), victory(false),
        victoryType("") {
    }

    void reset() {
        totalTurns = 0;
        totalCost = 0;
        environmentScore = 0;
        workersLost = 0;
        eventsTriggered = 0;
        victory = false;
        victoryType = "";
    }
};

// 存档数据
struct SaveData {
    std::string saveName;
    std::string timestamp;
    GameState gameState;
    GameConfig config;
    GameStats stats;
    int currentTurn;

    SaveData()
        : saveName(""), timestamp(""), gameState(GameState::MAIN_MENU),
        currentTurn(1) {
    }
};

// 游戏状态管理器
class GameStateManager {
private:
    GameState currentState;
    GameState previousState;
    GameConfig currentConfig;
    GameStats currentStats;
    GameEnums::Season currentSeason;
    GameEnums::Weather currentWeather;

    // 状态转换回调
    std::function<void(GameState, GameState)> onStateChange;

public:
    GameStateManager();

    // 状态管理
    void setState(GameState newState);
    void returnToPreviousState();
    GameState getCurrentState() const;
    GameState getPreviousState() const;

    // 配置管理
    void setConfig(const GameConfig& config);
    GameConfig getConfig() const;
    void setDifficulty(Difficulty difficulty);

    // 统计管理
    void updateStats(const GameStats& stats);
    GameStats getStats() const;
    void resetStats();

    // 季节天气管理
    void setSeason(GameEnums::Season season);
    GameEnums::Season getSeason() const;
    void setWeather(GameEnums::Weather weather);
    GameEnums::Weather getWeather() const;

    // 存档管理
    bool saveGame(const std::string& filename, const SaveData& data) const;
    bool loadGame(const std::string& filename, SaveData& data) const;

    // 工具方法
    std::string stateToString(GameState state) const;
    std::string seasonToString(GameEnums::Season season) const;
    std::string weatherToString(GameEnums::Weather weather) const;
    std::string difficultyToString(Difficulty difficulty) const;

    // 回调设置
    void setStateChangeCallback(std::function<void(GameState, GameState)> callback);

    // 验证状态转换
    bool isValidTransition(GameState from, GameState to) const;
};

#endif