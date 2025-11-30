#pragma once
//一些常量定义
namespace Constants {
	//窗口设置
	const int WINDOW_WIDTH = 1920;
	const int WINDOW_HEIGHT = 1080;
	const int GRID_SIZE = 100;//网格边长（未确定）

	//建设成本
	const double COST_ROAD_PLAIN = 10.0;//平地修路
	const double COST_BRIDGE = 50.0;//水上架桥
	const double COST_TUNNEL = 100.0;//隧道

	//地形类型
	enum class TerrainType {
		PLAIN,
		RIVER,
		MOUNTAIN,
		SWAMP//沼泽
	};
	//enum class：枚举定义常量（plain为0，river为1，以此类推），也就是单词可以直接对应到二维数组中的存储格式。
}