#pragma once
//定义游戏状态（主菜单/游戏中 or 胜利/失败）
namespace GameState {
	enum class State {
		MAIN_MENU,
		LEVEL_SELECT,
		PLAYING,
		PAUSED,
		GAME_OVER
	};
	enum class Gameresult {
		NONE,
		SUCCESS,
		FAILURE
	};
}