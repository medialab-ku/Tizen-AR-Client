#ifndef __SCENE_ONE__
#define __SCENE_ONE__

#include <iostream>
#include "Scene.h"
#include "PhysicsActor.h"
#include "Shader.h"
#include "PrimitiveModels.h"
#include "Particle.h"
#include "Assets.h"
#include "obj-loader.h"
#include "Model.h"
#include <sstream>
#include <map>
#include <json/json.h>

using namespace std;

class FenceModel : public Model
{
public:
	FenceModel(Dali::Shader shader)
	 : Model(shader)
	{
		AddTexture(0, "default.jpg");
		Geometry geo = Geometry::New();
		SetGeometry(geo);
	}
};

class SceneOne : public Scene
{
public:
	SceneOne(Dali::Stage &stage, Dali::CameraActor &camera, Dali::Layer &uiLayer,
            btDiscreteDynamicsWorld *dynamicsWorld)
            : Scene(stage, camera, uiLayer, dynamicsWorld),
			  mLightDir(wVector3(-0.2, -1, 0.3))
	{

	}

	void OnStart() override
	{
        // a1 = CreateSphere( wVector3(-0.05, 0.2, 0));
        // a2 = CreateSphere( wVector3(0.05, 0.2, 0));
        // a1->SetVelocity(wVector3(0.1, 0, 0));
        // a2->SetVelocity(wVector3(-0.1, 0, 0));
        ParseRecipe();
        // //dlog_print(DLOG_DEBUG, "TIZENAR", "recipe parsed");
        PlaceObjects();
        // //dlog_print(DLOG_DEBUG, "TIZENAR", "object placed");
        CreateFence();
        // //dlog_print(DLOG_DEBUG, "TIZENAR", "fence placed");
        InitApple();

        // cout << "scene creation finished" << endl;
	}

	void OnUpdate(double deltaTime) override
	{
        // cout << "frame start, deltaTime : " << deltaTime << endl;
        // a1->Debug();
        // a2->Debug();
	}

	void OnKeyEvent(const Dali::KeyEvent &event) override
	{

	}

	void OnTouch(Dali::Actor actor, const Dali::TouchData &touch) override
	{
		if (touch.GetPointCount() > 0)
		{
			Dali::Vector2 touchPos = touch.GetScreenPosition(0);
			if (touch.GetState(0) == PointState::DOWN)
			{
				ScreenToWorldResult stw = mCamera->ScreenToWorld(touchPos);
				//auto apple = CreateCube( worldToPlanePos(stw.worldPos) );
                auto apple = CreateApple( worldToPlanePos(stw.worldPos) );
				appleList.push_back(apple);
				if(appleList.size() > 100)
				{
					auto a = *(appleList.begin());
					a->RemoveRigidBody();
					RemoveActor(a);
					appleList.erase(appleList.begin());
				}
                apple->SetVelocity(worldToPlaneVec(stw.direction) * 1.2f);
				//apple->ApplyTorque(wVector3(-0.5f, 0, 0));
				//apple->ApplyForce( worldToPlaneVec(stw.direction) * 70);
			}
		}
	}

private:
	void ParseRecipe()
	{
		std::string data;
		ReadRecipe("scene/scene.json", data);
		////dlog_print(DLOG_DEBUG, "TIZENAR", data.c_str());
		Json::Reader reader;
		if(!reader.parse(data, root));
			//dlog_print(DLOG_DEBUG, "TIZENAR", "failed to parse json");
	}

	void PlaceObjects()
	{
		float globalScale = root["globalScale"].asFloat();
		Json::Value objs = root["objs"];
		for(int i = 0; i < objs.size(); i++)
		{
			string modelName = objs[i]["model"].asString();
			string material = objs[i]["material"].asString();

			auto it = objModelMap.find(modelName);
			if(it == objModelMap.end()) // if model is not loaded
			{
				Dali::Shader shader;
				if(material == std::string("color"))
				{
					Assets::GetShader("vertexDiffuse.glsl", "fragmentDiffuse.glsl", shader);
					shader.RegisterProperty("uLightDir", mLightDir.ToDali());
					shader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
					Dali::Vector3 floorColor =  Dali::Vector3(76.0f, 121.0f, 42.0f) / 255.0f;
					shader.RegisterProperty("uObjectColor", floorColor);
				}
				else
				{
					Assets::GetShader("vertexPhong.glsl", "fragmentPhong.glsl", shader);
					shader.RegisterProperty("uLightPos", mLightDir.ToDali());
					shader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
					shader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));
				}

				std::string textureName = objs[i]["texture"].asString() + ".png";
				PrimitiveObj model(textureName, shader, modelName + ".obj");
				objModelMap.insert(make_pair(modelName, model));
				it = objModelMap.find(modelName);
			}

			PrimitiveObj model = it->second;
			FrameActor* actor;
            bool isPhysicsActor = false;
			if(modelName == "block_wood1_LOD0" || modelName == "block_wood2_LOD0" )
			{
                isPhysicsActor = true;

				model.SetMass(1.5f);
                model.SetFriction(0.5f);
                model.SetRestitution(0.5f);

                Json::Value scaleVal = objs[i]["scale"];
                wVector3 scale(scaleVal["x"].asFloat(), scaleVal["y"].asFloat(), scaleVal["z"].asFloat());

				actor = new PhysicsActor(mStage, model, mDynamicsWorld, scale * globalScale);
			}
			else actor = new GraphicsActor(mStage, model);

