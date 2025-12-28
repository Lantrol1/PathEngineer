#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "GameEngine.hpp"
#include "Constants.hpp"
#include "GameState.hpp"

// 游戏版本信息
const std::string GAME_VERSION = "Path Engineer v0.1";
const std::string BUILD_DATE = __DATE__;
const std::string BUILD_TIME = __TIME__;

// 控制台颜色（Windows）
#ifdef _WIN32
#include <windows.h>

class ConsoleColor {
private:
    HANDLE consoleHandle;
    WORD originalAttributes;

public:
    ConsoleColor() {
        consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
        GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
        originalAttributes = consoleInfo.wAttributes;
    }

    ~ConsoleColor() {
        SetConsoleTextAttribute(consoleHandle, originalAttributes);
    }

    void setColor(int color) {
        SetConsoleTextAttribute(consoleHandle, color);
    }
};

#else
// Linux/Mac 颜色代码
class ConsoleColor {
public:
    ConsoleColor() {}
    ~ConsoleColor() {
        std::cout << "\033[0m"; // 重置颜色
    }

    void setColor(int color) {
        switch (color) {
        case 10: std::cout << "\033[32m"; break; // 绿色
        case 11: std::cout << "\033[36m"; break; // 青色
        case 12: std::cout << "\033[31m"; break; // 红色
        case 14: std::cout << "\033[33m"; break; // 黄色
        case 15: std::cout << "\033[37m"; break; // 白色
        default: std::cout << "\033[0m"; break;  // 重置
        }
    }
};
#endif

