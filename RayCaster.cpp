#include "pch.h"
#include "RayCaster.h"
#include "KeyMgr.h"
#include "Camera.h"
#include "Core.h"
#include "SceneMgr.h"
#include "Scene.h"
#include "Collider.h"
#include "GameObj.h"
#include "Transform.h"
#include "ThreadMgr.h"

RayCaster::RayCaster()
{
}

RayCaster::~RayCaster()
{
    m_bStopRequest = true;
    m_bWait.store(false,std::memory_order_seq_cst);
    m_bWait.notify_one();
    m_pickingThread.join();
}

void RayCaster::Init()
{
    m_pickingThread = std::jthread{ [this]()noexcept {
        while (true)
        {
            m_bWait.wait(true);

            if (m_bStopRequest)
            {
                return;
            }
           
            const auto curScene = Mgr(SceneMgr)->GetCurScene();
            m_vecCollider.clear();

            for (int i = 0; i < etoi(GROUP_TYPE::END); ++i)
            {
                for (auto& objs : curScene->GetGroupObj((GROUP_TYPE)i))
                {
                    for (auto& obj : *objs)
                    {
                        if (const auto pCollider = obj->GetComp<Collider>())
                        {
                           m_vecCollider.emplace_back(pCollider.get());
                        }
                    }
                }
            }

            m_bWait.store(true, std::memory_order_seq_cst);
        }
    } };
}

void RayCaster::Update()
{
	if (KEY_TAP(GLFW_MOUSE_BUTTON_LEFT))
	{
        if (!m_bDirty)
        {
            return;
        }

        while (!m_bWait.load(std::memory_order_seq_cst))
        {

        }

        const auto viewMat = Camera::GetMainCamViewMat();
        std::ranges::sort(m_vecCollider, [viewMat](const Collider* const a, const Collider* const b)noexcept {
            return (viewMat * glm::vec4{ a->GetColliderTransform()->GetWorldPosition(),1.f }).z < (viewMat * glm::vec4{ b->GetColliderTransform()->GetWorldPosition(),1.f }).z;
            });
        const auto cache = m_vecCollider.data();
        const ushort num = (const ushort)m_vecCollider.size();
        const Ray curRay = castRay();
        for (ushort i = 0; i < num; ++i)
        {
            if (rayIntersectsOBB(curRay.rayDir, curRay.rayStart, cache[i]->GetOBB()))
            {
                m_pCurPickedObj = cache[i]->GetGameObj();
                break;
            }
        }

        m_bDirty = false;
	}
}

glm::vec2 RayCaster::normalizeDeviceCoordinates(const glm::vec2& vPos_) const noexcept
{
    const auto mPos = vPos_;
	const auto [width, height] = Mgr(Core)->GetWidthHeight();
	const float ndcX = (2.f * mPos.x) / width - 1.f;
	const float ndcY = 1.f - (2.f * mPos.y) / height;
	return glm::vec2{ ndcX,ndcY };
}

Ray RayCaster::castRay() const noexcept
{
    const auto ndc = normalizeDeviceCoordinates(Mgr(KeyMgr)->GetMousePos());
    const auto pCam = Camera::GetCurCam();

    const glm::vec4 rayStartClip(ndc.x, ndc.y, -1.f, 1.f);
    
    const glm::vec4 rayEndClip(ndc.x, ndc.y, 1.f, 1.f);   

    
    glm::mat4 invProjMatrix = glm::inverse(pCam->GetCamMatProj());
    glm::vec4 rayStartView = invProjMatrix * rayStartClip;
    rayStartView /= rayStartView.w;
    glm::vec4 rayEndView = invProjMatrix * rayEndClip;
    rayEndView /= rayEndView.w;

    const glm::mat4 invViewMatrix = glm::inverse(pCam->GetCamMatView());
    const glm::vec3 rayStartWorld = glm::vec3(invViewMatrix * rayStartView);
    const glm::vec3 rayEndWorld = glm::vec3(invViewMatrix * rayEndView);

    
    const glm::vec3 rayDirWorld = glm::normalize(rayEndWorld - rayStartWorld);

    return Ray{ rayDirWorld,rayStartWorld };
}

const bool RayCaster::rayIntersectsOBB(const glm::vec3& rayDir, const glm::vec3& rayStart, const OBBBox& obb) const noexcept
{
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    const glm::vec3 obbCenter = obb.getCenter();
    const glm::vec3 delta = obbCenter - rayStart;

    
    for (int i = 0; i < 3; ++i) {
        const glm::vec3 axis = obb.getAxis(i);
        const float e = glm::dot(axis, delta);
        const float f = glm::dot(rayDir, axis);
        if (std::abs(f) > 0.001f) 
        {

            float t1 = (e + obb.getActualExtents()[i]) / f;
            float t2 = (e - obb.getActualExtents()[i]) / f;

            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax) return false;
        }
        else if (-e - obb.getActualExtents()[i] > 0.0f || -e + obb.getActualExtents()[i] < 0.0f)
        {
            return false;
        }
    }
    return true;
}

void RayCaster::MakeColliderList() noexcept
{
    m_bDirty = true;
    m_bWait.store(false, std::memory_order_seq_cst);
    m_bWait.notify_one();
}

