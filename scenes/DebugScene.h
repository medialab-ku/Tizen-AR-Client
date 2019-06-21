#ifndef __DEBUG_SCENE__
#define __DEBUG_SCENE__

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

class AppleModel : public Model
{
    public:
        AppleModel(std::string textureName, Dali::Shader &shader)
            : Model(shader)
        {
            AddTexture(0, textureName);
            ObjLoader obj;
            if (!Assets::GetObj("Apple.obj", obj))
            {
                return;
            }

            SetMass(0.3f);
            SetRollingFriction(0.05);
            
            Geometry geometry = obj.CreateGeometry(7, true);
		    SetGeometry(geometry);
        }

        btCollisionShape* CreateCollisionShape() override
        {
        	ObjLoader obj;
			if (!Assets::GetObj("Apple.obj", obj))
			{
				return new btSphereShape(btScalar(0.4));
			}

			Dali::Vector3 size = obj.GetSize();
			dlog_print(DLOG_DEBUG, "TIZENAR", "collision shape of apple : %f, %f, %f", size.x, size.y, size.z );

            btCollisionShape *shape = new btSphereShape(btScalar((size.x+size.y+size.z) / 6.0f));
            return shape;
        }

};

class DebugScene : public Scene
{
    public:
        DebugScene(Dali::Stage &stage, Dali::CameraActor &camera, Dali::Layer &uiLayer, 
            btDiscreteDynamicsWorld *dynamicsWorld)
            : Scene(stage, camera, uiLayer, dynamicsWorld),
              mLightDir(wVector3(0.2, 1, -0.3))
        {
        	dlog_print(DLOG_DEBUG, "TIZENAR", "debug scene created");
        	InitUI();
			dlog_print(DLOG_DEBUG, "TIZENAR", "debug scene init ui");
			// Debug_CreateContents();
//			CreateWater();

//			dlog_print(DLOG_DEBUG, "TIZENAR", "debug scene create floor");

//			dlog_print(DLOG_DEBUG, "TIZENAR", "debug scene create blocks");
//			CreateTorches();
//			CreateParticle();
//			CreateApple(wVector3(1,1,1));
//			dlog_print(DLOG_DEBUG, "TIZENAR", "debug scene create apple");
//			Test();
        }

        void OnStart() override
        {
        	CreateFloor();
        	CreateBlocks();
        }

        void OnUpdate(double deltaTime) override
        {
//        	UpdateWater(deltaTime);
//            UpdateFloor();
//            UpdateBlocks();
//            UpdateTorches();
//            UpdateParticles();
        }

        void OnKeyEvent(const Dali::KeyEvent &event) override
        {
        	static bool _state = true;

			if( event.state == KeyEvent::Down )
			{
				// s pressed
				if (event.keyCode == 39)
				{
					if(_state)
					{
						_parent->AddChild(_child);
					}
					else
					{
						_parent->RemoveChild(_child);
					}

					_state = not _state;
				}

				// a pressed
				if (event.keyCode == 38)
				{
					_parent->RotateBy(wQuaternion(Dali::Quaternion(Radian(0), Radian(Degree(90)), Radian(0))));
				}

				// d pressed
				if (event.keyCode == 40)
				{
					_child->RotateBy(wQuaternion(Dali::Quaternion(Radian(0), Radian(Degree(90)), Radian(0))));
				}
			}
        }

        void OnTouch(Dali::Actor actor, const Dali::TouchData &touch) override
        {
            if (touch.GetPointCount() > 0)
            {
                Dali::Vector2 touchPos = touch.GetScreenPosition(0);
                std::cout << "Touch!!!!!!!!!!!!!!" <<  touchPos << std::endl;
                if (touch.GetState(0) == PointState::DOWN)
                {
                    ScreenToWorldResult stw = mCamera->ScreenToWorld(touchPos);
                    auto apple = CreateApple( worldToPlanePos(stw.worldPos) );
                    apple->ApplyForce( worldToPlaneVec(stw.direction) * 10.0, wVector3(0, 0.015, 0));
                }
            }
        }