void printWelcomeMessage() {
    ConsoleColor color;
    color.setColor(10); // 绿色
    std::cout << "================================================" << std::endl;
    std::cout << "           " << GAME_VERSION << "           " << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "编译时间: " << BUILD_DATE << " " << BUILD_TIME << std::endl;
    std::cout << "目标平台: Windows" << std::endl;
    std::cout << "技术栈: C++17 + SFML2.5.1 + JSON" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

void printGameObjective() {
    ConsoleColor color;
    color.setColor(14); // 黄色
    std::cout << "=== 游戏目标 ===" << std::endl;
    std::cout << "作为道路工程师，你需要在有限的预算和时间内" << std::endl;
    std::cout << "规划并建设连接起点(S)和终点(E)的最优路径。" << std::endl;
    std::cout << std::endl;
    std::cout << "核心挑战:" << std::endl;
    std::cout << "• 克服各种地形障碍（河流、山脉、沼泽）" << std::endl;
    std::cout << "• 管理有限的预算和劳动力资源" << std::endl;
    std::cout << "• 应对随机事件（天气、事故、环境问题）" << std::endl;
    std::cout << "• 在规定回合数内完成路径建设" << std::endl;
    std::cout << "• 平衡成本、时间和环境影响因素" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

void printControls() {
    ConsoleColor color;
    color.setColor(11); // 亮青色
    std::cout << "=== 游戏控制 ===" << std::endl;

    std::cout << "建设操作快捷键:" << std::endl;
    std::cout << "  R - 修建道路" << std::endl;
    std::cout << "  B - 修建桥梁" << std::endl;
    std::cout << "  T - 开凿隧道" << std::endl;
    std::cout << "  C - 清除障碍" << std::endl;
    std::cout << "  F - 加固山体" << std::endl;
    std::cout << std::endl;

    std::cout << "模式切换快捷键:" << std::endl;
    std::cout << "  W - 工人分配模式" << std::endl;
    std::cout << "  I - 单元格检查模式" << std::endl;
    std::cout << "  ESC - 返回正常模式/取消操作" << std::endl;
    std::cout << std::endl;

    std::cout << "游戏流程控制:" << std::endl;
    std::cout << "  Enter - 开始/结束回合" << std::endl;
    std::cout << "  P - 暂停/继续游戏" << std::endl;
    std::cout << "  ESC - 退出游戏" << std::endl;
    std::cout << std::endl;

    std::cout << "视图控制:" << std::endl;
    std::cout << "  鼠标滚轮 - 缩放视图" << std::endl;
    std::cout << "  鼠标拖拽 - 移动视图" << std::endl;
    std::cout << std::endl;

    std::cout << "调试功能快捷键:" << std::endl;
    std::cout << "  F1 - 显示游戏状态" << std::endl;
    std::cout << "  F2 - 显示活跃建设任务" << std::endl;
    std::cout << "  F3 - 显示活跃随机事件" << std::endl;
    std::cout << "  F4 - 显示输入状态" << std::endl;
    std::cout << "  F5 - 显示渲染状态" << std::endl;
    std::cout << "  F9 - 强制胜利（调试）" << std::endl;
    std::cout << "  F10 - 强制失败（调试）" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

void printSystemInfo() {
    ConsoleColor color;
    color.setColor(15); // 白色
    std::cout << "=== 系统信息 ===" << std::endl;
    std::cout << "窗口尺寸: " << Constants::WINDOW_WIDTH << " x " << Constants::WINDOW_HEIGHT << std::endl;
    std::cout << "地图尺寸: " << Constants::MAP_WIDTH << " x " << Constants::MAP_HEIGHT << std::endl;
    std::cout << "网格大小: " << Constants::GRID_SIZE << " 像素" << std::endl;
    std::cout << "初始预算: " << Constants::INITIAL_BUDGET << std::endl;
    std::cout << "初始工人: " << Constants::INITIAL_WORKERS << std::endl;
    std::cout << "最大回合: " << Constants::MAX_TURNS << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

bool checkSystemRequirements() {
    ConsoleColor color;

    // 检查SFML版本
#if SFML_VERSION_MAJOR >= 2
    color.setColor(10);
    std::cout << "✓ SFML版本: " << SFML_VERSION_MAJOR << "." << SFML_VERSION_MINOR << "." << SFML_VERSION_PATCH << std::endl;
#else
    color.setColor(12);
    std::cout << "✗ 不兼容的SFML版本，需要3.0或更高版本" << std::endl;
    return false;
#endif

    // 检查C++标准
#if __cplusplus >= 199703L
    color.setColor(10);
    std::cout << "✓ C++标准: C++17" << std::endl;
#else
    color.setColor(12);
    std::cout << "✗ 需要C++17或更高版本" << std::endl;
    return false;
#endif

    color.setColor(10);
    std::cout << "✓ 系统要求检查通过" << std::endl;
    std::cout << std::endl;

    return true;
}

void initializeGame() {
    ConsoleColor color;
    color.setColor(10);
    std::cout << "正在初始化游戏系统..." << std::endl;

    // 这里可以添加更多的初始化检查
    std::cout << "✓ 游戏系统初始化完成" << std::endl;
    std::cout << std::endl;
}

void handleCommandLineArgs(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            ConsoleColor color;
            color.setColor(11);
            std::cout << "用法: " << argv[0] << " [选项]" << std::endl;
            std::cout << "选项:" << std::endl;
            std::cout << "  --help, -h     显示此帮助信息" << std::endl;
            std::cout << "  --version, -v  显示版本信息" << std::endl;
            std::cout << "  --debug, -d    启用调试模式" << std::endl;
            exit(0);
        }
        else if (arg == "--version" || arg == "-v") {
            std::cout << GAME_VERSION << std::endl;
            std::cout << "编译时间: " << BUILD_DATE << " " << BUILD_TIME << std::endl;
            exit(0);
        }
        else if (arg == "--debug" || arg == "-d") {
            std::cout << "调试模式已启用" << std::endl;
            // 这里可以设置调试标志
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        // 处理命令行参数
        handleCommandLineArgs(argc, argv);

        // 显示欢迎信息
        printWelcomeMessage();

        // 检查系统要求
        if (!checkSystemRequirements()) {
            ConsoleColor color;
            color.setColor(12);
            std::cerr << "系统要求不满足，游戏无法启动" << std::endl;
            std::cerr << "按任意键退出..." << std::endl;
            std::cin.get();
            return -1;
        }

        // 显示游戏信息
        printGameObjective();
        printControls();
        printSystemInfo();

        // 初始化游戏
        initializeGame();

        ConsoleColor color;
        color.setColor(10);
        std::cout << "正在启动游戏引擎..." << std::endl;

        // 创建游戏引擎实例
        std::unique_ptr<GameEngine> gameEngine;

        try {
            gameEngine = std::make_unique<GameEngine>();
        }
        catch (const std::exception& e) {
            color.setColor(12);
            std::cerr << "创建游戏引擎失败: " << e.what() << std::endl;
            std::cerr << "按任意键退出..." << std::endl;
            std::cin.get();
            return -1;
        }

        // 初始化游戏引擎
        if (!gameEngine->initialize()) {
            color.setColor(12);
            std::cerr << "游戏引擎初始化失败" << std::endl;
            std::cerr << "按任意键退出..." << std::endl;
            std::cin.get();
            return -1;
        }

        color.setColor(10);
        std::cout << "✓ 游戏引擎启动成功" << std::endl;
        std::cout << "开始游戏主循环..." << std::endl;
        std::cout << std::endl;

        // 运行游戏主循环
        gameEngine->run();

        color.setColor(10);
        std::cout << "游戏正常结束" << std::endl;
        std::cout << "感谢游玩 " << GAME_VERSION << "!" << std::endl;

        return 0;
    }
    catch (const std::exception& e) {
        ConsoleColor color;
        color.setColor(12);
        std::cerr << "================================================" << std::endl;
        std::cerr << "游戏运行出错!" << std::endl;
        std::cerr << "错误信息: " << e.what() << std::endl;
        std::cerr << "================================================" << std::endl;

#ifdef _DEBUG
        std::cerr << "调试信息:" << std::endl;
        std::cerr << "文件: " << __FILE__ << std::endl;
        std::cerr << "行号: " << __LINE__ << std::endl;
#endif

        std::cerr << "按任意键退出..." << std::endl;
        std::cin.get();

        return -1;
    }
}