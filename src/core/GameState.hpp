#pragma once
//定义游戏状态（主菜单/游戏中 or 胜利/失败）
#include<functional>
#include<unordered_map>

namespace GameState {
	enum class State {
		MAIN_MENU,
		LEVEL_SELECT,
		PLAYING,
		PAUSED,
		GAME_OVER
	};
	enum class GameResult {
		NONE,
		SUCCESS,
		FAILURE
	};
}

class GameStateManager {
public:
	static GameStateManager& getInstance();
	//状态转换
	void changeState(GameState::State newState);
	GameState::State getCurrentState() const;
	//游戏胜负
	void setGameResult(GameState::GameResult result);
	GameState::GameResult getGameResult() const;
	//暂停
	void SwitchToPause();
private:
	GameStateManager() = default;
	GameState::State currentState = GameState::State::MAIN_MENU;//初始状态在主菜单
	GameState::State previousState = GameState::State::MAIN_MENU;//初始状态的上一个状态也初始化为主菜单
	GameState::GameResult gameResult = GameState::GameResult::NONE;//初始胜负未知
};