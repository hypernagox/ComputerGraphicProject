#pragma once

class UI;
class PannelUI;

class UIMgr
	:public Singleton<UIMgr>
{
	friend class Singleton;
	UIMgr();
	~UIMgr();
	static const std::function<bool(const UI*, const UI*)> cmpZDepth;
private:
	vector<shared_ptr<PannelUI>> m_vecUI;
	std::set<PannelUI*, decltype(cmpZDepth)> m_setUI{cmpZDepth};
public:
	void Init();
	void Start();
	void Update();
	void Render();
	void AddUI(shared_ptr<PannelUI> pUI) { m_vecUI.emplace_back(std::move(pUI)); }
	vector<shared_ptr<PannelUI>>& GetUIVec() { return m_vecUI; }

	void SaveForPractice(string_view _strPracticeName);
	void LoadForPractice(string_view _strPracticeName);
};

