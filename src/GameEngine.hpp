#ifndef GAMEENGINE_HPP
#define GAMEENGINE_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include "Constants.hpp"
#include "GameState.hpp"
#include "MapSystem.hpp"
#include "ResourceManager.hpp"
#include "ConstructionSystem.hpp"
#include "RandomEvent.hpp"
#include "InputHandler.hpp"
#include "Render.hpp"
class Render;
class GameEngine {
private:
    // SFML窗口
    sf::RenderWindow window;

    // 游戏状态
    GameState currentState;
    bool gameRunning;
    bool gamePaused;
    
    // 游戏系统
    std::unique_ptr<MapSystem> mapSystem;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<ConstructionSystem> constructionSystem;
    std::unique_ptr<RandomEvent> randomEvent;
    std::unique_ptr<InputHandler> inputHandler;
    std::unique_ptr<Render> Rrender;
    std::unique_ptr<GameStateManager> stateManager;
    // 游戏计时
    sf::Clock gameClock;
    sf::Time turnTime;
    sf::Time elapsedTime;

    // 回合管理
    int currentTurn;
    int maxTurns;

    // 游戏结果
    bool gameWon;
    std::string gameResultMessage;

    // 内部方法
    void initializeSystems();
    void setupCallbacks();
    void checkGameConditions();
    void handleGameOver();

public:
    GameEngine();
    ~GameEngine();

    // 游戏生命周期
    bool initialize();
    void run();
    void cleanup();

    // 游戏流程控制
    void startGame();
    void endGame(bool won, const std::string& message);
    void pauseGame();
    void resumeGame();
    void restartGame();

    // 回合管理
    void startTurn();
    void processTurn();
    void endTurn();

    // 事件处理
    void handleEvents();
    void handleInput(const sf::Event& event);
    void update(float deltaTime);
    void render();

    // 状态获取
    GameState getCurrentState() const { return currentState; }
    bool isRunning() const { return gameRunning; }
    bool isPaused() const { return gamePaused; }
    int getCurrentTurn() const { return currentTurn; }
    int getMaxTurns() const { return maxTurns; }
    bool isGameWon() const { return gameWon; }
    std::string getGameResult() const { return gameResultMessage; }

    // 系统访问（用于调试和测试）
    MapSystem* getMapSystem() { return mapSystem.get(); }
    ResourceManager* getResourceManager() { return resourceManager.get(); }
    ConstructionSystem* getConstructionSystem() { return constructionSystem.get(); }
    RandomEvent* getRandomEvent() { return randomEvent.get(); }

    // 调试工具
    void printGameState() const;
    void forceWin() { endGame(true, "强制胜利（调试）"); }
    void forceLose() { endGame(false, "强制失败（调试）"); }
};

#endif