#include "pch.h"
#include <MyOpenGL.hpp>

#include "MCTilemap.h"
#include "MCTerrainGenerator.h"
#include "MCTilemapMeshGenerator.h"

shared_ptr<GameObj> pObserver;
shared_ptr<Camera>  observerCam;

void Update();

int main()
{
    Mgr(Core)->Init();
    Mgr(Core)->SetClearColor(RGBA_WHITE);

    Camera::GetCurCam()->GetTransform()->SetLocalPosition({ 0.0f, 0.0f, -1.0f });
    //Camera::GetCurCam()->GetTransform()->SetLocalRotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));

    //pObserver = Mgr(AssimpMgr)->Load("SimpleShader.glsl", "yup.obj");

   // observerCam = pObserver->GetComp<Camera>();

   // pObserver->GetTransform()->SetLocalPosition(glm::vec3{ 0.0f, 0.0f, 0.0f });
   // pObserver->GetTransform()->SetLocalRotation(0.0f);
   // pObserver->GetTransform()->SetLocalScale(0.25f);

    MCTilemap* tilemap = new MCTilemap();
    MCTerrainGenerator* terrainGenerator = new MCTerrainGenerator();
    MCTilemapMeshGenerator* meshGenerator = new MCTilemapMeshGenerator();

    terrainGenerator->Generate(tilemap);
    shared_ptr<Mesh> mesh = meshGenerator->CreateMesh(tilemap);

    shared_ptr<GameObj> terrainObj = GameObj::make_obj<GameObj>();
    auto renderer = terrainObj->AddComponent<MeshRenderer>();
    renderer->AddMesh(mesh);
    renderer->SetShader("SimpleShader.glsl");
    terrainObj->GetTransform()->SetLocalScale(0.1f);
    terrainObj->GetTransform()->SetLocalPosition({ -3.5f,-1.2f,-1.2f });
    const auto curScene = Mgr(SceneMgr)->GetCurScene();
  //  curScene->AddObject(pObserver, GROUP_TYPE::DEFAULT);
    curScene->AddObject(terrainObj, GROUP_TYPE::DEFAULT);
    curScene->AddUpdateFp(SCENE_ADDED_UPDATE::PRERENDER, &Update);

    {
        auto pLight = GameObj::make_obj();
        auto l = pLight->AddComponent<Light>();
        l->SetCurLightType(LIGHT_TYPE::DIRECTIONAL);
        l->SetLightPos({ -100,100,0 });
       // pLight->GetTransform()->SetLocalRotation(90.f, X_AXIS);
        pLight->GetTransform()->SetLookAt({ -3.5f,-1.2f,-1.2f });
        l->SetSpecular({ .5f,.5f,.5f });
        l->SetDiffuse({ .5f,.5f,.5f });
        curScene->AddObject(pLight, GROUP_TYPE::DEFAULT);
    }
    curScene->SetSkyBox(SKYBOX_TYPE::SPHERE, "basic_skybox_3d_flip.fbx", "magical_forest_fantasy_6k.jpg");
    Mgr(Core)->GameLoop();
}

void Update()
{
   // static bool b = false;
   // auto& uboBlock = Mgr(Core)->GetUBOData();
   // uboBlock.viewPos = pObserver->GetTransform()->GetWorldPosition();
   // Mgr(Core)->BindUBOData();
}