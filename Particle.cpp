#include "pch.h"
#include "Particle.h"
#include "PannelUI.h"
#include "Transform.h"

std::mt19937 Particle::g_rng = std::mt19937{ std::random_device{}() };
std::uniform_real_distribution<float> Particle::g_urd = std::uniform_real_distribution<float>{ 0.f, 360.f };

Particle::Particle()
{
}

Particle::~Particle()
{
}

void Particle::Update()
{
	const auto pTrans = m_pPolyForRender->GetTransform();
	m_fLife -= DT;
	m_vVelocity += glm::vec2{ 0.f,1.f } *480.0f * DT;
	m_vMidPos += m_vVelocity * DT;
	m_pPolyForRender->SetUIPosition(m_vMidPos);
	pTrans->SetLocalScale(pTrans->GetLocalScale() - (0.5f * DT));
	//m_pPolyForRender->Update();
	m_pPolyForRender->FinalUpdate();
	//pTrans->FinalUpdate();
}

void Particle::Render()
{
	m_pPolyForRender->Render();
	if (0.f >= m_fLife)
	{
		m_bIsActivate = false;
	}
}

void Particle::ActivateParticle(const glm::vec2& vMidPos_, const glm::vec3& vColor_)
{
	m_bIsActivate = true;
	m_fLife = 2.f;
	m_vMidPos = vMidPos_;
	const float fDeg = glm::radians(g_urd(g_rng));
	m_vVelocity = glm::vec2(std::cosf(fDeg),std::sinf(fDeg)) * g_particleSpeed;
	const glm::vec2 particleLT = m_vMidPos - m_particleSize / 2.f;
	const glm::vec2 particleRB = m_vMidPos + m_particleSize / 2.f;
	m_pPolyForRender = UI::CreateUI<PannelUI>(particleLT, particleRB);
	m_pPolyForRender->SetColor(vColor_);
}