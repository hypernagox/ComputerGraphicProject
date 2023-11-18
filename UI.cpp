#include "pch.h"
#include "Core.h"
#include "UI.h"
#include "Camera.h"
#include "KeyMgr.h"
#include "Transform.h"
#include "PathMgr.h"

float UI::g_curMaxZDepth = 0.f;

bool UI::ptInRect(const glm::vec2& point, const glm::vec2& leftTop, const glm::vec2& rightBottom) {
	return point.x >= leftTop.x && point.x <= rightBottom.x &&
		point.y >= leftTop.y && point.y <= rightBottom.y;
}

SimpleVertex UI::wc2GL(const glm::vec2& point) {
	const auto [width, height] = Mgr(Core)->GetWidthHeight();
	return glm::vec3{
		(point.x / width) * 2.0f - 1.0f, 
		1.0f - (point.y / height) * 2.0f,
		0.0f
	};
}

glm::vec2 UI::gl2WC(const glm::vec3& point) {
	const auto [width, height] = Mgr(Core)->GetWidthHeight();
	return glm::vec2{ point.x + width / 2.f, height / 2.f - point.y };
}

vector<SimpleVertex> UI::makeRect(const glm::vec2& LT, const glm::vec2& RB, glm::vec3& glCenter) {
	vector<SimpleVertex> temp(4);

	temp[0] = wc2GL(LT);           
	temp[1] = wc2GL(glm::vec2(RB.x, LT.y)); 
	temp[2] = wc2GL(RB);        
	temp[3] = wc2GL(glm::vec2(LT.x, RB.y));
	for (const auto& v : temp) {
		glCenter += v.pos;
	}
	glCenter /= static_cast<float>(temp.size());

	for (auto& v : temp) 
	{
		v.pos -= glCenter;
	}

	return temp;
}

UI_STATE UI::UpdateCurUIState()
{
	const auto MousePos = Mgr(KeyMgr)->GetMousePos();
	const bool bIsOnMouse = ptInRect(MousePos, m_arrLTRB[LT], m_arrLTRB[RB]);
	const KEY_STATE MouseStateL = Mgr(KeyMgr)->GetKeyState(GLFW_MOUSE_BUTTON_LEFT);
	const KEY_STATE MouseStateM = Mgr(KeyMgr)->GetKeyState(GLFW_MOUSE_BUTTON_MIDDLE);
	const KEY_STATE MouseStateR = Mgr(KeyMgr)->GetKeyState(GLFW_MOUSE_BUTTON_RIGHT);

	if (bIsOnMouse)
	{
		if (MouseStateL == KEY_STATE::TAP 
			|| MouseStateM == KEY_STATE::TAP 
			|| MouseStateR == KEY_STATE::TAP)
		{
			return UI_STATE::ON_CLICK;
		}
		else if (MouseStateL == KEY_STATE::AWAY
			|| MouseStateM == KEY_STATE::AWAY
			|| MouseStateR == KEY_STATE::AWAY)
		{
			return UI_STATE::CLICKED;
		}
		else
		{
			return UI_STATE::ON_MOUSE;
		}
	}
	else
	{
		return UI_STATE::NONE;
	}
}

UI::UI()
{
}

void UI::Save(string_view _resName, rapidjson::PrettyWriter<rapidjson::StringBuffer>& writer, const fs::path& _savePath)
{
	std::ofstream outFile((_savePath / _resName / _resName).string() + "_UI.json");
	nlohmann::json uiJson;
	uiJson["LTRB"] = nlohmann::json::array();

	for (auto& p : m_arrLTRB)
	{
		uiJson["LTRB"].push_back({ p.x,p.y });
	}

	outFile << uiJson.dump(4);
	
	MyPolygon::Save(_resName, writer,_savePath);
}

void UI::Load(string_view _dirName, const rapidjson::Value& doc, const fs::path& _loadPath)
{
	std::ifstream inFile((_loadPath / _dirName / _dirName).string() + "_UI.json");

	nlohmann::json uiJson;

	inFile >> uiJson;

	int cnt = 0;

	for (const auto& p : uiJson["LTRB"])
	{
		glm::vec2 v;
		v.x = p[0];
		v.y = p[1];
		m_arrLTRB[cnt++] = v;
	}
	glm::vec3 glCenter{};

	makeRect(m_arrLTRB[LT], m_arrLTRB[RB], glCenter);
	GetTransform()->SetLocalPosition(glCenter);
	
	UI::g_curMaxZDepth -= 0.01f;
	m_fCurZDepth = UI::g_curMaxZDepth;

	MyPolygon::Load(_dirName, doc,_loadPath);
}

void UI::SetUIScale(const float _fRatio)
{
	const glm::vec2 center = (m_arrLTRB[LT] + m_arrLTRB[RB]) / 2.f;
	const float w = ((m_arrLTRB[RB].x - m_arrLTRB[LT].x) / 2.f) * _fRatio;
	const float h = ((m_arrLTRB[RB].y - m_arrLTRB[LT].y) / 2.f) * _fRatio;
	m_arrLTRB[LT].x = center.x - w;
	m_arrLTRB[LT].y = center.y - h;
	m_arrLTRB[RB].x = center.x + w;
	m_arrLTRB[RB].y = center.y + h;
}

void UI::SetZDepthUI()
{
	g_curMaxZDepth -= 0.01f;
	m_fCurZDepth = g_curMaxZDepth;
}

UI::UI(const glm::vec2& _LT, const glm::vec2& _RB,glm::vec3 glCenter)
	:MyPolygon{makeRect(_LT,_RB,glCenter)}
	, m_arrLTRB{ {_LT,_RB} }
	, m_fCurZDepth {UI::g_curMaxZDepth -= 0.01f}
{
	try {
		if(_LT.x >= _RB.x || _LT.y >= _RB.y)
			throw std::runtime_error("Point value Error");
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what();
		exit(0);
	}
	GetTransform()->SetLocalPosition(glCenter);
	GetComp<Camera>()->SetCamProjType(PROJECTION_TYPE::ORTHOGRAPHIC);
}

UI::~UI()
{
}

void UI::Update()
{
	m_eCurUIState = UpdateCurUIState();
}
