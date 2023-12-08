#pragma once
#include "GameObj.h"

class Material;
class Shader;

class ChunkMesh
	:public GameObj
{
private:
	vector<Vertex> m_vecChunkVertex;
	vector<GLsizei> m_vecChunkIndex;

	std::vector<GLsizei> m_indexCounts;
	vector<const void*> m_indexOffsets;


	GLuint vao, vbo, ebo;
	GLuint m_numOfVertices = 0;
	GLuint m_numOfIndices = 0;
	shared_ptr<Material> m_pChunckMeshMaterial;
	shared_ptr<Shader> m_pChunckMeshShader;
	
public:
	ChunkMesh();
	~ChunkMesh();

	void MergeMeshData()noexcept;

	void SetChunkMaterial(shared_ptr<Material> pMaterial)noexcept { m_pChunckMeshMaterial = pMaterial; }

	void InitChunkMesh(string_view strShaderName)noexcept;

	void Render()override;
};

