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

    //Camera::GetCurCam()->GetTransform()->SetLocalPosition({ 0.0f, 0.0f, -1.0f });
    //Camera::GetCurCam()->GetTransform()->SetLocalRotation(glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)));

    //pObserver = Mgr(AssimpMgr)->Load("SimpleShader.glsl", "yup.obj");

   // observerCam = pObserver->GetComp<Camera>();

   // pObserver->GetTransform()->SetLocalPosition(glm::vec3{ 0.0f, 0.0f, 0.0f });
   // pObserver->GetTransform()->SetLocalRotation(0.0f);
   // pObserver->GetTransform()->SetLocalScale(0.25f);


    //shared_ptr<Material> material = make_shared<Material>();
    //material->AddTexture2D("stone.png");

    //MCTilemap* tilemap = new MCTilemap();
    //MCTerrainGenerator* terrainGenerator = new MCTerrainGenerator();
    //MCTilemapMeshGenerator* meshGenerator = new MCTilemapMeshGenerator();
    //
    //terrainGenerator->Generate(tilemap);
    //shared_ptr<Mesh> mesh = meshGenerator->CreateMesh(tilemap);
    
    std::thread([]()noexcept {
        MCTilemap* tilemap = new MCTilemap();
        MCTerrainGenerator* terrainGenerator = new MCTerrainGenerator();
        MCTilemapMeshGenerator* meshGenerator = new MCTilemapMeshGenerator();

        terrainGenerator->Generate(tilemap);
        meshGenerator->CreateMesh(tilemap); }).detach();
    
    const auto curScene = Mgr(SceneMgr)->GetCurScene();
   
    curScene->AddUpdateFp(SCENE_ADDED_UPDATE::PRERENDER, &Update);

    {
        auto pLight = Mgr(AssimpMgr)->Load("SimpleShaderHasColorLight.glsl","MyCube.fbx");
        auto l = pLight->AddComponent<Light>();
        l->SetCurLightType(LIGHT_TYPE::DIRECTIONAL);
        l->SetLightPos({ -5,5,0 });
        pLight->GetTransform()->SetLocalScale(0.1f);
        pLight->GetTransform()->SetLocalRotation(90.f, X_AXIS);
        pLight->GetTransform()->SetLookAt({ -3.5f,-1.2f,-1.2f });
        l->SetSpecular(glm::vec3{ 1.0f,1.0f,1.0f });
        l->SetDiffuse(glm::vec3{ 1.0f,1.0f,1.0f });
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
    Mgr(CollisionMgr)->RegisterGroup(GROUP_TYPE::MONSTER, GROUP_TYPE::PROJ_PLAYER);
    {
        auto player = make_shared <Player>();
        player->SetObjName("player");
        curScene->AddObject(player, GROUP_TYPE::PLAYER);
        player->AddChild(make_shared<PlayerCam>());
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