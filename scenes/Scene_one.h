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

using namespace std;

class SceneOne : public Scene
{
public:
	SceneOne(Dali::Stage &stage, Dali::CameraActor &camera, Dali::Layer &uiLayer,
            btDiscreteDynamicsWorld *dynamicsWorld)
            : Scene(stage, camera, uiLayer, dynamicsWorld)
	{


	}

	void OnStart() override
	{

	}

	void OnUpdate() override
	{

	}

	void OnKeyEvent(const Dali::KeyEvent &event) override
	{

	}

	void OnTouch(Dali::Actor actor, const Dali::TouchData &touch) override
	{

	}

private:

};

#endif
