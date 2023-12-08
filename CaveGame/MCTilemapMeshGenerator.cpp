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
#include "ChunkMesh.h"


static constexpr uint MAX_TEXTURE = 36;

void MCTilemapMeshGenerator::CreateMeshAll(MCTilemap* tilemap)
{
    shared_ptr<Material> material[MAX_TEXTURE];

    shared_ptr<ChunkMesh> pChunkDatas[MAX_TEXTURE];

    for (int i = 0; i < MAX_TEXTURE; ++i)
    {
        material[i] = make_shared<Material>();
        material[i]->AddTexture2D("sand.png");
        pChunkDatas[i] = make_shared<ChunkMesh>();
        pChunkDatas[i]->SetChunkMaterial(material[i]);
    }
    
   
    for (int chunkX = 0; chunkX < MCTilemap::CHUNK_SIZE; ++chunkX)
    {
        for (int chunkZ = 0; chunkZ < MCTilemap::CHUNK_SIZE; ++chunkZ)
        {
            for (int textureID = 0; textureID < MAX_TEXTURE; ++textureID)
            {
                shared_ptr<Mesh> mesh = this->CreateMeshFromChunk(tilemap, chunkX, chunkZ);
                shared_ptr<GameObj> terrainObj = GameObj::make_obj();
                terrainObj->GetTransform()->SetLocalScale(0.1f);
                terrainObj->GetTransform()->SetLocalPosition({ chunkX * MCTileChunk::CHUNK_WIDTH * 0.1f, 0.0f, chunkZ * MCTileChunk::CHUNK_WIDTH * 0.1f });
                auto renderer = terrainObj->AddComponent<MeshRenderer>();
                renderer->AddMesh(mesh);
                pChunkDatas[textureID]->AddChild(terrainObj);
            }
        }
    }

    Mgr(EventMgr)->AddEventNeedLock([pChunkDatas]() noexcept
        {   
            const auto curScene = Mgr(SceneMgr)->GetCurScene();
            for (int i = 0; i < MAX_TEXTURE; ++i)
            {
                Mgr(ThreadMgr)->Enqueue(&ChunkMesh::MergeMeshData, pChunkDatas[i].get());
            }
            Mgr(ThreadMgr)->WaitAllJob();
            for (int i = 0; i < MAX_TEXTURE; ++i)
            {
                pChunkDatas[i]->InitChunkMesh("DefaultFogShader.glsl");

                curScene->AddChunkMesh(i, pChunkDatas[i]);
            }
        });
}

