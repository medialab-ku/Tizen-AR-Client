#include "Scene.h"
#include "FileSystem.h"
#include <iostream>
#include <dlog.h>
#include <sstream>

PlaneActor::PlaneActor(Dali::Stage &stage, Model &model)
	: FrameActor(stage)
{
	SetName("Plane");
}

wVector3
PlaneActor::GetLocalPosition() { return wVector3(0, 0, 0); }
wQuaternion
PlaneActor::GetLocalRotation() { return wQuaternion(0, 0, 0, 1); }
wVector3
PlaneActor::GetPlanePosition() { return wVector3(0, 0, 0); }
wQuaternion
PlaneActor::GetPlaneRotation() { return wQuaternion(0, 0, 0, 1); }

void
PlaneActor::OnSpaceUpdated(FrameActor *plane, wVector3 basisX, wVector3 basisY, wVector3 basisZ, wVector3 origin)
{
	FrameActor::OnSpaceUpdated(plane, basisX, basisY, basisZ, origin);
	auto pos = origin;
	auto rot = wQuaternion(Dali::Quaternion(basisX.ToDali(), basisY.ToDali(), basisZ.ToDali()));
	SetPosition(pos);
	SetRotation(rot);
}

Scene::Scene(Dali::Stage &stage, Dali::CameraActor &camera, Dali::Layer &uiLayer,
    btDiscreteDynamicsWorld *dynamicsWorld)
    : mListActor(),
      mStage(stage),
      mUILayer(uiLayer),
      mDynamicsWorld(dynamicsWorld),
	  _origin(), _basisX(), _basisY(), _basisZ()
{
    mCamera = new CameraFrameActor(stage, camera);

    Dali::Shader planeShader;
    //dlog_print(DLOG_DEBUG, "TIZENAR", "getshader");
    if (not Assets::GetShader("vertexColor.glsl", "fragmentColor.glsl", planeShader))
    {
    	dlog_print(DLOG_DEBUG, "TIZENAR", "error");
    }
    //dlog_print(DLOG_DEBUG, "TIZENAR", "asdf");
	planeShader.RegisterProperty("uAlpha", 0.3f);
	PrimitivePlane model("wood.png", planeShader);
	mPlane = new PlaneActor(stage, model);

    AddActor(mCamera);
    AddActor(mPlane);
}

void
Scene::Start()
{
	OnStart();
	for(auto itr = mListActor.begin(); itr != mListActor.end(); ++itr)
	{
		(*itr)->OnStart();
	}
}

void
Scene::Update(double deltaTime, wVector3 planeNormal, wVector3 planeOrigin, wVector3 cameraPos, wQuaternion cameraRot)
{
	// dlog_print(DLOG_DEBUG, "TIZENAR", "plane");
	_UpdatePlane(planeNormal, planeOrigin);
	// dlog_print(DLOG_DEBUG, "TIZENAR", "camera");
	_UpdateCamera(cameraPos, cameraRot);
	// dlog_print(DLOG_DEBUG, "TIZENAR", "world");
	_UpdateWorld(deltaTime, planeNormal);
	// dlog_print(DLOG_DEBUG, "TIZENAR", "onupdate");
    OnUpdate(deltaTime);
    for(auto itr = mListActor.begin(); itr != mListActor.end(); ++itr)
    {
    	// dlog_print(DLOG_DEBUG, "TIZENAR", (*itr)->GetName().c_str());
        (*itr)->OnUpdate(deltaTime);
    }
}

void
Scene::KeyEvent(const Dali::KeyEvent &event)
{
    OnKeyEvent(event);
}

void
Scene::Touch(Dali::Actor actor, const Dali::TouchData &touch)
{
    OnTouch(actor, touch);
}

void
Scene::_UpdatePlane(wVector3 planeNormal, wVector3 planeOrigin)
{
    // origin
    _origin = planeOrigin;

    // basis
    auto n = planeNormal.ToDali();
    auto z = Dali::Vector3::XAXIS.Cross(n);
    auto x = n.Cross(z);
    _basisX = x;
    _basisY = n;
    _basisZ = z;

    for(auto itr = mListActor.begin(); itr != mListActor.end(); ++itr)
    {
        (*itr)->OnSpaceUpdated(mPlane, _basisX, _basisY, _basisZ, _origin);
    }
}

void
Scene::_UpdateCamera(wVector3 cameraPos, wQuaternion cameraRot)
{
    mCamera->SetPosition(cameraPos);
    mCamera->SetRotation(cameraRot);
}

void
Scene::_UpdateWorld(double deltaTime, wVector3 planeNormal)
{
//	float gravity = planeNormal.y > 0 ? -9.81f : 9.81f;
//	mDynamicsWorld->setGravity(planeNormal.ToBullet() * gravity);
	mDynamicsWorld->stepSimulation(deltaTime, 3);
}

void 
Scene::AddActor(FrameActor *actor)
{
    bool found = (std::find(mListActor.begin(), mListActor.end(), actor) != mListActor.end());
    if (!found)
    {
    	mListActor.push_back(actor);
        actor->OnSpaceUpdated(mPlane, _basisX, _basisY, _basisZ, _origin);

        if (actor != mPlane && actor != mCamera)
        {
        	mPlane->AddChild(actor);
        }
    }
}

void 
Scene::RemoveActor(FrameActor *actor)
{
    bool found = (std::find(mListActor.begin(), mListActor.end(), actor) != mListActor.end());
    if (found)
    {
    	mListActor.remove(actor);
        actor->OnSpaceUpdated(mPlane, wVector3::right, wVector3::up, wVector3::forward, wVector3::zero);

        if (actor != mPlane && actor != mCamera)
        {
            mPlane->RemoveChild(actor);
        }

        actor->RemoveActor();
        delete actor;
    }
}

void
Scene::AddUI(Dali::Actor &ui)
{
    mUILayer.Add(ui);
}

void
Scene::ReadRecipe(std::string filename, std::string& outData)
{
	std::string filepath = FileSystem::GetResourcePath(filename);

	std::ifstream file(filepath);
	std::stringstream buf;
	buf << file.rdbuf();

	outData = std::string(buf.str());
}

wVector3
Scene::worldToPlanePos(wVector3 worldPos)
{
	auto planePos = _origin;
	auto planeRot = wQuaternion(Dali::Quaternion(_basisX.ToDali(), _basisY.ToDali(), _basisZ.ToDali()));

	auto translated = worldPos - planePos;
	auto rotated = planeRot.Inverse() * translated;
	return rotated;
}

wVector3
Scene::planeToWorldPos(wVector3 planePos)
{
	auto pP = _origin;
	auto planeRot = wQuaternion(Dali::Quaternion(_basisX.ToDali(), _basisY.ToDali(), _basisZ.ToDali()));

	auto rotated = planeRot * planePos;
	auto translated = rotated + pP;
	return translated;
}
wVector3
Scene::worldToPlaneVec(wVector3 worldVec)
{
	auto planeRot = wQuaternion(Dali::Quaternion(_basisX.ToDali(), _basisY.ToDali(), _basisZ.ToDali()));

	return planeRot.Inverse() * worldVec;
}

void
Scene::OnStart()
{

}

void
Scene::OnUpdate(double deltaTime)
{

}

void
Scene::OnKeyEvent(const Dali::KeyEvent &event)
{

}

void
Scene::OnTouch(Dali::Actor actor, const Dali::TouchData &touch)
{

}
