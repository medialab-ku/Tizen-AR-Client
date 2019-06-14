#include "Scene.h"
#include <iostream>
#include <dlog.h>

PlaneActor::PlaneActor(Dali::Stage &stage, Model &model)
	: GraphicsActor(stage, model)
{
	SetName("Plane");
}

void
PlaneActor::OnSpaceUpdated(FrameActor *plane, wVector3 basisX, wVector3 basisY, wVector3 basisZ, wVector3 origin)
{
	GraphicsActor::OnSpaceUpdated(plane, basisX, basisY, basisZ, origin);
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
    dlog_print(DLOG_DEBUG, "TIZENAR", "getshader");
    if (not Assets::GetShader("vertexColor.glsl", "fragmentColor.glsl", planeShader))
    {
    	dlog_print(DLOG_DEBUG, "TIZENAR", "error");
    }
    dlog_print(DLOG_DEBUG, "TIZENAR", "asdf");
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
	dlog_print(DLOG_DEBUG, "TIZENAR", "plane");
	_UpdatePlane(planeNormal, planeOrigin);
	dlog_print(DLOG_DEBUG, "TIZENAR", "camera");
	_UpdateCamera(cameraPos, cameraRot);
	dlog_print(DLOG_DEBUG, "TIZENAR", "world");
	_UpdateWorld(deltaTime, planeNormal);
	dlog_print(DLOG_DEBUG, "TIZENAR", "onupdate");
    OnUpdate(deltaTime);
    for(auto itr = mListActor.begin(); itr != mListActor.end(); ++itr)
    {
    	dlog_print(DLOG_DEBUG, "TIZENAR", (*itr)->GetName().c_str());
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
	float gravity = planeNormal.y > 0 ? -9.81f : 9.81f;
	mDynamicsWorld->setGravity(planeNormal.ToBullet() * gravity);
	   mDynamicsWorld->stepSimulation(deltaTime);
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

        delete actor;
    }
}

void
Scene::AddUI(Dali::Actor &ui)
{
    mUILayer.Add(ui);
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
