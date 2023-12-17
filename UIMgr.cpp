#include "pch.h"
#include "UIMgr.h"
#include "UI.h"
#include "PannelUI.h"
#include "KeyMgr.h"
#include "Transform.h"
#include "TimeMgr.h"
#include "ThreadMgr.h"
#include "Core.h"
#include "Camera.h"
#include "ResMgr.h"
#include "Shader.h"
#include "PannelUI.h"
#include "EventMgr.h"

const std::function<bool(const UI*, const UI*)> UIMgr::cmpZDepth = [](const UI* a, const UI* b){ return *a  < *b; };

extern bool g_bCanResume;

UIMgr::UIMgr()
{
}

UIMgr::~UIMgr()
{
}

void UIMgr::Init()
{
	const auto [width, height] = Mgr(Core)->GetWidthHeight();
	const float fScaleFactor = Mgr(Core)->GetScaleFactor();
	const float w = width / fScaleFactor;
	const float h = height / fScaleFactor;

	{
		const int gridCount = 16; 
		const float gridSizeW = w / gridCount; 
		const float gridSizeH = h / gridCount;

		for (int y = 0; y < gridCount; ++y)
		{
			for (int x = 0; x < gridCount; ++x)
			{
				const glm::vec2 pos(gridSizeW * x, gridSizeH * y); 
				auto background = make_shared<PannelUI>(pos, "introBackGround.png", 13.f);
				m_vecUI[etoi(SCENE_TYPE::INTRO)].emplace_back(background);
			}
		}

		auto back_logo = make_shared<PannelUI>(glm::vec2{ w/2.f,(h/2.f) - 100.f}, "Picture1.png", 1.f);
		m_vecUI[etoi(SCENE_TYPE::INTRO)].emplace_back(back_logo);

		auto gui1 = make_shared<PannelUI>(glm::vec2{ w / 2.f,(h / 2.f) + 100.f }, "introGUI_NewGame.png", 3.f, glm::vec2{ 0,1 / 3.f }, glm::vec2{ 1,2 / 3.f });
		m_vecUI[etoi(SCENE_TYPE::INTRO)].emplace_back(gui1);

		gui1->AddClickedEvent([]() {
			ChangeScene(SCENE_TYPE::STAGE, true);
			});
		
		auto gui2 = make_shared<PannelUI>(glm::vec2{ w / 2.f,(h / 2.f) + 100.f }, "introGUI_NewGame.png", 3.f, glm::vec2{ 0,0 }, glm::vec2{ 1,1 / 3.f });
		
		gui1->AddUIStateTex(UI_STATE::ON_MOUSE, gui2);
		
		{
			auto gui3 = make_shared<PannelUI>(glm::vec2{ w / 2.f,(h / 2.f) + 200.f }, "introGUI_Resume.png", 3.f, glm::vec2{ 0,1 / 3.f }, glm::vec2{ 1,2 / 3.f });
			m_vecUI[etoi(SCENE_TYPE::INTRO)].emplace_back(gui3);

			gui3->AddClickedEvent([]() {
				if (g_bCanResume)
				{
					ChangeScene(SCENE_TYPE::STAGE, false);
				}
				});

			auto gui4 = make_shared<PannelUI>(glm::vec2{ w / 2.f,(h / 2.f) + 200.f }, "introGUI_Resume.png", 3.f, glm::vec2{ 0,0 }, glm::vec2{ 1,1 / 3.f });

			gui3->AddUIStateTex(UI_STATE::ON_MOUSE, gui4);
		}

		auto gui3 = make_shared<PannelUI>(glm::vec2{ w / 2.f,(h / 2.f) + 300.f }, "introGUI_Quit.png", 3.f, glm::vec2{ 0,1 / 3.f }, glm::vec2{ 1,2 / 3.f });
		m_vecUI[etoi(SCENE_TYPE::INTRO)].emplace_back(gui3);

		gui3->AddClickedEvent([]() {
			UnLoadScene();
			});

		auto gui4 = make_shared<PannelUI>(glm::vec2{ w / 2.f,(h / 2.f) + 300.f }, "introGUI_Quit.png", 3.f, glm::vec2{ 0,0 }, glm::vec2{ 1,1 / 3.f });

		gui3->AddUIStateTex(UI_STATE::ON_MOUSE, gui4);
	}

	{
		auto quick_bar = make_shared<PannelUI>(glm::vec2{ w / 2.f,h - 35.f }, "gui.png", 3.f);
		m_vecUI[etoi(SCENE_TYPE::STAGE)].emplace_back(quick_bar);

		auto cross_line = make_shared<PannelUI>(glm::vec2{ w,h } / 2.f, "cross.png", 2.f);
		m_vecUI[etoi(SCENE_TYPE::STAGE)].emplace_back(cross_line);
	}


	//auto temp = make_shared<PannelUI>(glm::vec2{ 10,10 }, "cross.png", 1.f);
	//quick_bar->AddChild(temp);
}

