#include "pch.h"
#include "MCTilemap.h"

const int Tile::TEXTURES[][6] = {
	   0, 0, 0, 0, 0, 0,
	   2, 2, 2, 2, 2, 2,
	   3, 3, 3, 3, 3, 3,
	   1, 3, 4, 4, 4, 4,
	   10, 10, 10, 10, 10, 10,
	   12, 12, 12, 12, 12, 12,
	   5, 5, 5, 5, 5, 5,
	   7, 7, 6, 6, 6, 6,
	   15, 15, 14, 14, 14, 14,
	   32, 32, 32, 32, 32, 32,
};

const int Tile::TILE_OPAQUE[] = {
	0, 1, 1, 1, 1, 1, 1, 1, 1, 0
};

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
