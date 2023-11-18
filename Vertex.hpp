#pragma once
#include "pch.h"

struct Vertex
{
	glm::vec3 position = glm::vec3{ 0.f,0.f,0.f };
	glm::vec3 normal = glm::vec3{ 1.f,1.f,1.f };
	glm::vec3 tangent = glm::vec3{ 1.f,1.f,1.f };
	glm::vec2 uv = glm::vec2{ 0.f,0.f };
	glm::vec4 color = glm::vec4{ 0.f,0.f,0.f,0.f };

	//glm::ivec4 boneIds = {};
	//glm::vec4 boneWeights = {};

	Vertex() = default;

	Vertex(const glm::vec3& _v):position{_v}{}
};