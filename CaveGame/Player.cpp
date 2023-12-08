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
#include "MeshRenderer.h"

#include "ParticleMgr.h"

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

void Player::MoveByView(const glm::vec3& vDelta)
{
	glm::vec3 vWorldDelta = glm::quat(glm::vec3(0.0f, glm::radians(m_cameraAngleAxis.y), 0.0f)) * vDelta;
	m_vAccelation += vWorldDelta;
	m_rendererBodyAngleY = Lerp(m_rendererBodyAngleY, m_cameraAngleAxis.y, DT * 8.0f);
}

void Player::UpdateRenderer()
{
	float rotationFactor = glm::length(m_vVelocity) * sin(m_fMoveTime * 10.0f) * glm::pi<float>() * 0.25f;

	m_rendererBodyAngleY = glm::clamp(m_rendererBodyAngleY, m_cameraAngleAxisSmooth.y - 30.0f, m_cameraAngleAxisSmooth.y + 30.0f);
	m_rendererObj->GetTransform()->SetLocalRotation(glm::quat(glm::vec3(0.0f, glm::radians(m_rendererBodyAngleY - 90.0f), 0.0f)));

	glm::quat headRotation = glm::quat(glm::vec3(glm::radians(90.0f), glm::radians(m_cameraAngleAxisSmooth.y - m_rendererBodyAngleY), 0.0f));
	headRotation = glm::rotate(headRotation, glm::radians(glm::clamp(-m_cameraAngleAxisSmooth.x, -60.0f, 60.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
	m_transformHead->SetLocalRotation(headRotation);
	m_transformHeadOut->SetLocalRotation(headRotation);

	m_transformLArm->SetLocalPosition(glm::vec3(rotationFactor * 120.0f, 0.0f, m_transformLArm->GetLocalPosition().z));
	m_transformLArmOut->SetLocalPosition(glm::vec3(rotationFactor * 120.0f, 0.0f, m_transformLArmOut->GetLocalPosition().z));
	m_transformRArm->SetLocalPosition(glm::vec3(-rotationFactor * 120.0f, 0.0f, m_transformRArm->GetLocalPosition().z));
	m_transformRArmOut->SetLocalPosition(glm::vec3(-rotationFactor * 120.0f, 0.0f, m_transformRArmOut->GetLocalPosition().z));
	m_transformLLeg->SetLocalPosition(glm::vec3(-rotationFactor * 120.0f, m_transformLLeg->GetLocalPosition().y, m_transformLLeg->GetLocalPosition().z));
	m_transformLLegOut->SetLocalPosition(glm::vec3(-rotationFactor * 120.0f, m_transformLLegOut->GetLocalPosition().y, m_transformLLegOut->GetLocalPosition().z));
	m_transformRLeg->SetLocalPosition(glm::vec3(rotationFactor * 120.0f, m_transformRLeg->GetLocalPosition().y, m_transformRLeg->GetLocalPosition().z));
	m_transformRLegOut->SetLocalPosition(glm::vec3(rotationFactor * 120.0f, m_transformRLegOut->GetLocalPosition().y, m_transformRLegOut->GetLocalPosition().z));

	m_transformLArm->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, rotationFactor)));
	m_transformLArmOut->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, rotationFactor)));
	m_transformRArm->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, -rotationFactor)));
	m_transformRArmOut->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, -rotationFactor)));
	m_transformLLeg->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, -rotationFactor)));
	m_transformLLegOut->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, -rotationFactor)));
	m_transformRLeg->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, rotationFactor)));
	m_transformRLegOut->SetLocalRotation(glm::quat(glm::vec3(glm::radians(90.0f), 0.0f, rotationFactor)));
}

