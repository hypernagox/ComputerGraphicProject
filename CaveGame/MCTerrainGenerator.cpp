#include "pch.h"
#include "MCTerrainGenerator.h"
#include "MCTilemap.h"
#include "PerlinNoise.hpp"

void MCTerrainGenerator::Generate(MCTilemap* tilemap)
{
	for (int x = 0; x < MCTilemap::MAP_WIDTH; ++x)
	{
		for (int y = 0; y < MCTilemap::MAP_HEIGHT; ++y)
		{
			for (int z = 0; z < MCTilemap::MAP_WIDTH; ++z)
			{
				float perlinValue = glm::perlin(glm::vec3(x, y, z) * 0.1f);
				int tile = perlinValue * 8.0f + 8.0f - y > 0.0f;
				tilemap->SetTile(x, y, z, tile);
			}
		}
	}
}