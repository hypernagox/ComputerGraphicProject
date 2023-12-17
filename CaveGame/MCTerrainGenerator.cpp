#include "pch.h"
#include <random>
#include "MCTerrainGenerator.h"
#include "MCTilemap.h"
#include "PerlinNoise.hpp"

void MCTerrainGenerator::Generate(MCTilemap* tilemap)
{
	std::default_random_engine dre{ (unsigned int)time(NULL) };
	std::uniform_int_distribution uid{ 0, MCTilemap::MAP_WIDTH };

	int x0 = uid(dre) % 1000;
	int y0 = uid(dre) % 1000;
	int z0 = uid(dre) % 1000;

	for (int x = 0; x < MCTilemap::MAP_WIDTH; ++x)
	{
		for (int z = 0; z < MCTilemap::MAP_WIDTH; ++z)
		{
			float yLevel = Perlin::Noise((x0 + x) * 0.02f, (z0 + z) * 0.02f) * 16.0f + 12.0f;
			tilemap->SetTile(x, 0, z, 1);
			for (int y = 1; y < MCTilemap::MAP_HEIGHT; ++y)
			{
				float perlinValue = Perlin::Noise((x0 + x) * 0.1f, (y0 + y) * 0.1f, (z0 + z) * 0.1f);
				int tile = perlinValue * 6.0f + yLevel - y > 0.0f;
				tilemap->SetTile(x, y, z, tile);
			}
		}
	}

	for (int x = 0; x < MCTilemap::MAP_WIDTH; ++x)
	{
		for (int z = 0; z < MCTilemap::MAP_WIDTH; ++z)
		{
			for (int y = 0; y < MCTilemap::MAP_HEIGHT; ++y)
			{
				if (tilemap->GetTile(x, y, z) != 1)
					continue;
				if (y + 1 >= MCTilemap::MAP_HEIGHT || tilemap->GetTile(x, y + 1, z) == 0)
					tilemap->SetTile(x, y, z, 3);
			}
		}
	}

	const char* tree_prefab[5][3] =
	{
		"000",
		"080",
		"000",

		"000",
		"080",
		"000",

		"999",
		"989",
		"999",

		"999",
		"989",
		"999",

		"090",
		"999",
		"090",
	};

	for (int n = 0; n < 512; ++n)
	{
		int x = uid(dre) % MCTilemap::MAP_WIDTH;
		int z = uid(dre) % MCTilemap::MAP_WIDTH;
		int y = MCTilemap::MAP_HEIGHT - 1;
		while (y > 0)
		{
			if (tilemap->GetTile(x, y - 1, z) == 3)
				break;
			y -= 1;
		}

		for (int dx = 0; dx < 3; ++dx)
		{
			for (int dy = 0; dy < 5; ++dy)
			{
				for (int dz = 0; dz < 3; ++dz)
				{
					int xp = x + dx - 1;
					int yp = y + dy;
					int zp = z + dz - 1;

					if (xp < 0 || xp >= MCTilemap::MAP_WIDTH)
						continue;
					if (yp < 0 || yp >= MCTilemap::MAP_HEIGHT)
						continue;
					if (zp < 0 || zp >= MCTilemap::MAP_WIDTH)
						continue;

					int tile = tree_prefab[dy][dx][dz] - '0';
					if (tile == 0)
						continue;

					if (tilemap->GetTile(xp, yp, zp) == 0)
						tilemap->SetTile(xp, yp, zp, tile);
				}
			}
		}
	}
}