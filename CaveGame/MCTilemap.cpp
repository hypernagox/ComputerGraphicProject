#include "pch.h"
#include "MCTilemap.h"

MCTilemap::MCTilemap()
{

}

void MCTilemap::SetTile(int x, int y, int z, int tile)
{
	tilemapData[x][y][z] = tile;
}

int MCTilemap::GetTile(int x, int y, int z) const
{
	return tilemapData[x][y][z];
}
