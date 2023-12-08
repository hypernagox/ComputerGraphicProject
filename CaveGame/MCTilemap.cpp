#include "pch.h"
#include "MCTilemap.h"

MCTileChunk::MCTileChunk()
{

}

void MCTileChunk::SetTile(int x, int y, int z, int tile)
{
	tileData[x][y][z] = tile;
}

int MCTileChunk::GetTile(int x, int y, int z) const
{
	return tileData[x][y][z];
}

MCTilemap::MCTilemap()
{

}

void MCTilemap::SetTile(int x, int y, int z, int tile)
{
	constexpr int modulo = MCTileChunk::CHUNK_WIDTH;
	tileChunk[x / modulo][z / modulo].SetTile(x % modulo, y, z % modulo, tile);
}

int MCTilemap::GetTile(int x, int y, int z) const
{
	constexpr int modulo = MCTileChunk::CHUNK_WIDTH;
	return tileChunk[x / modulo][z / modulo].GetTile(x % modulo, y, z % modulo);
}

MCTileChunk* MCTilemap::GetChunk(int x, int z)
{
	return &(tileChunk[x][z]);
}