    private:
        void InitUI()
        {
            mLabel = Dali::Toolkit::TextLabel::New();
            cout << "label init pos is : " << mLabel.GetCurrentWorldPosition() << endl; 
            cout << "label init rot is : " << mLabel.GetCurrentWorldOrientation() << endl;
            mLabel.SetParentOrigin( Dali::ParentOrigin::TOP_LEFT );
            mLabel.SetAnchorPoint( Dali::AnchorPoint::TOP_LEFT );
            // Is it necessary?
            mLabel.SetSize(Dali::Vector2( mStage.GetSize().y * (640.0f/480.0f), mStage.GetSize().y ) / 5.0f);
            // The size of font in points
            mLabel.SetProperty( Dali::Toolkit::TextLabel::Property::POINT_SIZE, 30.0f );
            // After title text has been entered, it can be retrieved from the TEXT property
            mLabel.SetProperty( Dali::Toolkit::TextLabel::Property::TEXT, "title" );
            // The text can be aligned horizontally to the beginning, end, or center of the available area
            mLabel.SetProperty( Dali::Toolkit::TextLabel::Property::HORIZONTAL_ALIGNMENT, "CENTER" );
            // The text can be aligned vertically to the beginning, end, or center of the available area
            mLabel.SetProperty( Dali::Toolkit::TextLabel::Property::VERTICAL_ALIGNMENT, "CENTER" );
            AddUI(mLabel);
            mLabel.SetPosition(Dali::Vector3(0, 0, 0));
        }

