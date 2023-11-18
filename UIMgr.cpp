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

const std::function<bool(const UI*, const UI*)> UIMgr::cmpZDepth = [](const UI* a, const UI* b){ return *a  < *b; };

UIMgr::UIMgr()
{
}

UIMgr::~UIMgr()
{
}

void UIMgr::Init()
{
	m_vecUI.reserve(100);
}

void UIMgr::Update()
{
	for (auto& ui : m_vecUI)
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
			(*m_setUI.begin())->SetZDepth();
		}

		if (UI_STATE::ON_CLICK == eCurState)
		{
			(*m_setUI.begin())->DragMoveUI();
			(*m_setUI.begin())->ExecuteOnClickEvent();
		}
		else if (UI_STATE::CLICKED == eCurState)
		{
			(*m_setUI.begin())->ExecuteClickEvent();
		}
	}

	m_setUI.clear();

	for (auto& ui : m_vecUI)
	{
		if (!ui->IsActivate())continue;
		ui->MyPolygon::FinalUpdate();
		m_setUI.emplace(ui.get());
	}
	Mgr(ThreadMgr)->WaitAllJob();
}

void UIMgr::Start()
{
	for (auto& ui : m_vecUI)
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
	sceneData.projMat = sceneData.viewMat = glm::mat4{ 1.f };
	Mgr(Core)->BindUBOData();

	for (auto& ui : m_setUI)
	{
		//ui->ClearComponentWaitFlag();
		ui->Render();
	}

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
		ui->GameObj::Save(std::to_string(cnt++), practicePath / "UI");
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
		m_vecUI.emplace_back(std::move(pObj));
	}
}