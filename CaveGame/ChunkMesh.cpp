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


void ChunkMesh::ReConstructMesh(vector<Vertex>& vert_shrink, vector<GLuint>& idx_shrink) noexcept
{
    m_arrFutureForReConstruct[0] = Mgr(ThreadMgr)->EnqueueTaskFuture([this, &vert_shrink]()noexcept {
        std::lock_guard<std::mutex> lock{ m_mt[0]};
        m_vecChunkVertex.clear();
        vert_shrink.shrink_to_fit();
        for (const auto& chunk : m_vecChunkInfo)
        {
            const shared_ptr<const Mesh>& chunkMesh = chunk.refMesh;
            const auto& v = chunkMesh->GetVertices();

            for (const auto& vert : v)
            {
                m_vecChunkVertex.emplace_back(vert).position = chunk.worldMat * glm::vec4{ vert.position,1.f };
            }
        }
        m_numOfVertices = (GLuint)m_vecChunkVertex.size();
        });

    m_arrFutureForReConstruct[1] = Mgr(ThreadMgr)->EnqueueTaskFuture([this, &idx_shrink]()noexcept {
        std::lock_guard<std::mutex> lock{ m_mt[1]};
        m_vecChunkIndex.clear();
        idx_shrink.shrink_to_fit();
        GLsizei currentOffset = 0;
        GLsizei currentOffsetI = 0;
        GLsizei cnt = 0;
        const auto cache_offset = m_indexOffsets.data();
        const auto cache_cnt = m_indexCounts.data();
        for (const auto& chunk : m_vecChunkInfo)
        {
            const shared_ptr<const Mesh>& chunkMesh = chunk.refMesh;
            const auto& i = chunkMesh->GetIndicies();

            for (const auto index : i)
            {
                m_vecChunkIndex.emplace_back(index + currentOffset);
            }

            cache_offset[cnt] = (reinterpret_cast<void*>(static_cast<GLsizei>(currentOffsetI) * sizeof(GLsizei)));
            cache_cnt[cnt] = (static_cast<GLsizei>(i.size()));

            currentOffset += (GLsizei)m_vecVertexSize[cnt];
            currentOffsetI += (GLsizei)i.size();

            ++cnt;
        }
        m_numOfIndices = (GLuint)m_vecChunkIndex.size();
        });
}

void ChunkMesh::ReBindMesh() noexcept
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_numOfVertices * sizeof(Vertex), m_vecChunkVertex.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_numOfIndices * sizeof(GLsizei), m_vecChunkIndex.data());
}

ChunkMesh::ChunkMesh(const shared_ptr<MCTilemap>& pTileMap)
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
    auto& children = GetChildObj();
    const size_t num_of_child = children.size();
    children.shrink_to_fit();
    m_indexOffsets.reserve(num_of_child);
    m_indexCounts.reserve(num_of_child);
    m_vecChunkInfo.reserve(num_of_child);
    m_vecVertexSize.reserve(num_of_child);
    m_mapChunkToIndex.rehash(num_of_child);
	for (const auto& child : children)
	{
        child->GetTransform()->MakeFinalMat();
        const auto& childMesh = child->GetComp<MeshRenderer>()->GetMesh().front();
		auto& v = childMesh->GetVertices();
        auto& i = childMesh->GetIndicies();
        const auto obj_mat = child->GetObjectWorldTransform();

        for (const auto& vert : v)
        {
            m_vecChunkVertex.emplace_back(vert).position = obj_mat * glm::vec4{ vert.position,1.f };
        }

        for (const auto index : i) 
        {
            m_vecChunkIndex.emplace_back(index + currentOffset);
        }

        m_vecChunkInfo.emplace_back(childMesh, obj_mat);
        m_vecVertexSize.emplace_back(v.size());

        m_indexOffsets.emplace_back(reinterpret_cast<void*>(static_cast<GLsizei>(currentOffsetI) * sizeof(GLsizei)));
        m_indexCounts.emplace_back(static_cast<GLsizei>(i.size()));

        currentOffset += (GLsizei)v.size();
        currentOffsetI += (GLsizei)i.size();

        Mgr(ThreadMgr)->Enqueue([&v, &i]()noexcept { v.shrink_to_fit(); i.shrink_to_fit(); });
	}

    m_vecChunkVertex.shrink_to_fit();
    m_vecChunkIndex.shrink_to_fit();

    m_numOfVertices = (GLuint)m_vecChunkVertex.size();
    m_numOfIndices = (GLuint)m_vecChunkIndex.size();

    m_vecChunkVertex.reserve(m_numOfVertices * 2);
    m_vecChunkIndex.reserve(m_numOfIndices * 2);
}

void ChunkMesh::InitChunkMesh(string_view strShaderName) noexcept
{
    m_pChunckMeshShader = Mgr(ResMgr)->GetRes<Shader>(strShaderName);

    m_pChunckMeshShader->Use();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (m_numOfVertices * 2 + 1024) * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_numOfVertices * sizeof(Vertex), m_vecChunkVertex.data());

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (m_numOfIndices * 2 + 1024) * sizeof(GLsizei), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_numOfIndices * sizeof(GLsizei), m_vecChunkIndex.data());

    glBindVertexArray(0);
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
    if (m_bDirty)
    {
        m_bDirty = false;
        m_arrFutureForReConstruct[1].get(); m_arrFutureForReConstruct[0].get();
        ReBindMesh();
    }
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
    const shared_ptr<Mesh> mesh = MCTilemapMeshGenerator::CreateMeshFromChunk(m_pTileMapForReDrawMesh, chunkX, chunkZ, m_iChunkTexID);

    auto& v1 = m_vecChunkInfo[idx].refMesh->GetVertices();
    auto& i1 = m_vecChunkInfo[idx].refMesh->GetIndicies();

    auto& v2 = mesh->GetVertices();
    auto& i2 = mesh->GetIndicies();
    
    if (v1 == v2 && i1 == i2)
        return;
    
    m_vecVertexSize[idx] = v2.size();

    v1.swap(v2);
    i1.swap(i2);

    m_bDirty = true;
    ReConstructMesh(v1,i1);
}

void ChunkMesh::AddChunk(shared_ptr<GameObj> pChild, MCTileChunk* pChunk) noexcept
{
    m_mapChunkToIndex.emplace(pChunk, (GLuint)GetChildObj().size());
    GameObj::AddChild(pChild);
}
