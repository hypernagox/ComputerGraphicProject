#include "pch.h"
#include "ChunkMesh.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Texture.h"
#include "ResMgr.h"
#include "Transform.h"
#include "MCTilemapMeshGenerator.h"
#include "MCTilemap.h"
#include "ThreadMgr.h"

void ChunkMesh::ReConstructMesh() noexcept
{
    GLsizei currentOffset = 0;
    GLsizei currentOffsetI = 0;
   
    m_vecChunkVertex.clear();
    m_vecChunkIndex.clear();
    m_indexOffsets.clear();
    m_indexCounts.clear();

    for (const auto& chunk : m_vecChunkInfo)
    {
        const shared_ptr<Mesh>& chunkMesh = chunk.refMesh;
        const auto& v = chunkMesh->GetVertices();
        const auto& i = chunkMesh->GetIndicies();

        for (const auto& vert : v)
        {
            Vertex temp = vert;
            temp.position = chunk.worldMat * glm::vec4{ vert.position,1.f };
            m_vecChunkVertex.emplace_back(temp);
        }

        for (const auto index : i)
        {
            m_vecChunkIndex.emplace_back(index + currentOffset);
        }
       
        m_indexOffsets.emplace_back(reinterpret_cast<void*>(static_cast<GLsizei>(currentOffsetI) * sizeof(GLsizei)));
        m_indexCounts.emplace_back(static_cast<GLsizei>(i.size()));

        currentOffset += (GLsizei)v.size();
        currentOffsetI += (GLsizei)i.size();
    }
}

void ChunkMesh::ReBindMesh() noexcept
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vecChunkVertex.size() * sizeof(Vertex), m_vecChunkVertex.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(4);


    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vecChunkIndex.size() * sizeof(GLsizei), m_vecChunkIndex.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    m_numOfVertices = (GLuint)m_vecChunkVertex.size();
    m_numOfIndices = (GLuint)m_vecChunkIndex.size();
}

ChunkMesh::ChunkMesh(MCTilemap* const pTileMap)
    :m_pTileMapForReDrawMesh{pTileMap}
{
    m_vecChunkVertex.reserve(1000000);
    m_vecChunkIndex.reserve(1000000);
}

ChunkMesh::~ChunkMesh()
{
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void ChunkMesh::MergeMeshData() noexcept
{
    GLsizei currentOffset = 0;
    GLsizei currentOffsetI= 0;
    const auto& children = GetChildObj();
    const size_t num_of_child = children.size();
    m_indexOffsets.reserve(num_of_child);
    m_indexCounts.reserve(num_of_child);
	for (const auto& child : children)
	{
        child->GetTransform()->MakeFinalMat();
        const auto& childMesh = child->GetComp<MeshRenderer>()->GetMesh().front();
		const auto& v = childMesh->GetVertices();
        const auto& i = childMesh->GetIndicies();
        const auto obj_mat = child->GetObjectWorldTransform();

        for (const auto& vert : v)
        {
            Vertex temp = vert;
            temp.position = obj_mat * glm::vec4{ vert.position,1.f };
            m_vecChunkVertex.emplace_back(temp);
        }

        for (const auto index : i) 
        {
            m_vecChunkIndex.emplace_back(index + currentOffset);
        }

        m_vecChunkInfo.emplace_back(childMesh, obj_mat);

        m_indexOffsets.emplace_back(reinterpret_cast<void*>(static_cast<GLsizei>(currentOffsetI) * sizeof(GLsizei)));
        m_indexCounts.emplace_back(static_cast<GLsizei>(i.size()));

        currentOffset += (GLsizei)v.size();
        currentOffsetI += (GLsizei)i.size();
	}    
}

void ChunkMesh::InitChunkMesh(string_view strShaderName) noexcept
{
    m_pChunckMeshShader = Mgr(ResMgr)->GetRes<Shader>(strShaderName);

    m_pChunckMeshShader->Use();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vecChunkVertex.size() * sizeof(Vertex), m_vecChunkVertex.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(4);


    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vecChunkIndex.size() * sizeof(GLsizei), m_vecChunkIndex.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(0);

    m_numOfVertices = (GLuint)m_vecChunkVertex.size();
    m_numOfIndices = (GLuint)m_vecChunkIndex.size();
}

void ChunkMesh::Render()
{
    m_pChunckMeshShader->Use();
    glBindVertexArray(vao);
    m_pChunckMeshMaterial->PushMaterialData();
    for (const auto& tex : m_pChunckMeshMaterial->GetTex())
    {
        tex->BindTexture();
    }
    m_pChunckMeshShader->SetUniformMat4(GetObjectWorldTransform(), "uModel");
    glMultiDrawElements(GL_TRIANGLES, m_indexCounts.data(), GL_UNSIGNED_INT, m_indexOffsets.data(), (GLsizei)m_indexCounts.size());
    for (const auto& tex : m_pChunckMeshMaterial->GetTex())
    {
        tex->UnBindTexture();
    }
    glBindVertexArray(0);
}

void ChunkMesh::OnChunkMeshChanged(MCTileChunk* const pChunk, int chunkX, int chunkZ)noexcept
{
    if (m_vecChunkInfo.empty())
        return;

    const auto idx = m_mapChunkToIndex[pChunk];
    shared_ptr<Mesh> mesh = MCTilemapMeshGenerator::CreateMeshFromChunk(m_pTileMapForReDrawMesh, chunkX, chunkZ, m_iChunkTexID);

    const auto& v1 = m_vecChunkInfo[idx].refMesh->GetVertices();
    const auto& i1 = m_vecChunkInfo[idx].refMesh->GetIndicies();
    const auto& v2 = mesh->GetVertices();
    const auto& i2 = mesh->GetIndicies();
    
    if (v1 == v2 && i1 == i2)
        return;
    
    m_vecChunkInfo[idx].refMesh->GetVertices() = std::move(v2);
    m_vecChunkInfo[idx].refMesh->GetIndicies() = std::move(i2);

    m_bDirty = true;
    ReConstructMesh();
}

void ChunkMesh::AddChunk(shared_ptr<GameObj> pChild, MCTileChunk* pChunk) noexcept
{
    m_mapChunkToIndex.emplace(pChunk, (GLuint)GetChildObj().size());
    GameObj::AddChild(pChild);
}

void ChunkMesh::LastUpdate()
{
    GameObj::LastUpdate();
    if (m_bDirty)
    {
        ReBindMesh();
    }
    m_bDirty = false;
}
