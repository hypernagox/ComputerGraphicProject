#pragma once
#include "pch.h"

struct Ray
{
	glm::vec3 rayDir;
	glm::vec3 rayStart;
	Ray(const glm::vec3& dir_,const glm::vec3& point_)
		:rayDir{dir_},rayStart{point_}{}
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
	vector<Collider*> m_vecCollider;
	bool m_bDirty = false;
	std::jthread m_pickingThread;
	std::atomic<bool> m_bWait = true;
	bool m_bStopRequest = false;
public:
	void Init();
	void Update();
	glm::vec2 normalizeDeviceCoordinates(const glm::vec2& vPos_)const noexcept;
	Ray castRay()const noexcept;
	const bool rayIntersectsOBB(const glm::vec3& rayDir, const glm::vec3& rayStart, const OBBBox& obb)const noexcept;
	void MakeColliderList()noexcept;
	const shared_ptr<GameObj>& GetCurPickedObj()const noexcept{ return m_pCurPickedObj; }
	const shared_ptr<GameObj>& GetCurPickedObjNow()noexcept
	{
		Update();
		return m_pCurPickedObj;
	}
};