        void Debug_CreateContents()
        {
            Dali::Shader cubeShader = LoadShaders("vertexPhong.glsl", "fragmentPhong.glsl");
            cubeShader.RegisterProperty("uLightPos", Dali::Vector3(1, -1, 11));
            cubeShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
            cubeShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));

            // Primitive models are defined in PrimitiveModels.h
            // Model need to receive its shader from the outside
            PrimitiveTexturedCube cubeModel("wood.png", cubeShader);

            // PhysicsActor behaves as dynamic rigidbody
            // and it inherits FrameActor which is basic concept of object
            auto cube1 = new PhysicsActor(mStage, cubeModel, mDynamicsWorld);
            cube1->SetName("Cube 1");
            cube1->SetPosition(-2, 0, 0);
            // Pass FrameActor so that it can be controlled and receive events
            AddActor(cube1);

            cubeModel.SetMass(1);
            auto cube2 = new PhysicsActor(mStage, cubeModel, mDynamicsWorld);
            cube2->SetName("Cube 2");
            cube2->SetPosition(-1, 3, 1);
            AddActor(cube2);

            // You can also remove FrameActor by using RemoveActor() method
            // And destructor of FrameActor will be called when removed
        }

        void CreateWater()
        {
            mWaterShader = LoadShaders("water_vert.glsl", "water_frag.glsl");
            mWaterShader.RegisterProperty("uLightDir", mLightDir.ToDali());
            mWaterShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
            mWaterShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));
            mWaterShader.RegisterProperty("uAlpha", 0.7f);

            PrimitivePlane floorModel("water.png", mWaterShader);

            auto water = new GraphicsActor(mStage, floorModel);
            water->SetName("Water");
            water->SetPosition(0, 0.025, 0);
            water->SetSize( 1, 0.05, 1 );
            AddActor(water);
        }

        void UpdateWater(double deltaTime)
        {
            static float passedTime = 0.0f;
            mWaterShader.RegisterProperty("uTime", passedTime);
            mWaterShader.RegisterProperty("uViewPos", mCamera->GetPlanePosition().ToDali());
            passedTime += deltaTime;
        }

        void CreateFloor()
        {
            mFloorShader = LoadShaders("vertexDiffuse.glsl", "fragmentDiffuse.glsl");
            mFloorShader.RegisterProperty("uLightDir", mLightDir.ToDali());
            mFloorShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
            Dali::Vector3 floorColor =  Dali::Vector3(239.0, 199.0, 166.0) / 255.0f;
            mFloorShader.RegisterProperty("uObjectColor", floorColor);
            
            PrimitiveCube floorModel(mFloorShader ); // not using texture. only for consturctor
            floorModel.SetMass(0);

            auto floorActor = new PhysicsActor(mStage, floorModel, mDynamicsWorld);
            floorActor->SetName("Floor");
            floorActor->SetPosition(0, 0.025, 0);
            floorActor->SetSize(0.3, 0.05, 0.3);
            AddActor(floorActor);
        }

        void UpdateFloor()
        {
            mFloorShader.RegisterProperty("uViewPos", mCamera->GetPlanePosition().ToDali());
        }

        void CreateBlocks()
        {
            mBlockShader = LoadShaders("vertexPhong.glsl", "fragmentPhong.glsl");
            mBlockShader.RegisterProperty("uLightPos", mLightDir.ToDali());
            mBlockShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
            mBlockShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));

            PrimitiveTexturedCube cubeModel("wood.png", mBlockShader);
            cubeModel.SetMass(0.5);

            auto cube1 = new PhysicsActor(mStage, cubeModel, mDynamicsWorld);
            cube1->SetName("Cube 1");
            cube1->SetPosition(0.035, 0.15, 0.0);
            cube1->SetSize(0.02, 0.08, 0.02);
            // Pass FrameActor so that it can be controlled and receive events
            AddActor(cube1);

            auto cube2 = new PhysicsActor(mStage, cubeModel, mDynamicsWorld);
            cube2->SetName("Cube 2");
            cube2->SetPosition(-0.035, 0.15, 0.0);
            cube2->SetSize(0.02, 0.08, 0.02);
            AddActor(cube2);

            auto cube3 = new PhysicsActor(mStage, cubeModel, mDynamicsWorld);
            cube3->SetName("Cube 3");
            cube3->SetPosition(0.0, 0.25, 0.0);
            cube3->SetSize(0.08, 0.02, 0.02);
            AddActor(cube3);
        }

        void UpdateBlocks()
        {
            mBlockShader.RegisterProperty("uViewPos", mCamera->GetPlanePosition().ToDali());
        }

        void CreateParticle()
        {
            Dali::Shader particleShader = LoadShaders("vertexParticle.glsl", "fragmentParticle.glsl");
            mParticleEffect = new ParticleEffect(mStage, particleShader, 10);
            // particleEffect->LoadTexture(0, "../res/images/001.jpg");
            mParticleEffect->LoadTexture(0, "FlameRoundParticleSheet.png");
            mParticleEffect->SetTile(10, 5);
            mParticleEffect->SetPosition(-0.1, 0.2, 0.1);
            mParticleEffect->SetSize(0.2, 0.2, 0.2);
            FireEmitter* fe = new FireEmitter();
            mParticleEffect->SetParticleEmitter(fe);
            AddActor(mParticleEffect);
        }

        void UpdateParticles()
        {
            mParticleEffect->SetRotation(wQuaternion(0, 0, 0, 1.0f));
        }

        void CreateTorches()
        {
            mTorchShader = LoadShaders("vertexPhong.glsl", "fragmentPhong.glsl");
            mTorchShader.RegisterProperty("uLightPos", mLightDir.ToDali());
            mTorchShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
            mTorchShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));
