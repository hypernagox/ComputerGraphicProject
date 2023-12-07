#pragma once
#include "pch.h"
#include "MCTilemap.h"

class Mesh;

class MCTilemapMeshGenerator
{
public:
	void CreateMesh(MCTilemap* tilemap)noexcept;
	void AddPlaneGreedyMesh(int map[][MCTilemap::MAP_WIDTH], int mapWidth, int mapHeight, function<void(int, int, int, int)>&& vertexAddCallback, glm::vec3 normal, vector<glm::vec3>& vertices, vector<GLuint>& triangles, vector<glm::vec3>& normals, vector<glm::vec2>& uvs);
};

