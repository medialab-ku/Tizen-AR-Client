#ifndef __SCENE_H__
#define __SCENE_H__

#include <dali-toolkit/dali-toolkit.h>
#include <dali/dali.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <list>
#include "FrameActor.h"
#include "GraphicsActor.h"
#include "CameraFrameActor.h"
#include "PrimitiveModels.h"
#include "Assets.h"

class PlaneActor : public GraphicsActor
{
	public:
		PlaneActor(Dali::Stage &stage, Model &model);
	    wVector3 GetLocalPosition() override;
	    wQuaternion GetLocalRotation() override;
	    wVector3 GetPlanePosition() override;
	    wQuaternion GetPlaneRotation() override;
		void OnSpaceUpdated(FrameActor *plane, wVector3 basisX, wVector3 basisY, wVector3 basisZ, wVector3 origin) override;
};

class Scene
{
    public:
        Scene(Dali::Stage &stage, Dali::CameraActor &camera, Dali::Layer &uiLayer,
                    btDiscreteDynamicsWorld *dynamicsWorld);
        virtual void Start();
        virtual void Update(double deltaTime, wVector3 planeNormal, wVector3 planeOrigin, wVector3 cameraPos, wQuaternion cameraRot);
        virtual void KeyEvent(const Dali::KeyEvent &event);
        virtual void Touch(Dali::Actor actor, const Dali::TouchData &touch);

    protected:
        virtual void OnStart();
        virtual void OnUpdate(double deltaTime);
        virtual void OnKeyEvent(const Dali::KeyEvent &event);
        virtual void OnTouch(Dali::Actor actor, const Dali::TouchData &touch);

    protected:
        void AddActor(FrameActor *actor);
        void RemoveActor(FrameActor *actor);
        void AddUI(Dali::Actor &ui);    // todo : Wrap with FrameActor
        wVector3 worldToPlanePos(wVector3 worldPos);
        wVector3 planeToWorldPos(wVector3 planePos);
        wVector3 worldToPlaneVec(wVector3 worldVec);

    private:
        void _UpdatePlane(wVector3 planeNormal, wVector3 planeOrigin);
		void _UpdateCamera(wVector3 cameraPos, wQuaternion cameraRot);
		void _UpdateWorld(double deltaTime, wVector3 planeNormal);

    protected:
        // Essentials
        // FrameActor is pointer since it can be created and removed at any time
        std::list<FrameActor*> mListActor;
        btDiscreteDynamicsWorld *mDynamicsWorld;
        Dali::Stage mStage;
        Dali::Layer mUILayer;
        FrameActor *mPlane;
        CameraFrameActor *mCamera;
        wVector3 _basisX, _basisY, _basisZ;
        wVector3 _origin;
};

#endif
