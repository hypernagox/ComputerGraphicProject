#pragma once

class MCTileChunk
{
public:
	static constexpr int CHUNK_WIDTH = 16;
	static constexpr int CHUNK_HEIGHT = 16;

private:
	int tileData[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];

public:
	MCTileChunk();

	void SetTile(int x, int y, int z, int tile);
	int GetTile(int x, int y, int z) const;
};

class MCTilemap
{
public:
	static constexpr int MAP_WIDTH = 512;
	static constexpr int MAP_HEIGHT = MCTileChunk::CHUNK_HEIGHT;
	static constexpr int CHUNK_SIZE = (MAP_WIDTH + (MCTileChunk::CHUNK_WIDTH - 1)) / MCTileChunk::CHUNK_WIDTH;

private:
	MCTileChunk tileChunk[CHUNK_SIZE][CHUNK_SIZE];

public:
	MCTilemap();

	void SetTile(int x, int y, int z, int tile);
	int GetTile(int x, int y, int z) const;
	MCTileChunk* GetChunk(int x, int z);
};