shared_ptr<Mesh> MCTilemapMeshGenerator::CreateMeshFromChunk(MCTilemap* tilemap, int chunkX, int chunkZ) noexcept
{
    MCTileChunk* chunk = tilemap->GetChunk(chunkX, chunkZ);
    int offsetX = chunkX * MCTileChunk::CHUNK_WIDTH;
    int offsetZ = chunkZ * MCTileChunk::CHUNK_WIDTH;

    vector<glm::vec3> vertices;
    vector<GLuint> triangles;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;

    static int planeMap[MCTileChunk::CHUNK_WIDTH][MCTileChunk::CHUNK_WIDTH];

    for (int y = 0; y < MCTileChunk::CHUNK_HEIGHT; y++)
    {
        for (int x = 0; x < MCTileChunk::CHUNK_WIDTH; x++)
        {
            for (int z = 0; z < MCTileChunk::CHUNK_WIDTH; z++)
                planeMap[x][z] = tilemap->GetTile(x + offsetX, y, z + offsetZ) ? (y < MCTilemap::MAP_HEIGHT - 1 ? (tilemap->GetTile(x + offsetX, y + 1, z + offsetZ) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTileChunk::CHUNK_WIDTH, MCTileChunk::CHUNK_WIDTH, [&vertices, y](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmin, y + 1, ymin));
                vertices.push_back(glm::vec3(xmax, y + 1, ymin));
                vertices.push_back(glm::vec3(xmin, y + 1, ymax));
                vertices.push_back(glm::vec3(xmax, y + 1, ymax));
            }, glm::vec3(0.0f, 1.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int y = 0; y < MCTileChunk::CHUNK_HEIGHT; y++)
    {
        for (int x = 0; x < MCTileChunk::CHUNK_WIDTH; x++)
        {
            for (int z = 0; z < MCTileChunk::CHUNK_WIDTH; z++)
                planeMap[x][z] = tilemap->GetTile(x + offsetX, y, z + offsetZ) ? (y > 0 ? (tilemap->GetTile(x + offsetX, y - 1, z + offsetZ) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTileChunk::CHUNK_WIDTH, MCTileChunk::CHUNK_WIDTH, [&vertices, y](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmax, y, ymin));
                vertices.push_back(glm::vec3(xmin, y, ymin));
                vertices.push_back(glm::vec3(xmax, y, ymax));
                vertices.push_back(glm::vec3(xmin, y, ymax));
            }, glm::vec3(0.0f, -1.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int x = 0; x < MCTileChunk::CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < MCTileChunk::CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < MCTileChunk::CHUNK_WIDTH; z++)
                planeMap[z][y] = tilemap->GetTile(x + offsetX, y, z + offsetZ) ? (x + offsetX < MCTilemap::MAP_WIDTH - 1 ? (tilemap->GetTile(x + offsetX + 1, y, z + offsetZ) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTileChunk::CHUNK_WIDTH, MCTileChunk::CHUNK_HEIGHT, [&vertices, x](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(x + 1, ymin, xmin));
                vertices.push_back(glm::vec3(x + 1, ymin, xmax));
                vertices.push_back(glm::vec3(x + 1, ymax, xmin));
                vertices.push_back(glm::vec3(x + 1, ymax, xmax));
            }, glm::vec3(1.0f, 0.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int x = 0; x < MCTileChunk::CHUNK_WIDTH; x++)
    {
        for (int y = 0; y < MCTileChunk::CHUNK_HEIGHT; y++)
        {
            for (int z = 0; z < MCTileChunk::CHUNK_WIDTH; z++)
                planeMap[z][y] = tilemap->GetTile(x + offsetX, y, z + offsetZ) ? (x + offsetX > 0 ? (tilemap->GetTile(x + offsetX - 1, y, z + offsetZ) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTileChunk::CHUNK_WIDTH, MCTileChunk::CHUNK_HEIGHT, [&vertices, x](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(x, ymin, xmax));
                vertices.push_back(glm::vec3(x, ymin, xmin));
                vertices.push_back(glm::vec3(x, ymax, xmax));
                vertices.push_back(glm::vec3(x, ymax, xmin));
            }, glm::vec3(-1.0f, 0.0f, 0.0f), vertices, triangles, normals, uvs);
    }

    for (int z = 0; z < MCTileChunk::CHUNK_WIDTH; z++)
    {
        for (int y = 0; y < MCTileChunk::CHUNK_HEIGHT; y++)
        {
            for (int x = 0; x < MCTileChunk::CHUNK_WIDTH; x++)
                planeMap[x][y] = tilemap->GetTile(x + offsetX, y, z + offsetZ) ? (z + offsetZ < MCTilemap::MAP_WIDTH - 1 ? (tilemap->GetTile(x + offsetX, y, z + offsetZ + 1) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTileChunk::CHUNK_WIDTH, MCTileChunk::CHUNK_HEIGHT, [&vertices, z](int xmin, int ymin, int xmax, int ymax)
            {
                vertices.push_back(glm::vec3(xmax, ymin, z + 1));
                vertices.push_back(glm::vec3(xmin, ymin, z + 1));
                vertices.push_back(glm::vec3(xmax, ymax, z + 1));
                vertices.push_back(glm::vec3(xmin, ymax, z + 1));
            }, glm::vec3(0.0f, 0.0f, 1.0f), vertices, triangles, normals, uvs);
    }

    for (int z = 0; z < MCTileChunk::CHUNK_WIDTH; z++)
    {
        for (int y = 0; y < MCTileChunk::CHUNK_HEIGHT; y++)
        {
            for (int x = 0; x < MCTileChunk::CHUNK_WIDTH; x++)
                planeMap[x][y] = tilemap->GetTile(x + offsetX, y, z + offsetZ) ? (z + offsetZ > 0 ? (tilemap->GetTile(x + offsetX, y, z + offsetZ - 1) ? 0 : 1) : 1) : 0;
        }

        AddPlaneGreedyMesh(planeMap, MCTileChunk::CHUNK_WIDTH, MCTileChunk::CHUNK_HEIGHT, [&vertices, z](int xmin, int ymin, int xmax, int ymax)
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
        v.normal = normals[index];
        v.color = glm::vec4{ glm::abs(v.normal),1.f };
        v.uv = uvs[index];
        sVertices.push_back(v);
    }

    shared_ptr<Mesh> mesh = make_shared<Mesh>(sVertices, triangles);
    return mesh;
}

void MCTilemapMeshGenerator::AddPlaneGreedyMesh(int map[][MCTileChunk::CHUNK_WIDTH], int mapWidth, int mapHeight, function<void(int, int, int, int)>&& vertexAddCallback, glm::vec3 normal, vector<glm::vec3>& vertices, vector<GLuint>& triangles, vector<glm::vec3>& normals, vector<glm::vec2>& uvs)
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
                triangles.push_back(triangleIndex + 2);
                triangles.push_back(triangleIndex + 1);
                triangles.push_back(triangleIndex + 3);
                triangles.push_back(triangleIndex + 1);
                triangles.push_back(triangleIndex + 2);

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
