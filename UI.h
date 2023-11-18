#pragma once
#include "MyPolygon.h"
#include "UIMgr.h"

enum class UI_STATE
{
	NONE,
	ON_MOUSE,
	ON_CLICK,
	CLICKED,

	END
};

class MyPolygon;

class UI
	:public MyPolygon
{
public:
	static bool ptInRect(const glm::vec2& point, const glm::vec2& leftTop, const glm::vec2& rightBottom);
	static SimpleVertex wc2GL(const glm::vec2& point);
	static glm::vec2 gl2WC(const glm::vec3& point);
	static vector<SimpleVertex> makeRect(const glm::vec2& LT, const glm::vec2& RB, glm::vec3& glCenter);
protected:
	static float g_curMaxZDepth;
	enum UI_RECT{LT,RB,END};

private:
	UI_STATE	m_eCurUIState = UI_STATE::NONE;
	Delegate m_onClickEvent;
	Delegate m_ClickedEvent;
protected:
	array<glm::vec2, UI_RECT::END> m_arrLTRB;
	float m_fCurZDepth = 0.f;
	bool m_bIsActivate = true;
	UI_STATE UpdateCurUIState();
	UI(const glm::vec2& _LT, const glm::vec2& _RB,glm::vec3 glCenter ={});
protected:
	void Save(string_view _resName, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, const fs::path& _savePath)override;
	
	void Load(string_view _dirName, const rapidjson::Value& doc, const fs::path& _loadPath) override;
	
public:
	void ExecuteOnClickEvent()
	{
		if (m_onClickEvent)
		{
			m_onClickEvent();
		}
	}

	void ExecuteClickedEvent() {
		if (m_ClickedEvent)
		{
			m_ClickedEvent();
		}
	}
	UI();
	void SetUIScale(const float _fRatio);
	void SetZDepthUI();
	virtual ~UI();
	UI(const UI&) = delete;
	UI& operator = (const UI&) = delete;

	template<typename Func, typename... Args>
		requires std::invocable<Func, Args...>
	void AddOnClickEvent(Func&& fp, Args&&... args) { m_onClickEvent += std::bind_front(std::forward<Func>(fp),std::forward<Args>(args)...); }
	template<typename Func, typename... Args>
		requires std::invocable<Func, Args...>
	void AddClickedEvent(Func&& fp, Args&&... args) { m_ClickedEvent += std::bind_front(std::forward<Func>(fp), std::forward<Args>(args)...); }

	void ResetUIState() { m_eCurUIState = UI_STATE::NONE; }
	UI_STATE GetCurUIState()const { return m_eCurUIState; }

	void Update();
	void Render() override = 0;

	auto operator <=> (const UI& _other) const {return m_fCurZDepth <=> _other.m_fCurZDepth;}

	bool IsActivate()const { return m_bIsActivate; }
	void SetActivate(bool b_ = false) { m_bIsActivate = b_; }
	auto GetUILTRB()const { return std::make_pair(m_arrLTRB[LT], m_arrLTRB[RB]); }

	template<typename T>
		requires std::derived_from<T,UI>
	static shared_ptr<T> CreateUI(const glm::vec2& _LT, const glm::vec2& _RB);
};

template<typename T>
	requires std::derived_from<T, UI>
inline shared_ptr<T> UI::CreateUI(const glm::vec2& _LT, const glm::vec2& _RB)
{
	struct make_shared_enabler :public T {
		void Render()override { T::Render(); }
		make_shared_enabler(const glm::vec2& _LT, const glm::vec2& _RB) :T( _LT,_RB ){}
	};
	auto pUI = make_shared<make_shared_enabler>(_LT, _RB);
	pUI->InitGameObj();
	return pUI;
}
