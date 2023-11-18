#pragma once

class Scene;
class GameObj;

class SceneMgr
	:public Singleton<SceneMgr>
{
	friend class Singleton;
	SceneMgr();
	~SceneMgr();
private:
	array<shared_ptr<Scene>, etoi(SCENE_TYPE::END)> m_arrScene;
	shared_ptr<Scene> m_pCurScene;

public:
	void Init();
	void Update();

	void PreFinalUpdate();
	void FinalUpdate();

	void Render()const;
	void Enter();

	const shared_ptr<Scene>& GetCurScene()const { return m_pCurScene; }

};

