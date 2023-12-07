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
	const float sign = 2 == m_curCamMode ? -1.f : 1.f;
	const auto camTrans = GetTransform();

	glm::vec2 offset = Mgr(KeyMgr)->GetMouseDelta() * m_fCamSensivity;

	m_fPitchOffsetAcc = glm::clamp(m_fPitchOffsetAcc + (offset.y * sign), -90.0f, 90.0f);
	offset.y = m_fPitchOffsetAcc - (m_fPitchOffsetAcc - offset.y * sign);

	const glm::quat camQuat = camTrans->GetWorldRotationRecursion();
	const glm::quat pitch = glm::angleAxis(glm::radians(offset.y), GetRightByQuat(camQuat));
	const glm::quat yaw = glm::angleAxis(glm::radians(offset.x), Y_AXIS);
	const glm::vec3 newLookVec = yaw * pitch * GetLookByQuat(camQuat);

	camTrans->SetLocalRotation(glm::quatLookAtLH(glm::normalize(newLookVec), Y_AXIS));
}

void Player::InitCamDirection() noexcept
{
	GetTransform()->SetLookAt(GetPlayerLook());
}

Player::Player()
{
	*static_cast<GameObj*>(this) = *Mgr(AssimpMgr)->Load("SimpleShaderHasColorLight.glsl", "MyCube.fbx");
	auto pCam = make_shared<Camera>();
	pCam->SetNear(6.f);
	m_arrComp[etoi(COMPONENT_TYPE::CAMERA)] = pCam;
	GetTransform()->SetLocalScale(0.01f);
}

Player::~Player()
{
}

void Player::Start()
{
	GameObj::Start();
	m_fpChangeCamMode[0] = [this]()noexcept {GetComp<Camera>()->SetMainCam(); };
	m_fpChangeCamMode[1] = [this]()noexcept {
		const auto pChild = static_pointer_cast<PlayerCam>(m_vecChildObj.front());
		pChild->SetThisObjMainCam();
		pChild->ReverseCam();
		};
	m_fpChangeCamMode[2] = [this]()noexcept {
		static_pointer_cast<PlayerCam>(m_vecChildObj.front())->ReverseCam();
		};
	InitCamDirection();
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
		m_fpChangeCamMode[m_curCamMode]();
		m_curCamMode = wrapAround(m_curCamMode + 1, 0, 3);
		m_fPitchOffsetAcc = 0.f;
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


