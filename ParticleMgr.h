#pragma once
#include "Particle.h"

class ParticleMgr
	:public Singleton<ParticleMgr>
{
	friend class Singleton;
	ParticleMgr();
	~ParticleMgr();
private:
	array<Particle, 1000> m_arrParticles;
	unsigned short m_iCurParticleNum = 0;
public:
	void Init();
	void SetParticles(const glm::vec2& vPos_,const glm::vec3& vColor_,const glm::vec2& vSize_);
	void Update();
};

