#include "pch.h"
#include <MyOpenGL.hpp>

shared_ptr<GameObj> pObserver;
shared_ptr<Camera>  observerCam;

void Update();

int main()
{
    Mgr(Core)->Init();
    Mgr(Core)->SetClearColor(RGBA_BLACK);

    const auto curScene = Mgr(SceneMgr)->GetCurScene();

    Camera::GetCurCam()->GetTransform()->SetLocalPosition({ 0.0f, 0.0f, -1.0f });

    pObserver = Mgr(AssimpMgr)->Load("ObserverShader.glsl", "yup.obj");
    observerCam = pObserver->GetComp<Camera>();

    pObserver->GetTransform()->SetLocalPosition(glm::vec3{ 0.5f,0,0 });
    pObserver->GetTransform()->SetLocalRotation(90.f);
    pObserver->GetTransform()->SetLocalScale(0.007f);

    curScene->AddObject(pObserver, GROUP_TYPE::DEFAULT);
    // curScene->AddUpdateFp(SCENE_ADDED_UPDATE::PRERENDER, Update);

    Mgr(Core)->GameLoop();
}

void Update()
{
    static bool b = false;
    Mgr(ResMgr)->GetRes<Shader>("ObserverShader.glsl")->SetUniformMat4(observerCam->GetCamMatView(), "uObserverView");
    Mgr(ResMgr)->GetRes<Shader>("CubeShader.glsl")->SetUniformMat4(observerCam->GetCamMatView(), "uObserverView");
    auto& uboBlock = Mgr(Core)->GetUBOData();
    uboBlock.viewPos = pObserver->GetTransform()->GetWorldPosition();
    Mgr(Core)->BindUBOData();
}