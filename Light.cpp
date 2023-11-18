#include "pch.h"
#include "Light.h"
#include "GameObj.h"
#include "Transform.h"

Light::Light()
	:Component{COMPONENT_TYPE::LIGHT}
{

}

Light::~Light()
{
}

void Light::FinalUpdate()
{
	const auto lightTrans = GetTransform();

	//lightTrans->GetWaitLock().Wait();

	m_position = lightTrans->GetWorldPosition();

	if (IsZeroVector(m_position))
	{
		m_position = glm::vec3{ 0.f,1.f,0.f };
	}
}

void Light::SetLightPos(const glm::vec3& _position)
{
	GetTransform()->SetLocalPosition(_position);
}

void Light::PushLightData(const GLint _curShaderID,const int _idx) const
{
	const string lightLoc = std::format("lights[{}]", _idx);
	glUniform3fv(glGetUniformLocation(_curShaderID, (lightLoc + ".position").data()), 1, glm::value_ptr(GetLightPos()));
	glUniform3fv(glGetUniformLocation(_curShaderID, (lightLoc + ".ambient").data()), 1, glm::value_ptr(GetLightAmbient()));
	glUniform3fv(glGetUniformLocation(_curShaderID, (lightLoc + ".diffuse").data()), 1, glm::value_ptr(GetLightDiffuse()));
	glUniform3fv(glGetUniformLocation(_curShaderID, (lightLoc + ".specular").data()), 1, glm::value_ptr(GetLightSpecular()));
}