void UIMgr::Update()
{
	for (auto& ui : m_vecUI[etoi(m_eCurUIScene)])
	{
		std::ranges::for_each(*ui, std::mem_fn(&UI::Update));

		const UI_STATE eCurState = ui->GetCurUIState();

		if (UI_STATE::ON_CLICK == eCurState ||
			UI_STATE::CLICKED == eCurState)
		{
			m_setUI.emplace(ui.get());
		}
	}

	if (!m_setUI.empty())
	{
		const UI_STATE eCurState = (*m_setUI.begin())->GetCurUIState();

		if ((UI_STATE::ON_CLICK == eCurState && KEY_TAP(GLFW_MOUSE_BUTTON_LEFT)) ||
			UI_STATE::CLICKED == eCurState)
		{
			//(*m_setUI.begin())->SetZDepth();
		}

		if (UI_STATE::ON_CLICK == eCurState)
		{
			//(*m_setUI.begin())->DragMoveUI();
			(*m_setUI.begin())->ExecuteOnClickEvent();
		}
		else if (UI_STATE::CLICKED == eCurState)
		{
			(*m_setUI.begin())->ExecuteClickEvent();
		}
	}

	m_setUI.clear();

	for (auto& ui : m_vecUI[etoi(m_eCurUIScene)])
	{
		if (!ui->IsActivate())continue;
		ui->FinalUpdate();
		m_setUI.emplace(ui.get());
	}
	Mgr(ThreadMgr)->WaitAllJob();
}

void UIMgr::Start()
{
	for (auto& ui : m_vecUI | std::views::join)
	{
		try {
			if (!ui->GetParentGameObj().expired())
				throw std::runtime_error("Dont Add Child in UIMgr");
		}
		catch (const std::runtime_error& e) {
			std::cerr << e.what();
			exit(0);
		}
		ui->Awake();
		ui->Start();
		ui->InitGameObj();
	}
}

void UIMgr::Render()
{
	glClear(GL_DEPTH_BUFFER_BIT);

	static UBOData& sceneData = Mgr(Core)->GetUBOData();
	sceneData.viewMat = glm::mat4{ 1.f };

	const auto [width, height] = Mgr(Core)->GetWidthHeight();
	sceneData.projMat = glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	sceneData.viewMat = glm::mat4{ 1.f };
	
	Mgr(Core)->BindUBOData();

	Mgr(ResMgr)->GetRes<Shader>("UIShader.glsl")->Use();


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto& ui : m_setUI)
	{
		//ui->ClearComponentWaitFlag();
		ui->Render();
	}

	glDisable(GL_BLEND);

	//insertionSort(m_vecUI, [](const shared_ptr<PannelUI>& a, const shared_ptr<PannelUI>& b) {return *a < *b; });
	
	/*for (auto& ui : m_vecUI)
	{
		ui->Render();
	}*/

	m_setUI.clear();
}

void UIMgr::SaveForPractice(string_view _strPracticeName)
{
	const auto practicePath = Mgr(PathMgr)->GetSavePath() / _strPracticeName;
	fs::create_directory(practicePath);

	int cnt = 0;

	for (const auto& ui : m_vecUI)
	{
		//ui->GameObj::Save(std::to_string(cnt++), practicePath / "UI");
	}
}

void UIMgr::LoadForPractice(string_view _strPracticeName)
{
	const auto practicePath = Mgr(PathMgr)->GetSavePath() / _strPracticeName / "UI";

	fs::directory_iterator practiceIter{ practicePath };

	int cnt = 0;

	for (const auto& pracIter : practiceIter)
	{
		auto pObj = make_shared<PannelUI>();
		pObj->GameObj::Load(std::to_string(cnt++), practicePath);
		//m_vecUI.emplace_back(std::move(pObj));
	}
}