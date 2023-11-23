#pragma once

class MCTilemap
{
public:
	static constexpr int MAP_WIDTH = 64;
	static constexpr int MAP_HEIGHT = 16;

private:
	int tilemapData[MAP_WIDTH][MAP_HEIGHT][MAP_WIDTH];

public:
	MCTilemap();

	void SetTile(int x, int y, int z, int tile);
	int GetTile(int x, int y, int z) const;
};

