#include "pch.h"
#include "ParticleMgr.h"
#include "Core.h"

ParticleMgr::ParticleMgr()
{
}

ParticleMgr::~ParticleMgr()
{
}

void ParticleMgr::Init()
{
}

void ParticleMgr::SetParticles(const glm::vec2& vPos_, const glm::vec3& vColor_, const glm::vec2& vSize_)
{
	const unsigned short end_particles = m_iCurParticleNum + 20;
	static const auto cache = m_arrParticles.data();
	for (unsigned short i = m_iCurParticleNum; i < end_particles; ++i)
	{
		cache[i].SetParticleSize(vSize_);
		cache[i].ActivateParticle(vPos_,vColor_);
	}
	m_iCurParticleNum = (m_iCurParticleNum + 20) % 1000;
}

void ParticleMgr::Update()
{
	static const auto cache = m_arrParticles.data();
	for (unsigned short i = 0; i < 1000; ++i)
	{
		if (cache[i].IsActivate())
		{
			cache[i].Update();
		}
	}
	for (unsigned short i = 0; i < 1000; ++i)
	{
		if (cache[i].IsActivate())
		{
			cache[i].Render();
		}
	}
}