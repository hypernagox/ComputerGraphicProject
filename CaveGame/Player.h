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

	float m_fMoveSpeed = 1.f;
	float m_fCamSensivity = 0.1f;

	glm::vec3 m_cameraAngleAxis = glm::zero<glm::vec3>();
	glm::vec3 m_cameraAngleAxisSmooth = glm::zero<glm::vec3>();

	ushort m_curCamMode = 0;
private:
	void ChangeCamType()noexcept;
	void UpdatePlayerCamFpsMode()noexcept;
	void InitCamDirection()noexcept;
public:
	Player();
	~Player();

	void Start()override;

	void Update()override;
	
	const glm::vec3 GetPlayerLook()const noexcept;

	void Fire()noexcept;
};