Player::Player()
{
	m_rendererObj = Mgr(AssimpMgr)->LoadAllPartsAsGameObj("DefaultWarpShader.glsl", "Player.fbx");
	m_rendererObj->GetTransform()->SetLocalScale(0.0003f);

	m_transformHead = m_rendererObj->FindChildObj("Head")->GetTransform();
	m_transformHeadOut = m_rendererObj->FindChildObj("HeadOut")->GetTransform();
	m_transformLArm = m_rendererObj->FindChildObj("LArm")->GetTransform();
	m_transformLArmOut = m_rendererObj->FindChildObj("LArmOut")->GetTransform();
	m_transformRArm = m_rendererObj->FindChildObj("RArm")->GetTransform();
	m_transformRArmOut = m_rendererObj->FindChildObj("RArmOut")->GetTransform();
	m_transformLLeg = m_rendererObj->FindChildObj("LLeg")->GetTransform();
	m_transformLLegOut = m_rendererObj->FindChildObj("LLegOut")->GetTransform();
	m_transformRLeg = m_rendererObj->FindChildObj("RLeg")->GetTransform();
	m_transformRLegOut = m_rendererObj->FindChildObj("RLegOut")->GetTransform();

	m_fpChangeCamMode[0] = [this]() noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(glm::zero<glm::vec3>());
		m_cameraObj->GetTransform()->SetLocalRotation(glm::identity<glm::quat>());
		};
	m_fpChangeCamMode[1] = [this]() noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(glm::vec3(0.0f, 0.0f, -0.75f));
		m_cameraObj->GetTransform()->SetLocalRotation(glm::identity<glm::quat>());
		};
	m_fpChangeCamMode[2] = [this]() noexcept {
		m_cameraObj->GetTransform()->SetLocalPosition(glm::vec3(0.0f, 0.0f, 0.75f));
		m_cameraObj->GetTransform()->SetLocalRotation(glm::quat(glm::vec3(0.0f, glm::pi<float>(), 0.0f)));
		};

	m_cameraAnchor = make_obj<GameObj>();
	m_cameraAnchor->GetTransform()->SetLocalPosition(glm::vec3(0.0f, 0.1f, 0.0f));

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

	m_vAccelation = glm::zero<glm::vec3>();

	if (KEY_HOLD(GLFW_KEY_A))
	{
		MoveByView(glm::vec3(-1.0f, 0.0f, 0.0f) * 10.0f);
	}
	if (KEY_HOLD(GLFW_KEY_W))
	{
		MoveByView(glm::vec3(0.0f, 0.0f, 1.0f) * 10.0f);
	}
	if (KEY_HOLD(GLFW_KEY_S))
	{
		MoveByView(glm::vec3(0.0f, 0.0f, -1.0f) * 10.0f);
	}
	if (KEY_HOLD(GLFW_KEY_D))
	{
		MoveByView(glm::vec3(1.0f, 0.0f, 0.0f) * 10.0f);
	}
	if (KEY_HOLD(GLFW_KEY_Q))
	{
		pPlayerTrans->AddWorldRotation(-50.f * DT, Y_AXIS);
	}
	if (KEY_HOLD(GLFW_KEY_E))
	{
		pPlayerTrans->AddWorldRotation(50.f * DT, Y_AXIS);
	}
	if (KEY_HOLD(GLFW_KEY_SPACE))
	{
		MoveByView(glm::vec3(0.0f, 1.0f, 0.0f) * 10.0f);
	}
	if (KEY_HOLD(GLFW_KEY_LEFT_SHIFT))
	{
		MoveByView(glm::vec3(0.0f, -1.0f, 0.0f) * 10.0f);
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

	m_vVelocity = m_vVelocity + m_vAccelation * DT;
	float l = glm::length(m_vVelocity);
	if (l > m_vVelocityMax)
		m_vVelocity *= m_vVelocityMax / l;
	if (l > 0.0f)
		m_vVelocity = m_vVelocity - glm::normalize(m_vVelocity) * glm::min(l, 2.0f * DT);

	m_fMoveTime += glm::length(m_vVelocity) * DT;

	glm::vec3 position = GetTransform()->GetLocalPosition();
	GetTransform()->SetLocalPosition(position + m_vVelocity * DT);

	UpdatePlayerCamFpsMode();
	GameObj::Update();
	//TODO юс╫ц
	const auto [target, point] = Mgr(RayCaster)->GetPickedObjAndPoint();
	if (target)
	{
		Mgr(ParticleMgr)->SetParticles(target, 0.1f, point);
	}

	UpdateRenderer();
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


