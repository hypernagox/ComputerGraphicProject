#pragma once

class GameObj;
class Mesh;
class Material;
class Animator;
struct asAnimationNode;
struct Animation;
class MyPolygon;
struct SimpleVertex;
class Model;

class AssimpMgr
	:public Singleton<AssimpMgr>
{
	friend class Singleton;
	AssimpMgr();
	~AssimpMgr();
private:
	Assimp::Importer m_importer = {};
	vector<std::future<void>> m_wait;
	vector<std::future<void>> m_fpSetBuf;
	//SpinLock m_spinLock = {};
private:
	shared_ptr<Model> ProcessNode(aiNode* pNode, const aiScene* pAiScene);
	shared_ptr<Mesh> ProcessMesh(aiMesh* pAiMesh);
	//void ProcessBone(vector<Vertex>& vertices, aiNode* pNode, const aiScene* pAiScene);
	shared_ptr<Material> ProcessMaterial(aiMesh* pAiMesh,const aiScene* pAiScene);
private:
	shared_ptr<Animation> ProcessAnimation(aiAnimation* pAiAnim,const aiScene* pAiScene);
	shared_ptr<asAnimationNode> ParseAnimNode(shared_ptr<Animation> pAnim, aiNodeAnim* pAnimNode);
	void ReadKeyFrameData(shared_ptr<Animation> pAnim, aiNode* pNode, map<string, shared_ptr<asAnimationNode>>& cache);
private:
	shared_ptr<MyPolygon> ProcessNodePoly(aiNode* pNode, const aiScene* pAiScene);
	std::pair<vector<SimpleVertex>, vector<GLuint>> ProcessMeshPoly(aiMesh* pAiMesh);
public:
	void Init();
	shared_ptr<GameObj> Load(string_view _strShaderName ,string_view _strModelFileName);
	shared_ptr<Model> LoadModel(string_view _strModelFileName);
	shared_ptr<MyPolygon> LoadPoly(string_view _strPolyFileName);
};

