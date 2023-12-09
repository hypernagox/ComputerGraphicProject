#include "pch.h"
#include "PlayerShadow.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Transform.h"
#include "Core.h"
#include "Shader.h"

PlayerShadow::PlayerShadow()
{
}

PlayerShadow::~PlayerShadow()
{
}

void PlayerShadow::Start()
{
    const int numSegments = 40; 
    const float radius = 1.0f; 
    
    vector<Vertex> vertices;
    vector<GLuint> indices;

    for (int i = 0; i <= numSegments; ++i)
    {
        Vertex v;
        const float angle = 2.0f * glm::pi<float>() * float(i) / float(numSegments);
        const float x = radius * cos(angle);
        const float z = radius * sin(angle);

        v.position = glm::vec3(x, 0.01f, z);
        v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        vertices.emplace_back(v);
    }

    for (int i = 1; i <= numSegments; ++i)
    {
        indices.emplace_back(0);
        indices.emplace_back(i % numSegments + 1);
        indices.emplace_back(i);
    }
    auto mesh = make_shared<Mesh>(vertices, indices);
    mesh->SetBuffers();
    auto mr = AddComponent<MeshRenderer>();
    mr->AddMesh(mesh);
    mr->SetShader("ShadowShader.glsl");
    GetTransform()->SetLocalScale(0.05f);
    GetTransform()->SetLocalPosition({ 0,-0.1f,0 });

    GetParentGameObj().lock()->GetTransform()->MakeFinalMat();

    m_fPlayerOriginY = GetParentGameObj().lock()->GetTransform()->GetWorldPosition().y;
}

void PlayerShadow::Update()
{
   // const auto lightDir = glm::normalize(Mgr(Core)->GetUBOData().dirLight.lData.direction);

    const auto lightDir = Y_AXIS;
    const auto pParentTrans = GetParentGameObj().lock()->GetTransform();
    const auto pTrans = GetTransform();

    const float angle = glm::acos(glm::dot(lightDir, Y_AXIS));
    
    const glm::vec3 parentPos = Y_AXIS;
    const glm::vec3 b = parentPos + glm::vec3{0,0.1f,0};
    const float len = glm::length(b);

    const glm::vec3 a = lightDir * (1.f / glm::cos(angle)) * len;
   
    const glm::vec3 res = glm::normalize(a + b) * glm::tan(angle) * len;

    auto c = (res - parentPos);
    c.y = -0.1f;

    pTrans->SetLocalPosition(c);

    const float fDelta = 0.1f * (pParentTrans->GetWorldPosition().y - m_fPlayerOriginY);
    pTrans->SetLocalScale(0.05f + fDelta);
    m_fShadowAlpha = glm::clamp(1.f - fDelta * 20.f,0.f,1.f);
    GameObj::Update();
}

void PlayerShadow::Render()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const auto& shader = GetComp<MeshRenderer>()->GetShader();
    shader->Use();
    GLint progID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &progID);
    glUniform1f(glGetUniformLocation(progID, "uShadowAlpha"),m_fShadowAlpha);
    GameObj::Render();

    glDisable(GL_BLEND);
}
