#pragma once

class PannelUI;

class Particle
{
	static constexpr float g_particleSpeed = 60.0f;
	static std::mt19937 g_rng;
	static std::uniform_real_distribution<float> g_urd;
private:
	glm::vec2 m_particleSize = { 10.f,10.f };
	glm::vec2 m_vMidPos;
	glm::vec2 m_vVelocity;
	shared_ptr<PannelUI> m_pPolyForRender;
	bool m_bIsActivate = false;
	float m_fLife = 0.f;
public:
	Particle();
	~Particle();
	Particle(const Particle&) = delete;
	Particle operator = (const Particle&) = delete;
	void SetParticleSize(const glm::vec2& vSize_) { m_particleSize = vSize_; }
	void ActivateParticle(const glm::vec2& vMidPos_,const glm::vec3& vColor_);
	bool IsActivate()const { return m_bIsActivate; }
	void Update();
	void Render();
};

