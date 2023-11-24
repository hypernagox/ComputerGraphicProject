#pragma once
#include "pch.h"

struct Ray
{
	glm::vec3 rayDir;
	glm::vec3 rayStart;
	glm::vec3 rayEnd;
	Ray(const glm::vec3& dir_,const glm::vec3& start_,const glm::vec3& end_)noexcept
		:rayDir{dir_},rayStart{start_},rayEnd{end_}{}
};

class GameObj;
struct OBBBox;
class Collider;

class RayCaster
	:public Singleton<RayCaster>
{
	friend class Singleton;
	RayCaster();
	~RayCaster();
private:
	shared_ptr<GameObj> m_pCurPickedObj;
	vector<weak_ptr<Collider>> m_vecCollider;
	bool m_bDirty = false;
public:
	void Init();
	void Update()noexcept;
	void AddCollider(weak_ptr<Collider> pCol_)noexcept { m_vecCollider.emplace_back(std::move(pCol_)); }
	glm::vec2 normalizeDeviceCoordinates(const glm::vec2& vPos_)const noexcept;
	Ray castRay()const noexcept;
	const bool rayIntersectsOBB(const glm::vec3& rayDir, const glm::vec3& rayStart, const OBBBox& obb)const noexcept;
	const shared_ptr<GameObj>& GetCurPickedObj()const noexcept{ return m_pCurPickedObj; }
};

