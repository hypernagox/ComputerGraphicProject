#pragma once

class GameObj;
class Light;
class Collider;

enum class SCENE_ADDED_UPDATE
{
	UPDATE,
	LATEUPDATE,
	LASTUPDATE,

	PRERENDER,
	RENDER,

	END
};

class SkyBox;

class Scene
{
private:
	vector<shared_ptr<GameObj>> m_vecObj[etoi(GROUP_TYPE::END)];
	vector<weak_ptr<Light>> m_vecLights;
	shared_ptr<SkyBox> m_skyBox;
	array<Delegate, etoi(SCENE_ADDED_UPDATE::END)> m_arrAddedUpdateFp;
	//int m_iNumOfJob = 0;
public:
	vector<shared_ptr<GameObj>>& GetGroupObj(GROUP_TYPE _eType) { return m_vecObj[etoi(_eType)]; }
	Scene();
	virtual ~Scene();
	void AddObject(shared_ptr<GameObj> _pObj, GROUP_TYPE _eType);

	void Awake();
	void Start();

	void Update();
	void LateUpdate();
	void LastUpdate();

	void PreFinalUpdate();

	void FinalUpdate();

	void PreRender();
	void Render();

	void AddLights(weak_ptr<Light> _pLight) { m_vecLights.emplace_back(std::move(_pLight)); }

	template<typename Func, typename... Args> requires std::invocable<Func, Args...>
	void AddUpdateFp(SCENE_ADDED_UPDATE _eUpdateType,Func&& fp, Args&&... args)
	{
		m_arrAddedUpdateFp[etoi(_eUpdateType)] += [fp = std::forward<Func>(fp), ...args = std::forward<Args>(args)]()mutable {std::invoke(std::forward<Func>(fp), std::forward<Args>(args)...); };
	}

	void SaveForPractice(string_view _strPracticeName);
	void LoadForPractice(string_view _strPracticeName);
	//void WaitUpdate();
	void SetSkyBox(SKYBOX_TYPE _eSkyBoxType,string_view _strSkyBoxName, string_view _strTextureName);
};

