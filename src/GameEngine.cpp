#include "GameEngine.hpp"
#include <iostream>
#include <sstream>

GameEngine::GameEngine()
    : currentState(GameState::MAIN_MENU),
    gameRunning(false),
    gamePaused(false),
    currentTurn(1),
    maxTurns(Constants::MAX_TURNS),
    gameWon(false),
    gameResultMessage("") {

    // 创建窗口
    window.create(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
        Constants::WINDOW_TITLE);
    window.setFramerateLimit(60);
}

GameEngine::~GameEngine() {
    cleanup();
}

bool GameEngine::initialize() {
    std::cout << "初始化游戏引擎..." << std::endl;

    try {
        initializeSystems();
        setupCallbacks();

        gameRunning = true;
        gamePaused = false;
        currentState = GameState::DEPLOYMENT;
        currentTurn = 1;

        std::cout << "游戏引擎初始化完成" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "初始化失败: " << e.what() << std::endl;
        return false;
    }
}

void GameEngine::initializeSystems() {
    // 创建各个系统
    mapSystem = std::make_unique<MapSystem>();
    resourceManager = std::make_unique<ResourceManager>();
    constructionSystem = std::make_unique<ConstructionSystem>();
    randomEvent = std::make_unique<RandomEvent>();
    inputHandler = std::make_unique<InputHandler>();
    Rrender = std::make_unique<Render>();

    // 初始化各个系统
    stateManager = std::make_unique<GameStateManager>();
    mapSystem->initialize();
    resourceManager->initialize();

    constructionSystem->initialize(mapSystem.get(), resourceManager.get(), randomEvent.get());
    randomEvent->initialize(resourceManager.get(), mapSystem.get(), constructionSystem.get(), this);
    inputHandler->initialize(constructionSystem.get(), mapSystem.get(), resourceManager.get(), this);
    Rrender->initialize(mapSystem.get(), resourceManager.get(), constructionSystem.get(), randomEvent.get(), inputHandler.get());

    std::cout << "所有系统初始化完成" << std::endl;
}

void GameEngine::setupCallbacks() {
    // 设置资源管理器回调
    resourceManager->setBudgetChangeCallback([this](int newBudget, const std::string& reason) {
        std::cout << "预算变化: " << newBudget << " - " << reason << std::endl;
        });

    resourceManager->setWorkersChangeCallback([this](int available, int total) {
        std::cout << "工人变化: " << available << "/" << total << std::endl;
        });

    resourceManager->setEnvironmentChangeCallback([this](int environment) {
        std::cout << "环境分数: " << environment << std::endl;
        });

    resourceManager->setTurnChangeCallback([this](int turn) {
        std::cout << "当前回合: " << turn << std::endl;
        });

    // 设置建设系统回调
    constructionSystem->setTaskStartedCallback([this](const ConstructionTask& task) {
        std::cout << "开始建设: " << task.description << " 在 (" << task.x << ", " << task.y << ")" << std::endl;
        });

    constructionSystem->setTaskProgressCallback([this](const ConstructionTask& task) {
        // 进度更新（可以在这里添加进度显示更新）
        });

    constructionSystem->setTaskCompletedCallback([this](const ConstructionTask& task) {
        std::cout << "完成建设: " << task.description << " 在 (" << task.x << ", " << task.y << ")" << std::endl;

        // 检查是否完成路径
        if (constructionSystem->isPathComplete()) {
            endGame(true, "Complete!");
        }
        });

    constructionSystem->setConstructionErrorCallback([this](const std::string& error) {
        std::cout << "建设错误: " << error << std::endl;
        });

    // 设置随机事件回调
    randomEvent->setEventTriggeredCallback([this](const GameEvent& event) {
        std::cout << "事件触发: " << event.title << " - " << event.description << std::endl;
        });

    randomEvent->setEventEndedCallback([this](const GameEvent& event) {
        std::cout << "事件结束: " << event.title << std::endl;
        });

    randomEvent->setWeatherChangedCallback([this](GameEnums::Season season, GameEnums::Weather weather) {
        std::string seasonStr, weatherStr;

        switch (season) {
        case GameEnums::Season::SPRING: seasonStr = "春季"; break;
        case GameEnums::Season::SUMMER: seasonStr = "夏季"; break;
        case GameEnums::Season::AUTUMN: seasonStr = "秋季"; break;
        case GameEnums::Season::WINTER: seasonStr = "冬季"; break;
        }

        switch (weather) {
        case GameEnums::Weather::CLEAR: weatherStr = "晴朗"; break;
        case GameEnums::Weather::CLOUDY: weatherStr = "多云"; break;
        case GameEnums::Weather::RAIN: weatherStr = "雨天"; break;
        case GameEnums::Weather::STORM: weatherStr = "暴风雨"; break;
        case GameEnums::Weather::SNOW: weatherStr = "雪天"; break;
        }

        std::cout << "天气变化: " << seasonStr << " " << weatherStr << std::endl;
        });

    // 设置输入处理器回调
    inputHandler->setConstructionSelectedCallback([this](int x, int y, Constants::ConstructionType type) {
        std::cout << "选择建设: (" << x << ", " << y << ") 类型: " << static_cast<int>(type) << std::endl;
        });

    inputHandler->setCellHoveredCallback([this](int x, int y) {
        Rrender->setHoveredCell(x, y);
        });

    inputHandler->setCellSelectedCallback([this](int x, int y) {
        Rrender->setSelectedCell(x, y);
        });

    inputHandler->setInputMessageCallback([this](const std::string& message) {
        std::cout << "输入消息: " << message << std::endl;
        });

    std::cout << "所有回调设置完成" << std::endl;
}

