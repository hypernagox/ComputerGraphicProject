#pragma once
#include "GameObj.h"

class Player
	:public GameObj
{
private:
	shared_ptr<GameObj> m_rendererObj;
	shared_ptr<GameObj> m_cameraAnchor;
	shared_ptr<GameObj> m_cameraObj;

	std::function<void(void)> m_fpChangeCamMode[3];

	float m_fMoveSpeed = 10.f;
	float m_fMoveTime = 0.0f;
	float m_fCamSensivity = 0.1f;

	glm::vec3 m_vVelocity = glm::zero<glm::vec3>();
	glm::vec3 m_vAccelation = glm::zero<glm::vec3>();
	float m_vVelocityMax = 1.0f;

	glm::vec3 m_cameraAngleAxis = glm::zero<glm::vec3>();
	glm::vec3 m_cameraAngleAxisSmooth = glm::zero<glm::vec3>();

	shared_ptr<Transform> m_transformHead;
	shared_ptr<Transform> m_transformHeadOut;
	shared_ptr<Transform> m_transformLArm;
	shared_ptr<Transform> m_transformLArmOut;
	shared_ptr<Transform> m_transformRArm;
	shared_ptr<Transform> m_transformRArmOut;
	shared_ptr<Transform> m_transformLLeg;
	shared_ptr<Transform> m_transformLLegOut;
	shared_ptr<Transform> m_transformRLeg;
	shared_ptr<Transform> m_transformRLegOut;

	float m_rendererBodyAngleY = 0.0f;

	ushort m_curCamMode = 0;
private:
	void ChangeCamType()noexcept;
	void UpdatePlayerCamFpsMode()noexcept;
	void InitCamDirection()noexcept;
	void MoveByView(const glm::vec3& vDelta);
	void UpdateRenderer();
public:
	Player();
	~Player();

	void Start()override;

	void Update()override;
	
	const glm::vec3 GetPlayerLook()const noexcept;

	void Fire()noexcept;
};

