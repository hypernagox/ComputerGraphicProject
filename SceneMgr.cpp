#include "pch.h"
#include "SceneMgr.h"
#include "Scene.h"
#include "GameObj.h"

SceneMgr::SceneMgr()
{
}

SceneMgr::~SceneMgr()
{
}

void SceneMgr::Init()
{
	m_arrScene[etoi(SCENE_TYPE::START)] = make_shared<Scene>();

	m_pCurScene = m_arrScene[etoi(SCENE_TYPE::START)];

	Enter();
}

void SceneMgr::Update()
{
	m_pCurScene->Update();
	m_pCurScene->LateUpdate();
	m_pCurScene->LastUpdate();
}

void SceneMgr::PreFinalUpdate()
{
	m_pCurScene->PreFinalUpdate();
}

void SceneMgr::FinalUpdate()
{
	m_pCurScene->FinalUpdate();
}

void SceneMgr::Render() const
{
	m_pCurScene->PreRender();
	m_pCurScene->Render();
}

void SceneMgr::Enter()
{
	m_pCurScene->Awake();
	m_pCurScene->Start();
}


