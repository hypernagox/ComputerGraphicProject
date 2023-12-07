#include "pch.h"

#include "MCTilemapMeshGenerator.h"
#include "MCTilemap.h"

#include <Vertex.hpp>
#include <Mesh.h>
#include "MeshRenderer.h"
#include "SceneMgr.h"
#include "Scene.h"
#include "GameObj.h"
#include "Material.h"
#include "Transform.h"
#include "EventMgr.h"

void MCTilemapMeshGenerator::CreateMesh(MCTilemap* tilemap)noexcept
{
    vector<glm::vec3> vertices;
    vector<GLuint> triangles;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;

    static int planeMap[MCTilemap::MAP_WIDTH][MCTilemap::MAP_WIDTH];

    for (int y = 0; y < MCTilemap::MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MCTilemap::MAP_WIDTH; x++)
        {
            for (int z = 0; z < MCTilemap::MAP_WIDTH; z++)
                planeMap[x][z] = tilemap->GetTile(x, y, z) ? (y < MCTilemap::MAP_HEIGHT - 1 ? (tilemap->GetTile(x, y + 1, z) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTilemap::MAP_WIDTH, MCTilemap::MAP_WIDTH, [&vertices, y](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmin, y + 1, ymin));
                vertices.push_back(glm::vec3(xmax, y + 1, ymin));
                vertices.push_back(glm::vec3(xmin, y + 1, ymax));
                vertices.push_back(glm::vec3(xmax, y + 1, ymax));
            }, glm::vec3(0.0f, 1.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int y = 0; y < MCTilemap::MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MCTilemap::MAP_WIDTH; x++)
        {
            for (int z = 0; z < MCTilemap::MAP_WIDTH; z++)
                planeMap[x][z] = tilemap->GetTile(x, y, z) ? (y > 0 ? (tilemap->GetTile(x, y - 1, z) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTilemap::MAP_WIDTH, MCTilemap::MAP_WIDTH, [&vertices, y](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmax, y, ymin));
                vertices.push_back(glm::vec3(xmin, y, ymin));
                vertices.push_back(glm::vec3(xmax, y, ymax));
                vertices.push_back(glm::vec3(xmin, y, ymax));
            }, glm::vec3(0.0f, -1.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int x = 0; x < MCTilemap::MAP_WIDTH; x++)
    {
        for (int y = 0; y < MCTilemap::MAP_HEIGHT; y++)
        {
            for (int z = 0; z < MCTilemap::MAP_WIDTH; z++)
                planeMap[z][y] = tilemap->GetTile(x, y, z) ? (x < MCTilemap::MAP_WIDTH - 1 ? (tilemap->GetTile(x + 1, y, z) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTilemap::MAP_WIDTH, MCTilemap::MAP_HEIGHT, [&vertices, x](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(x + 1, ymin, xmin));
                vertices.push_back(glm::vec3(x + 1, ymin, xmax));
                vertices.push_back(glm::vec3(x + 1, ymax, xmin));
                vertices.push_back(glm::vec3(x + 1, ymax, xmax));
            }, glm::vec3(1.0f, 0.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int x = 0; x < MCTilemap::MAP_WIDTH; x++)
    {
        for (int y = 0; y < MCTilemap::MAP_HEIGHT; y++)
        {
            for (int z = 0; z < MCTilemap::MAP_WIDTH; z++)
                planeMap[z][y] = tilemap->GetTile(x, y, z) ? (x > 0 ? (tilemap->GetTile(x - 1, y, z) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTilemap::MAP_WIDTH, MCTilemap::MAP_HEIGHT, [&vertices, x](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(x, ymin, xmax));
                vertices.push_back(glm::vec3(x, ymin, xmin));
                vertices.push_back(glm::vec3(x, ymax, xmax));
                vertices.push_back(glm::vec3(x, ymax, xmin));
            }, glm::vec3(-1.0f, 0.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int z = 0; z < MCTilemap::MAP_WIDTH; z++)
    {
        for (int y = 0; y < MCTilemap::MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MCTilemap::MAP_WIDTH; x++)
                planeMap[x][y] = tilemap->GetTile(x, y, z) ? (z < MCTilemap::MAP_WIDTH - 1 ? (tilemap->GetTile(x, y, z + 1) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTilemap::MAP_WIDTH, MCTilemap::MAP_HEIGHT, [&vertices, z](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmax, ymin, z + 1));
                vertices.push_back(glm::vec3(xmin, ymin, z + 1));
                vertices.push_back(glm::vec3(xmax, ymax, z + 1));
                vertices.push_back(glm::vec3(xmin, ymax, z + 1));
            }, glm::vec3(0.0f, 0.0f, 1.0f), vertices, triangles, normals, uvs);
    }

    for (int z = 0; z < MCTilemap::MAP_WIDTH; z++)
    {
        for (int y = 0; y < MCTilemap::MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MCTilemap::MAP_WIDTH; x++)
                planeMap[x][y] = tilemap->GetTile(x, y, z) ? (z > 0 ? (tilemap->GetTile(x, y, z - 1) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTilemap::MAP_WIDTH, MCTilemap::MAP_HEIGHT, [&vertices, z](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmin, ymin, z));
                vertices.push_back(glm::vec3(xmax, ymin, z));
                vertices.push_back(glm::vec3(xmin, ymax, z));
                vertices.push_back(glm::vec3(xmax, ymax, z));
            }, glm::vec3(0.0f, 0.0f, -1.0f), vertices, triangles, normals, uvs);
    }

    vector<Vertex> sVertices;
    for (int index = 0; index < vertices.size(); ++index)
    {
        Vertex v(vertices[index]);
        // v.position.y = -v.position.y;
        v.normal = normals[index];
        v.color = glm::vec4{ glm::abs(v.normal),1.f };

        v.uv = uvs[index];
        sVertices.push_back(v);
    }

    shared_ptr<Mesh> mesh = make_shared<Mesh>(sVertices, triangles);

    Mgr(EventMgr)->AddEventNeedLock([mesh]()noexcept {
        shared_ptr<Material> material = make_shared<Material>();
        material->AddTexture2D("stone.png");
        shared_ptr<GameObj> terrainObj = GameObj::make_obj();
        auto renderer = terrainObj->AddComponent<MeshRenderer>();
        renderer->AddMesh(mesh);
        renderer->SetShader("DefaultShader.glsl");
        renderer->AddMaterial(material);
        terrainObj->GetTransform()->SetLocalScale(0.1f);
        terrainObj->GetTransform()->SetLocalPosition({ -3.5f,-1.2f,-1.2f });
        mesh->SetBuffers();
        const auto curScene = Mgr(SceneMgr)->GetCurScene();
        curScene->AddObject(terrainObj, GROUP_TYPE::DEFAULT);
        });
      
}

void MCTilemapMeshGenerator::AddPlaneGreedyMesh(int map[][MCTilemap::MAP_WIDTH], int mapWidth, int mapHeight, function<void(int, int, int, int)>&& vertexAddCallback, glm::vec3 normal, vector<glm::vec3>& vertices, vector<GLuint>& triangles, vector<glm::vec3>& normals, vector<glm::vec2>& uvs)
{
    for (int y = 0; y < mapHeight; y++)
    {
        for (int x = 0; x < mapWidth; x++)
        {
            if (map[x][y] != 0)
            {
                int width = 1;

                while (x + width < mapWidth)
                {
                    if (map[x + width][y] == 0)
                        break;
                    width++;
                }

                int height = 1;
                while (y + height < mapHeight)
                {
                    bool condition = true;

                    for (int i = 0; i < width; i++)
                    {
                        if (map[x + i][y + height] == 0)
                        {
                            condition = false;
                            break;
                        }
                    }

                    if (!condition)
                        break;
                    height++;
                }

                for (int i = 0; i < height; i++)
                {
                    for (int j = 0; j < width; j++)
                        map[x + j][y + i] = 0;
                }

                int triangleIndex = static_cast<int>(vertices.size()) / 4 * 4;
                vertexAddCallback(x, y, x + width, y + height);

                triangles.push_back(triangleIndex);
                triangles.push_back(triangleIndex + 1);
                triangles.push_back(triangleIndex + 2);
                triangles.push_back(triangleIndex + 3);
                triangles.push_back(triangleIndex + 2);
                triangles.push_back(triangleIndex + 1);

                normals.push_back(normal);
                normals.push_back(normal);
                normals.push_back(normal);
                normals.push_back(normal);

                uvs.push_back(glm::vec2(0.0f, 0.0f));
                uvs.push_back(glm::vec2(width, 0.0f));
                uvs.push_back(glm::vec2(0.0f, height));
                uvs.push_back(glm::vec2(width, height));
            }
        }
    }
}
