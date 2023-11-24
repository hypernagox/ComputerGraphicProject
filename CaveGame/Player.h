#pragma once
#include "GameObj.h"

class Player
	:public GameObj
{
private:
	std::function<void(void)> m_fpChangeCamMode[3];
	float m_fMoveSpeed = 1.f;
	float m_fCamSensivity = 0.1f;
	float m_fPitchOffsetAcc = 0.f;
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

