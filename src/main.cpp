#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include "GameEngine.hpp"
#include "Constants.hpp"
#include "GameState.hpp"
#include <windows.h>
void printGameObjective() {
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
    std::cout << "=== 游戏控制 ===" << std::endl;

    std::cout << "建设操作快捷键:" << std::endl;
    std::cout << "  R - 修建道路" << std::endl;
    std::cout << "  B - 修建桥梁" << std::endl;
    std::cout << "  T - 开凿隧道" << std::endl;
    std::cout << "  C - 清除障碍" << std::endl;
    std::cout << "  F - 加固山体" << std::endl;
    std::cout << "  W - 工人分配" << std::endl;
    std::cout << "  ESC - 无操作" << std::endl;
    std::cout << std::endl;

    std::cout << "游戏流程控制:" << std::endl;
    std::cout << "  Enter - 下一回合" << std::endl;
    std::cout << "  P - 暂停/继续游戏" << std::endl;
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
    std::cout << "=== 系统信息 ===" << std::endl;
    std::cout << "初始预算: " << Constants::INITIAL_BUDGET << std::endl;
    std::cout << "初始工人: " << Constants::INITIAL_WORKERS << std::endl;
    std::cout << "最大回合: " << Constants::MAX_TURNS << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    try {
        printGameObjective();
        printControls();
        printSystemInfo();
        std::cout << "正在启动游戏" << std::endl;
        std::unique_ptr<GameEngine> gameEngine;
        try {
            gameEngine = std::make_unique<GameEngine>();
        }
        catch (const std::exception& e) {
            std::cerr << "创建游戏引擎失败: " << e.what() << std::endl;
            std::cin.get();
            return -1;
        }
        if (!gameEngine->initialize()) {
            std::cerr << "游戏引擎初始化失败" << std::endl;
            std::cin.get();
            return -1;
        }
        std::cout << "开始游戏" << std::endl;
        std::cout << std::endl;
        gameEngine->run();
        std::cout << "游戏正常结束" << std::endl;
        std::cout << "感谢游玩!" << std::endl;

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "游戏运行出错!" << std::endl;
        std::cerr << "错误信息: " << e.what() << std::endl;
        std::cin.get();
        return -1;
    }
}