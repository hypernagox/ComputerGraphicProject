#include "pch.h"
#include <MyOpenGL.hpp>

#include "MCTilemap.h"
#include "MCTerrainGenerator.h"
#include "MCTilemapMeshGenerator.h"

#include "Player.h"
#include "PlayerCam.h"

shared_ptr<GameObj> pObserver;
shared_ptr<Camera>  observerCam;

void Update();

int main()
{
    Mgr(Core)->Init();
    Mgr(Core)->SetClearColor(RGBA_WHITE);

    glfwSetInputMode(Mgr(Core)->GetWinInfo(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    shared_ptr<Material> material = make_shared<Material>();
    material->AddTexture2D("stone.png");

    std::thread([]()noexcept {
        MCTilemap* tilemap = new MCTilemap();
        MCTerrainGenerator* terrainGenerator = new MCTerrainGenerator();
        MCTilemapMeshGenerator* meshGenerator = new MCTilemapMeshGenerator();

        terrainGenerator->Generate(tilemap);
        meshGenerator->CreateMesh(tilemap); }).detach();

    const auto curScene = Mgr(SceneMgr)->GetCurScene();
    curScene->AddUpdateFp(SCENE_ADDED_UPDATE::PRERENDER, &Update);

    {
        auto pLight = Mgr(AssimpMgr)->Load("EnvironmentShader.glsl", "MyCube.fbx");
        auto l = pLight->AddComponent<Light>();
        l->SetCurLightType(LIGHT_TYPE::DIRECTIONAL);
        l->SetLightPos({ -5,5,0 });
        auto mate = make_shared<Material>();
        mate->AddTexture2D("magical_forest_fantasy_6k.jpg");
        mate->SetMaterialDiffuse({ .3f,.3f,.3f });
        mate->SetMaterialSpecular({ .3f,.3f,.3f });
        pLight->GetComp<MeshRenderer>()->AddMaterial(mate);
        pLight->GetTransform()->SetLocalScale(0.1f);
        pLight->GetTransform()->SetLocalRotation(90.f, X_AXIS);
        pLight->GetTransform()->SetLookAt({ -3.5f,-1.2f,-1.2f });
        l->SetAmbient(glm::vec3{ 0.75f, 0.75f, 0.75f });
        l->SetDiffuse(glm::vec3{ 0.8f, 0.8f, 0.8f });
        curScene->AddObject(pLight, GROUP_TYPE::MONSTER);
        pLight->SetObjName("light");
        auto pCol = pLight->AddComponent<Collider>();
        pCol->SetColBoxScale({ 5,5,5 });
        pCol->GetCollisionHandler()->SetCollisionHandlerFunc([l,pLight](auto& a, auto& b) {
            static auto s = l->GetLightSpecular();
            static auto d = l->GetLightDiffuse();
            static bool flag = false;
            flag = !flag;
            if (flag)
            {
                l->SetSpecular({});
                l->SetDiffuse({});
            }
            else
            {
                l->SetSpecular(s);
                l->SetDiffuse(d);
            }
            },COLLISION_TYPE::COL_ENTER);
        
    }
    {
        auto pLight = Mgr(AssimpMgr)->Load("EnvironmentShader.glsl", "MySphereRed.fbx");
        auto l = pLight->AddComponent<Light>();
        l->SetCurLightType(LIGHT_TYPE::POINT);
        l->SetDiffuse({ 5.1f,.1f,.1f });
        l->SetSpecular({ 5.1f,.1f,.1f });
        auto m = pLight->GetComp<MeshRenderer>();
        auto mate = make_shared<Material>();
        mate->AddTexture2D("magical_forest_fantasy_6k.jpg");
        mate->SetMaterialDiffuse({ .3f,.3f,.3f });
        mate->SetMaterialSpecular({ .3f,.3f,.3f });
        m->AddMaterial(mate);
        pLight->GetTransform()->SetLocalScale(0.1f);
        pLight->GetTransform()->SetLocalRotation(90.f, X_AXIS);
        pLight->GetTransform()->SetLocalPosition({ 0,5,0 });
        curScene->AddObject(pLight, GROUP_TYPE::MONSTER);
    }

    Mgr(CollisionMgr)->RegisterGroup(GROUP_TYPE::MONSTER, GROUP_TYPE::PROJ_PLAYER);
    {
        auto player = make_shared<Player>();
        player->SetObjName("player");
        curScene->AddObject(player, GROUP_TYPE::PLAYER);
        /*player->AddChild(make_shared<PlayerCam>());*/
    }

    curScene->SetSkyBox(SKYBOX_TYPE::SPHERE, "basic_skybox_3d_flip.fbx", "skybox.png");
    Mgr(Core)->GameLoop();
}

void Update()
{

}