			Json::Value posVal = objs[i]["position"];
			wVector3 pos(posVal["x"].asFloat(), posVal["y"].asFloat(), posVal["z"].asFloat());
			//dlog_print(DLOG_DEBUG, "TIZENAR", "object pos : %f, %f, %f", pos.x, pos.y, pos.z);
			actor->SetPosition(pos * globalScale);

			Json::Value rotVal = objs[i]["rotation"];
			wQuaternion rot(rotVal["x"].asFloat(), rotVal["y"].asFloat(), rotVal["z"].asFloat(), rotVal["w"].asFloat());
			actor->SetRotation(rot);

            if(!isPhysicsActor)
            {
                Json::Value scaleVal = objs[i]["scale"];
                wVector3 scale(scaleVal["x"].asFloat(), scaleVal["y"].asFloat(), scaleVal["z"].asFloat());
                actor->SetSize(scale * globalScale);
            }

			AddActor(actor);
		}
	}

	PhysicsActor* CreateCube(wVector3 pos)
	{
		Dali::Shader shader;
		Assets::GetShader("vertexPhong.glsl", "fragmentPhong.glsl", shader);
		shader.RegisterProperty("uLightPos", mLightDir.ToDali());
		shader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
		shader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));

		PrimitiveObj model("cube_tex.png", shader, "block_wood2_LOD0.obj");

		model.SetMass(1.4f);
        model.SetFriction(0.5f);
        model.SetRollingFriction(0.005f);
		auto actor = new PhysicsActor(mStage, model, mDynamicsWorld, wVector3(0.02, 0.02, 0.02));
		actor->SetPosition(pos);
		//actor->SetSize(0.02, 0.02, 0.02);

		AddActor(actor);
		return actor;
	}

    PhysicsActor* CreateSphere(wVector3 pos)
	{
		Dali::Shader shader;
		Assets::GetShader("vertexPhong.glsl", "fragmentPhong.glsl", shader);
		shader.RegisterProperty("uLightPos", mLightDir.ToDali());
		shader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
		shader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));

        PrimitiveSphere model("001.jpg", shader);
		//PrimitiveObj model("cube_tex.png", shader, "block_wood2_LOD0.obj");

		model.SetMass(0.1f);
        model.SetFriction(0.1f);
        model.SetRestitution(1.f);
		auto actor = new PhysicsActor(mStage, model, mDynamicsWorld, wVector3(0.06, 0.06, 0.06));
		actor->SetPosition(pos);
		// actor->SetSize(0.06, 0.06, 0.06);

		AddActor(actor);
		return actor;
	}

    void InitApple()
    {
        Dali::Shader mAppleShader;
		mAppleShader = LoadShaders("vertexPhong.glsl", "fragmentPhong.glsl");
		mAppleShader.RegisterProperty("uLightPos", mLightDir.ToDali());
		mAppleShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
		mAppleShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));
		ObjLoader obj;
		if (!Assets::GetObj("Apple.obj", obj))
		{
			return;
		}

		appleModel = new AppleModel("Default_Palette.png", mAppleShader);
    }

	PhysicsActor* CreateApple(wVector3 pos)
	{
		auto apple = new PhysicsActor(mStage, *appleModel, mDynamicsWorld,  wVector3(0.03, 0.03, 0.03));
		apple->SetPosition(pos);
        apple->SetRotation(0, 0, 1, 0);
		//apple->SetSize(0.03, 0.03, 0.03);
		AddActor(apple);

		return apple;
	}

	void CreateFence()
	{
		float globalScale = root["globalScale"].asFloat();

		Dali::Shader blockShader;
		Assets::GetShader("vertexColor.glsl", "fragmentColor.glsl", blockShader);
		FenceModel cube(blockShader);
		// Assets::GetShader("vertexPhong.glsl", "fragmentPhong.glsl", blockShader);
		// PrimitiveTexturedCube cube("wood.png", blockShader);

		PhysicsActor* fence1 = new PhysicsActor(mStage, cube, mDynamicsWorld, wVector3(8.915535, 1, 0.2026377) * globalScale);
		fence1->SetPosition(wVector3(0, 1.5, -6.7) * globalScale);
		AddActor(fence1);

		PhysicsActor* fence2 = new PhysicsActor(mStage, cube, mDynamicsWorld, wVector3(8.915535, 1, 0.2026377) * globalScale);
		fence2->SetPosition(wVector3(0, 1.5, 6.65) * globalScale);
		AddActor(fence2);

		PhysicsActor* fence3 = new PhysicsActor(mStage, cube, mDynamicsWorld, wVector3(0.2026377, 1, 7.79048) * globalScale);
		fence3->SetPosition(wVector3(7.85, 1.5, 0) * globalScale);
		AddActor(fence3);

		PhysicsActor* fence4 = new PhysicsActor(mStage, cube, mDynamicsWorld, wVector3(0.2026377, 1, 7.79048) * globalScale);
		fence4->SetPosition(wVector3(-7.9, 1.5, 0) * globalScale);
		AddActor(fence4);

		cube.SetFriction(0.5f);
		cube.SetRestitution(0.2f);

		PhysicsActor* floor = new PhysicsActor(mStage, cube, mDynamicsWorld, wVector3(9, 5, 9) * globalScale);
		floor->SetPosition(wVector3(0, -5, 0) * globalScale);
		AddActor(floor);
	}

private:
	wVector3 mLightDir; // Directional Light
	Json::Value root;
	std::map<std::string, PrimitiveObj> objModelMap;

	std::vector<PhysicsActor*> appleList;

    AppleModel* appleModel;


    // Debug
    PhysicsActor* a1;
    PhysicsActor* a2;
};

#endif
