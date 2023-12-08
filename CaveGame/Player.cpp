#include "pch.h"
#include "KeyMgr.h"
#include "TimeMgr.h"
#include "Player.h"
#include "PlayerCam.h"
#include "AssimpMgr.h"
#include "Transform.h"
#include "Camera.h"
#include "AssimpMgr.h"
#include "Bullet.h"
#include "EventMgr.h"
#include "RayCaster.h"

glm::vec3 temp;
void Player::ChangeCamType() noexcept
{

}

void Player::UpdatePlayerCamFpsMode() noexcept
{
	const auto camTrans = m_cameraAnchor->GetTransform();
	glm::vec2 offset = Mgr(KeyMgr)->GetMouseDelta() * m_fCamSensivity;

	m_cameraAngleAxis += glm::vec3(offset.y, offset.x, 0.0f);
	m_cameraAngleAxis.x = glm::clamp(m_cameraAngleAxis.x, -89.0f, 89.0f);

	m_cameraAngleAxisSmooth = m_cameraAngleAxisSmooth + (m_cameraAngleAxis - m_cameraAngleAxisSmooth) * Mgr(TimeMgr)->GetDT() * 16.0f;
	camTrans->SetLocalRotation(glm::quat(glm::radians(m_cameraAngleAxisSmooth)));
}

void Player::InitCamDirection() noexcept
{
	m_rendererObj->GetTransform()->SetLocalRotation(glm::quat(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f)));
}

Player::Player()
{
	shared_ptr<Material> material = make_shared<Material>();
	material->AddTexture2D("player.png");
	m_rendererObj = Mgr(AssimpMgr)->Load("DefaultShader.glsl", "Player.obj");
	m_rendererObj->GetTransform()->SetLocalScale(0.03f);

	auto renderer = m_rendererObj->GetComp<MeshRenderer>();
	renderer->SetShader("DefaultShader.glsl");
	renderer->AddMaterial(material);

	m_fpChangeCamMode[0] = [this]()noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(glm::zero<glm::vec3>());
		m_cameraObj->GetTransform()->SetLocalRotation(glm::identity<glm::quat>());
		};
	m_fpChangeCamMode[1] = [this]()noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(glm::vec3(0.0f, 0.0f, -0.75f));
		m_cameraObj->GetTransform()->SetLocalRotation(glm::identity<glm::quat>());
		};
	m_fpChangeCamMode[2] = [this]()noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.75f));
		m_cameraObj->GetTransform()->SetLocalRotation(glm::quat(glm::vec3(0.0f, glm::pi<float>(), 0.0f)));
		};

	m_cameraAnchor = make_obj<GameObj>();
	m_cameraAnchor->GetTransform()->SetLocalPosition(glm::vec3(0.0f, 0.2f, 0.0f));

	m_cameraObj = make_obj<GameObj>();
	m_fpChangeCamMode[m_curCamMode]();
	m_cameraObj->GetTransform()->SetLocalScale(0.01f);

	auto pCam = m_cameraObj->AddComponent<Camera>();
	pCam->SetNear(6.0f);
	pCam->SetMainCam();
}

Player::~Player()
{
}

void Player::Start()
{
	GameObj::Start();
	InitCamDirection();

	AddChild(m_rendererObj);
	AddChild(m_cameraAnchor);
	m_cameraAnchor->AddChild(m_cameraObj);
}

void Player::Update()
{
	const auto pPlayerTrans = GetTransform();

	if (KEY_HOLD(GLFW_KEY_A))
	{
		pPlayerTrans->AddLeftRight(-m_fMoveSpeed * DT);
	}
	if (KEY_HOLD(GLFW_KEY_W))
	{
		pPlayerTrans->AddFrontBack(m_fMoveSpeed * DT);
	}
	if (KEY_HOLD(GLFW_KEY_S))
	{
		pPlayerTrans->AddFrontBack(-m_fMoveSpeed * DT);
	}
	if (KEY_HOLD(GLFW_KEY_D))
	{
		pPlayerTrans->AddLeftRight(m_fMoveSpeed * DT);
	}
	if (KEY_HOLD(GLFW_KEY_Q))
	{
		pPlayerTrans->AddWorldRotation(-50.f * DT, Y_AXIS);
	}
	if (KEY_HOLD(GLFW_KEY_E))
	{
		pPlayerTrans->AddWorldRotation(50.f * DT, Y_AXIS);
	}
	if (KEY_HOLD(GLFW_KEY_UP))
	{
		pPlayerTrans->AddUpDown(m_fMoveSpeed * DT);
	}
	if (KEY_HOLD(GLFW_KEY_DOWN))
	{
		pPlayerTrans->AddUpDown(-m_fMoveSpeed * DT);
	}
	if (KEY_TAP(GLFW_KEY_F5))
	{
		m_curCamMode = wrapAround(m_curCamMode + 1, 0, 3);
		m_fpChangeCamMode[m_curCamMode]();
	}
	if (KEY_TAP(GLFW_MOUSE_BUTTON_LEFT))
	{
		Fire();
	}
	UpdatePlayerCamFpsMode();
	GameObj::Update();
}

const glm::vec3 Player::GetPlayerLook() const noexcept
{
	const glm::vec3 worldCoords = NDC2World(ScreenToOpenGL2D(Mgr(KeyMgr)->GetMousePos()));
	const auto pTrans = GetTransform();
	const glm::vec3 curPos = pTrans->GetWorldPositionAccRecursion();
	return glm::normalize(worldCoords - curPos);
}

void Player::Fire()  noexcept
{
	auto bullet = make_shared<Bullet>(GetTransform()->GetWorldPosition(),Mgr(RayCaster)->castRay().rayDir);
	CreateObj(std::move(bullet), GROUP_TYPE::PROJ_PLAYER);
}