void GameEngine::run() {
    if (!initialize()) {
        std::cerr << "游戏初始化失败，无法运行" << std::endl;
        return;
    }

    std::cout << "开始游戏主循环" << std::endl;

    sf::Clock frameClock;

    while (window.isOpen() && gameRunning) {
        sf::Time deltaTime = frameClock.restart();
        elapsedTime += deltaTime;

        handleEvents();

        if (!gamePaused) {
            update(deltaTime.asSeconds());
        }

        render();

        // 限制帧率
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }

    cleanup();
}

void GameEngine::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        handleInput(event);

        // 系统事件处理
        switch (event.type) {
        case sf::Event::Closed:
            window.close();
            gameRunning = false;
            break;

        case sf::Event::LostFocus:
            if (currentState == GameState::IN_GAME) {
                pauseGame();
            }
            break;

        case sf::Event::GainedFocus:
            if (gamePaused) {
                resumeGame();
            }
            break;

        case sf::Event::Resized:
            // 调整视图大小
            Rrender->resetView();
            break;

        default:
            break;
        }
    }
}

void GameEngine::handleInput(const sf::Event& event) {
    if (!inputHandler) return;

    inputHandler->handleEvent(event, window);

    // 引擎特定的输入处理
    switch (event.type) {
    case sf::Event::KeyPressed:
        switch (event.key.code) {
        case sf::Keyboard::P:
            if (gamePaused) {
                resumeGame();
            }
            else {
                pauseGame();
            }
            break;

        case sf::Keyboard::Enter:
            if (currentState == GameState::DEPLOYMENT) {
                startTurn();
                //if (currentState == GameState::TURN_END) {
                //endTurn();
                //}
            }
            break;

        case sf::Keyboard::Escape:
            if (currentState == GameState::IN_GAME) {
                pauseGame();
            }
            else if (currentState == GameState::PAUSED) {
                resumeGame();
            }
            else if (currentState == GameState::GAME_OVER) {
                window.close();
            }
            break;

        case sf::Keyboard::F1:
            printGameState();
            break;

        case sf::Keyboard::F2:
            constructionSystem->printActiveTasks();
            break;

        case sf::Keyboard::F3:
            randomEvent->printActiveEvents();
            break;

        case sf::Keyboard::F4:
            inputHandler->printInputState();
            break;

        case sf::Keyboard::F5:
            Rrender->printRenderState();
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

void GameEngine::update(float deltaTime) {
    // 根据游戏状态进行更新
    switch (currentState) {
    case GameState::MAIN_MENU:
        // 主菜单逻辑
        break;

    case GameState::IN_GAME:
        // 游戏进行中逻辑
        inputHandler->update();

        // 检查游戏条件
        checkGameConditions();
        break;

    case GameState::DEPLOYMENT:
        // 部署阶段逻辑
        break;

    case GameState::TURN_PROCESSING:
        // 回合处理中逻辑
        processTurn();
        break;

    case GameState::TURN_END:
        // 回合结束逻辑
        break;

    case GameState::GAME_OVER:
        // 游戏结束逻辑
        break;

    case GameState::PAUSED:
        // 暂停状态逻辑
        break;

    default:
        break;
    }
}

void GameEngine::render() {
    window.clear(sf::Color::Black);

    if (Rrender) {
        Rrender->draw(window);
    }

    // 根据游戏状态渲染不同的UI
    switch (currentState) {
    case GameState::PAUSED:
        // 渲染暂停菜单
    {
        sf::Text pauseText("Paused - press P to continue.", Rrender->Render::Getfont(), 24);
        pauseText.setPosition(Constants::WINDOW_WIDTH / 2 - 100, Constants::WINDOW_HEIGHT / 2);
        pauseText.setFillColor(sf::Color::White);
        window.draw(pauseText);
    }
    break;

    case GameState::GAME_OVER:
        // 渲染游戏结束画面
    {
        sf::RectangleShape overlay(sf::Vector2f(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);

        sf::Text gameOverText(gameWon ? "Game Won!" : "Game Over!", Rrender->Getfont(), 36);
        gameOverText.setPosition(Constants::WINDOW_WIDTH / 2 - 100, Constants::WINDOW_HEIGHT / 2 - 50);
        gameOverText.setFillColor(gameWon ? sf::Color::Green : sf::Color::Red);
        window.draw(gameOverText);

        sf::Text resultText(gameResultMessage, Rrender->Render::Getfont(), 20);
        resultText.setPosition(Constants::WINDOW_WIDTH / 2 - 150, Constants::WINDOW_HEIGHT / 2);
        resultText.setFillColor(sf::Color::White);
        window.draw(resultText);

        sf::Text restartText("ESC", Rrender->Render::Getfont(), 18);
        restartText.setPosition(Constants::WINDOW_WIDTH / 2 - 80, Constants::WINDOW_HEIGHT / 2 + 50);
        restartText.setFillColor(sf::Color::White);
        window.draw(restartText);
    }
    break;

    default:
        break;
    }

    window.display();
}

void GameEngine::startGame() {
    std::cout << "开始新游戏" << std::endl;
    currentState = GameState::DEPLOYMENT;
    currentTurn = 1;
    gameWon = false;
    gameResultMessage = "";

    // 重置所有系统
    mapSystem->initialize();
    resourceManager->initialize();
    constructionSystem->initialize(mapSystem.get(), resourceManager.get(), randomEvent.get());

    std::cout << "游戏开始，当前回合: " << currentTurn << std::endl;
}

void GameEngine::endGame(bool won, const std::string& message) {
    gameWon = won;
    gameResultMessage = message;
    currentState = GameState::GAME_OVER;

    std::cout << "游戏结束: " << (won ? "胜利" : "失败") << " - " << message << std::endl;
}

void GameEngine::pauseGame() {
    if (!gamePaused && currentState == GameState::IN_GAME) {
        gamePaused = true;
        currentState = GameState::PAUSED;
        std::cout << "游戏暂停" << std::endl;
    }
}

void GameEngine::resumeGame() {
    if (gamePaused) {
        gamePaused = false;
        currentState = GameState::IN_GAME;
        std::cout << "游戏继续" << std::endl;
    }
}

void GameEngine::restartGame() {
    std::cout << "重新开始游戏" << std::endl;
    cleanup();
    initialize();
    startGame();
}

void GameEngine::startTurn() {
    if (currentState != GameState::DEPLOYMENT) return;

    std::cout << "=== 回合 " << currentTurn << " 开始 ===" << std::endl;

    currentState = GameState::TURN_PROCESSING;

    // 开始回合处理
    resourceManager->startTurn();
    randomEvent->processTurn();
}

void GameEngine::processTurn() {
    if (currentState != GameState::TURN_PROCESSING) return;

    // 处理建设进度
    constructionSystem->processTurn();

    // 处理资源消耗
    resourceManager->processTurn();

    // 回合处理完成，进入回合结束状态
    currentState = GameState::TURN_END;
    GameEngine::endTurn();
    std::cout << "回合 " << currentTurn << " 处理完成" << std::endl;
}

void GameEngine::endTurn() {
    std::cout << "Ending..." << std::endl;
    if (currentState != GameState::TURN_END) return;

    // 结束当前回合
    resourceManager->endTurn();

    // 检查游戏结束条件
    if (resourceManager->isGameOver()) {
        if (resourceManager->isTimeOut()) {
            endGame(false, "Gameover!");
        }
        else {
            endGame(false, "Gameover(R)");
        }
        return;
    }
    std::cout << "Checking..."<<std::endl;
    checkGameConditions();
    currentTurn++;

    // 进入下一回合的部署阶段
    currentState = GameState::DEPLOYMENT;

    std::cout << "回合 " << currentTurn - 1 << " 结束，进入回合 " << currentTurn << " 部署阶段" << std::endl;
    std::cout << "剩余回合: " << (maxTurns - currentTurn) << std::endl;
}

void GameEngine::checkGameConditions() {
    // 检查资源耗尽
    if (resourceManager->isGameOver()) {
        if (resourceManager->isTimeOut()) {
            endGame(false, "Gameover!");
        }
        else {
            endGame(false, "Gameover(R)");
        }
        return;
    }

    // 检查路径是否完成
    if (constructionSystem->isPathComplete()) {
        endGame(true, "Complete!");
        return;
    }

    // 检查环境严重破坏
    if (resourceManager->isEnvironmentCritical()) {
        endGame(false, "Gameover(E)");
        return;
    }
}

void GameEngine::cleanup() {
    std::cout << "清理游戏资源..." << std::endl;

    // 按依赖顺序清理系统
    inputHandler.reset();
    Rrender.reset();
    randomEvent.reset();
    constructionSystem.reset();
    resourceManager.reset();
    mapSystem.reset();

    if (window.isOpen()) {
        window.close();
    }

    std::cout << "游戏资源清理完成" << std::endl;
}

void GameEngine::printGameState() const {
    std::cout << "=== 游戏状态 ===" << std::endl;
    std::cout << "游戏状态: ";
    switch (currentState) {
    case GameState::MAIN_MENU: std::cout << "主菜单"; break;
    case GameState::IN_GAME: std::cout << "游戏中"; break;
    case GameState::DEPLOYMENT: std::cout << "部署阶段"; break;
    case GameState::TURN_PROCESSING: std::cout << "回合处理中"; break;
    case GameState::TURN_END: std::cout << "回合结束"; break;
    case GameState::GAME_OVER: std::cout << "游戏结束"; break;
    case GameState::PAUSED: std::cout << "暂停"; break;
    }
    std::cout << std::endl;

    std::cout << "游戏运行: " << (gameRunning ? "是" : "否") << std::endl;
    std::cout << "游戏暂停: " << (gamePaused ? "是" : "否") << std::endl;
    std::cout << "当前回合: " << currentTurn << "/" << maxTurns << std::endl;
    std::cout << "游戏胜利: " << (gameWon ? "是" : "否") << std::endl;

    if (resourceManager) {
        std::cout << "预算: " << resourceManager->getBudget() << std::endl;
        std::cout << "工人: " << resourceManager->getAvailableWorkers() << "/" << resourceManager->getTotalWorkers() << std::endl;
        std::cout << "环境: " << resourceManager->getEnvironmentScore() << std::endl;
    }

    if (constructionSystem) {
        std::cout << "活跃任务: " << constructionSystem->getActiveTaskCount() << std::endl;
        std::cout << "路径完成: " << (constructionSystem->isPathComplete() ? "是" : "否") << std::endl;
    }

    if (randomEvent) {
        std::cout << "活跃事件: " << randomEvent->getActiveEvents().size() << std::endl;
    }

    std::cout << "===============" << std::endl;
}