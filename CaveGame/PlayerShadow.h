#pragma once
#include "GameObj.h"

class PlayerShadow:
	public GameObj
{
private:
	float m_fPlayerOriginY = 0.f;
	float m_fShadowAlpha = 1.f;
public:
	PlayerShadow();
	~PlayerShadow();

	void Start()override;

	void Update()override;
	void Render()override;
};

