//src/core/GameState.cpp
#pragma once
#include"GameState.hpp"

GameStateManager& GameStateManager::getInstance() {
	static GameStateManager instance;
	return instance;
}

void GameStateManager::changeState(GameState::State newState) {
	previousState = currentState;
	currentState = newState;
	//上一个状态<-当前状态, 当前状态<-新状态
	if (newState == GameState::State::PLAYING) {
		//如果进入新游戏那么重置胜负
		gameResult = GameState::GameResult::NONE;
	}
}

GameState::State GameStateManager::getCurrentState() const {
	return currentState;
}

void GameStateManager::setGameResult(GameState::GameResult result) {
	gameResult = result;
}
GameState::GameResult GameStateManager::getGameResult() const {
	return gameResult;
}

void GameStateManager::SwitchToPause() {
	if (currentState == GameState::State::PLAYING) {
		changeState(GameState::State::PAUSED);
	}
	else if (currentState == GameState::State::PAUSED) {
		changeState(GameState::State::PLAYING);
	}
}