//            ObjLoader obj;
//            if (!Assets::GetObj("Campfire.obj", obj))
//            {
//                std::cout << "fuck! cant read obj" << std::endl;
//                return;
//            }
            PrimitiveObj objModel("Default_Palette.png", mTorchShader, "Campfire.obj");

            auto campfire = new GraphicsActor(mStage, objModel);
            //campfire->GetActor().GetRendererAt(0).SetProperty(Dali::Renderer::Property::FACE_CULLING_MODE, Dali::FaceCullingMode::FRONT);
            campfire->SetPosition(-0.1, 0.08, 0.1);
            campfire->SetSize(0.1, 0.1, 0.1);
            AddActor(campfire);
        }

        PhysicsActor* CreateApple(wVector3 pos)
        {
            mAppleShader = LoadShaders("vertexPhong.glsl", "fragmentPhong.glsl");
            mAppleShader.RegisterProperty("uLightPos", mLightDir.ToDali());
            mAppleShader.RegisterProperty("uViewPos", Dali::Vector3(0, 0, 0));
            mAppleShader.RegisterProperty("uLightColor", Dali::Vector3(1, 1, 1));
            ObjLoader obj;
            if (!Assets::GetObj("Apple.obj", obj))
            {
                std::cout << "fuck! cant read obj" << std::endl;
                return nullptr;
            }

            AppleModel appleModel("Default_Palette.png", mAppleShader);

            auto apple = new PhysicsActor(mStage, appleModel, mDynamicsWorld);
            apple->SetPosition(pos);
            apple->SetSize(0.1, 0.1, 0.1);
            AddActor(apple);

            return apple;
        }

        void UpdateTorches()
        {
            mTorchShader.RegisterProperty("uViewPos", mCamera->GetPlanePosition().ToDali());
        }

        void UpdateApple()
        {
            mAppleShader.RegisterProperty("uViewPos", mCamera->GetPlanePosition().ToDali());
        }

        void Test()
        {
        	Dali::Shader shader;
			if (not Assets::GetShader("vertexColor.glsl", "fragmentColor.glsl", shader))
				std::cout << "Failed to load shader." << std::endl;
			shader.RegisterProperty("uAlpha", 1.0f);
			PrimitiveCube model(shader);
			_parent = new GraphicsActor(mStage, model);
			_parent->SetPosition(-0.1, 0.02, 0);
			_parent->SetRotation( wQuaternion(0, 0.707107, 0, 0.707107) );
			_parent->SetSize(0.02, 0.02, 0.02);
			AddActor(_parent);

			_child = new GraphicsActor(mStage, model);
			_child->SetPosition(0.1, 0.02, 0);
			_child->SetSize(0.02, 0.02, 0.02);
			AddActor(_child);

			_parent->AddChild(_child);

			stringstream ss;

			ss << "parent's local position: " << _parent->GetLocalPosition();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "parent's local rotation: " << _parent->GetLocalRotation();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "parent's plane position: " << _parent->GetPlanePosition();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "parent's plane rotation: " << _parent->GetPlaneRotation();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");

			ss << "child's local position: " << _child->GetLocalPosition();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "child's local rotation: " << _child->GetLocalRotation();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "child's plane position: " << _child->GetPlanePosition();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "child's plane rotation: " << _child->GetPlaneRotation();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");

			_child->SetPosition(0.1, 0.02, 0, true);
			_child->SetRotation(wQuaternion(0, 0.707107, 0, 0.707107), true);
			ss << "child's local position: " << _child->GetLocalPosition();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "child's local rotation: " << _child->GetLocalRotation();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "child's plane position: " << _child->GetPlanePosition();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
			ss << "child's plane rotation: " << _child->GetPlaneRotation();
			dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
			ss.str("");
        }

    private:
        Dali::Toolkit::TextLabel mLabel;
        wVector3 mLightDir; // Directional Light

        // Debug
        vector<GraphicsActor*> mMapPointActors;

        // Water
        Dali::Shader mWaterShader;

        // Floor
        Dali::Shader mFloorShader;

        // Blocks
        Dali::Shader mBlockShader;

        // Particles
        ParticleEffect* mParticleEffect;

        // Objs
        Dali::Shader mTorchShader;
        Dali::Shader mAppleShader;

        FrameActor *_parent;
        FrameActor *_child;
};

#endif
