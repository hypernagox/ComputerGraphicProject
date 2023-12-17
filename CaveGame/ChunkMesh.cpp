#include "pch.h"
#include "ChunkMesh.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Texture.h"
#include "ResMgr.h"
#include "Transform.h"

ChunkMesh::ChunkMesh()
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
        auto& childMesh = child->GetComp<MeshRenderer>()->GetMesh().front();
		auto& v = childMesh->GetVertices();
        auto& i = childMesh->GetIndicies();
        const auto obj_mat = child->GetObjectWorldTransform();
        for (auto& vert : v)
        {
            vert.position = obj_mat * glm::vec4{ vert.position,1.f };
            m_vecChunkVertex.emplace_back(vert);
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

void ChunkMesh::InitChunkMesh(string_view strShaderName) noexcept
{
    m_pChunckMeshShader = Mgr(ResMgr)->GetRes<Shader>(strShaderName);

    m_pChunckMeshShader->Use();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vecChunkVertex.size() * sizeof(Vertex), m_vecChunkVertex.data(), GL_STATIC_DRAW);

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vecChunkIndex.size() * sizeof(GLsizei), m_vecChunkIndex.data(), GL_STATIC_DRAW);

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

void ChunkMesh::OnChunkMeshChanged(const MCTileChunk* pChunk, int chunkX, int chunkZ)
{
    std::cout << "Notify Chunk(" << pChunk << ") to ChunkMesh(" << this << ")\n";

    // Todo: Reconstruct mesh for designated chunk
}
