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
#include <dlog.h>
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
		 ParseRecipe();
		 dlog_print(DLOG_DEBUG, "TIZENAR", "recipe parsed");
		 PlaceObjects();
		 dlog_print(DLOG_DEBUG, "TIZENAR", "object placed");
		 CreateFence();
		 dlog_print(DLOG_DEBUG, "TIZENAR", "fence placed");
	}

	void OnUpdate(double deltaTime) override
	{

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
				auto apple = CreateApple( worldToPlanePos(stw.worldPos) );
				appleList.push_back(apple);
				if(appleList.size() > 8)
				{
					auto a = *(appleList.begin());
					RemoveActor(a);
					appleList.erase(appleList.begin());
				}
				//apple->ApplyTorque(wVector3(-1.3f, 0, 0));
				apple->ApplyForce( worldToPlaneVec(stw.direction) * 10.0, wVector3(0, 0, 0));
			}
		}
	}

private:
	void ParseRecipe()
	{
		std::string data;
		ReadRecipe("scene/scene.json", data);
		//dlog_print(DLOG_DEBUG, "TIZENAR", data.c_str());
		Json::Reader reader;
		if(!reader.parse(data, root))
			dlog_print(DLOG_DEBUG, "TIZENAR", "failed to parse json");
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
				model.SetFriction(0.3);
				model.SetRollingFriction(0.01);
				model.SetSpinningFriction(0.01);
				objModelMap.insert(make_pair(modelName, model));
				it = objModelMap.find(modelName);
			}

			PrimitiveObj model = it->second;
			FrameActor* actor;
			if(modelName == "block_wood1_LOD0" || modelName == "block_wood2_LOD0" )
			{
				if(modelName != "terrain-plane-plain") model.SetMass(0.2f);
				actor = new PhysicsActor(mStage, model, mDynamicsWorld);
			}
			else actor = new GraphicsActor(mStage, model);

			Json::Value posVal = objs[i]["position"];
			wVector3 pos(posVal["x"].asFloat(), posVal["y"].asFloat(), posVal["z"].asFloat());
			dlog_print(DLOG_DEBUG, "TIZENAR", "object pos : %f, %f, %f", pos.x, pos.y, pos.z);
			actor->SetPosition(pos * globalScale);

			Json::Value rotVal = objs[i]["rotation"];
			wQuaternion rot(rotVal["x"].asFloat(), rotVal["y"].asFloat(), rotVal["z"].asFloat(), rotVal["w"].asFloat());
			actor->SetRotation(rot);

			Json::Value scaleVal = objs[i]["scale"];
			wVector3 scale(scaleVal["x"].asFloat(), scaleVal["y"].asFloat(), scaleVal["z"].asFloat());
			actor->SetSize(scale * globalScale);

			AddActor(actor);
		}
	}

	PhysicsActor* CreateApple(wVector3 pos)
	{
        Dali::Shader mAppleShader;
		mAppleShader = LoadShaders("vertexPhong.glsl", "fragmentPhong.glsl");
		mAppleShader.RegisterProperty("uLightPos", mLightDir.ToDali());
		mAppleShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
		mAppleShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));
		ObjLoader obj;
		if (!Assets::GetObj("Apple.obj", obj))
		{
			return nullptr;
		}

		AppleModel appleModel("Default_Palette.png", mAppleShader);

		auto apple = new PhysicsActor(mStage, appleModel, mDynamicsWorld);
		apple->SetPosition(pos);
		apple->SetSize(0.03, 0.03, 0.03);
		AddActor(apple);

		return apple;
	}

	void CreateFence()
	{
		float globalScale = root["globalScale"].asFloat();

		Dali::Shader blockShader;
		Assets::GetShader("vertexColor.glsl", "fragmentColor.glsl", blockShader);
		FenceModel cube(blockShader);

		PhysicsActor* fence1 = new PhysicsActor(mStage, cube, mDynamicsWorld);
		fence1->SetPosition(wVector3(0.2265606, 1.185877, -7.797238)*globalScale);
		fence1->SetSize(wVector3(15.55178, 2.212319, 0.4080844)*globalScale);
		AddActor(fence1);

		PhysicsActor* fence2 = new PhysicsActor(mStage, cube, mDynamicsWorld);
		fence2->SetPosition(wVector3(0.2265606, 1.185877, 7.766111)*globalScale);
		fence2->SetSize(wVector3(15.55178, 2.212319, 0.4664226)*globalScale);
		AddActor(fence2);

		PhysicsActor* fence3 = new PhysicsActor(mStage, cube, mDynamicsWorld);
		fence3->SetPosition(wVector3(-7.541017, 1.185877, 0.1334667)*globalScale);
		fence3->SetSize(wVector3(0.4136515, 2.212319, 15.73171)*globalScale);
		AddActor(fence3);

		PhysicsActor* fence4 = new PhysicsActor(mStage, cube, mDynamicsWorld);
		fence4->SetPosition(wVector3(8.132362, 1.185877, 0.1334667)*globalScale);
		fence4->SetSize(wVector3(0.5355411, 2.212319, 15.73171)*globalScale);
		AddActor(fence4);

		PhysicsActor* floor = new PhysicsActor(mStage, cube, mDynamicsWorld);
		floor->SetPosition(wVector3(0, -5, 0) * globalScale);
		floor->SetSize(wVector3(15.8, 5, 15.8) * globalScale);
		AddActor(floor);
	}

private:
	wVector3 mLightDir; // Directional Light
	Json::Value root;
	std::map<std::string, PrimitiveObj> objModelMap;

	std::vector<PhysicsActor*> appleList;
};

#endif
