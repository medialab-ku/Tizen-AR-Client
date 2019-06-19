#include <dali-toolkit/dali-toolkit.h>
#include <dali/dali.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <iostream>
#include <list>
#include <string>
#include <chrono>
#include "Assets.h"
#include "FrameActor.h"
#include "PhysicsActor.h"
#include "Shader.h"
#include "PrimitiveModels.h"
#include "TUM.h"
#include "ORB-SLAM2/System.h"
#include "ORB-SLAM2/Converter.h"
#include "Background.h"
#include "Scene.h"
#include "DebugScene.h"
#include "RealSense.h"
#include "FileSystem.h"
#include "Net.h"

#include <dlog.h>
#include <sstream>

const unsigned int UPDATE_INTERVAL = 33;
const int IMAGE_WIDTH = 320;
const int IMAGE_HEIGHT = 240;
const float Focal_X = 517.306408f;  // TUM
const float Focal_Y = 516.469215f;  // TUM
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const float CAMERA_NEAR = 0.001f;
const float CAMERA_FAR = 2000.0f;
const float CAMERA_ASPECT = (SCREEN_WIDTH * Focal_Y) / (SCREEN_HEIGHT * Focal_X);
const float CAMERA_FOV = atanf(SCREEN_HEIGHT / (2.0f * Focal_Y)) * 2;

class TizenRendererSystem : public Dali::ConnectionTracker
{
public:
    TizenRendererSystem(Dali::Application &application)
        : mApplication(application)
    {
        Assets::Init();
        dlog_print(DLOG_DEBUG, "TIZENAR", "asset init");
        InitBullet();
        dlog_print(DLOG_DEBUG, "TIZENAR", "bullet init");
        mApplication.InitSignal().Connect(this, &TizenRendererSystem::Create);
    }

    void Dispose()
    {
        delete mDynamicsWorld;
    }

private:
    void Create(Dali::Application &application)
    {
    	dlog_print(DLOG_DEBUG, "TIZENAR", "create start");

        Dali::Window winHandle = application.GetWindow();
        winHandle.ShowIndicator( Dali::Window::INVISIBLE );

        // create scene
        mStage = Dali::Stage::GetCurrent();
        mStage.KeyEventSignal().Connect( this, &TizenRendererSystem::OnKeyEvent );

        //string backgroundPath = "../res/images/001.jpg";
        string backgroundPath = FileSystem::GetResourcePath("images/001.jpg");
        CreateBackgroundImg(backgroundPath);
        dlog_print(DLOG_DEBUG, "TIZENAR", "background created");

        mStage.GetRootLayer().SetBehavior(Dali::Layer::LAYER_3D);
        
        // Camera default transform
        // Initial rotation is (0, 180, 0)
        unsigned int tc = mStage.GetRenderTaskList().GetTaskCount();
        dlog_print(DLOG_DEBUG, "TIZENAR", "task count : %d", tc);

        mCamera = mStage.GetRenderTaskList().GetTask(0).GetCameraActor();
        mStage.GetRenderTaskList().GetTask(0).SetCullMode( false );
        mCamera.SetNearClippingPlane(CAMERA_NEAR);
        mCamera.SetFarClippingPlane(CAMERA_FAR);
        cout << "camera fov, asepct : " << mCamera.GetFieldOfView() << ", " << mCamera.GetAspectRatio() << endl;
        mCamera.SetAspectRatio(CAMERA_ASPECT);
        mCamera.SetFieldOfView(CAMERA_FOV);
        mCamera.SetAnchorPoint(AnchorPoint::CENTER);
        mCamera.SetParentOrigin(ParentOrigin::CENTER);
        mCamera.SetPosition(Dali::Vector3(0, 0, 0));
        mCamera.SetOrientation(Quaternion(1, 0, 0, 0));
        InitUILayer();
        mUILayer.TouchSignal().Connect(this, &TizenRendererSystem::OnTouch);
        dlog_print(DLOG_DEBUG, "TIZENAR", "camera, ui create");

        // Set update loop
		mTimer = Dali::Timer::New(UPDATE_INTERVAL);
		mTimer.TickSignal().Connect(this, &TizenRendererSystem::Update);
		mTimer.Start();
		dlog_print(DLOG_DEBUG, "TIZENAR", "timer start");

        mScene = new DebugScene(mStage, mCamera, mUILayer, mDynamicsWorld);
        dlog_print(DLOG_DEBUG, "TIZENAR", "create done");
    }

    bool Update()
    {
        static int _updateCount = 0;

        double deltaTime = 0.0;
        if (_updateCount < 1)
        {
            // First frame that Dali Actors are not initialized yet
        	++_updateCount;
        }
        else if (_updateCount < 2)
        {
            // Second frame right after Dali Actors are initialized
            Net::BeginClient(9999); // blocked until connection
            mInitTime = std::chrono::high_resolution_clock::now();
            mOldTime = mInitTime;
            mCurrentTime = mInitTime;
            mScene->Start();
            ++_updateCount;
        }
        else
        {
            // Get elapsed time measured in seconds
            mCurrentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = mCurrentTime - mOldTime;
            deltaTime = elapsed.count();

            wVector3 normal, origin;

            if(ReceiveCameraData())
            {
            	UpdateBackgroundMat(_rgb);
            }

            if(ReceivePlaneData())
            {
            	if(mUpdatePlane)
				{
					cout << "calc plane" << endl;
					// dlog_print(DLOG_DEBUG, "TIZENAR", "plane update Eq: %f, %f, %f, %f", _planeEq(0), _planeEq(1), _planeEq(2), _planeEq(3));
					// dlog_print(DLOG_DEBUG, "TIZENAR", "plane update Pos: %f, %f, %f", _planePos(0), _planePos(1), _planePos(2));
					normal = wVector3( Eigen::Vector3f(_planeEq(0), _planeEq(1), _planeEq(2)) );
					normal.Normalize();
					if (normal.y > 0) normal = -normal;
					origin = wVector3(_planePos);
				}
            }

            mScene->Update(deltaTime, normal, origin, _camPos, _camRot);
        }

        mOldTime = mCurrentTime;
        return true;
    }

    bool ReceiveCameraData()
    {
    	if (not Net::IsConnected()) return false;
		Net::Send(Net::ID_CAM, nullptr, 0);
		while(not Net::Receive());
		//if (not Net::Receive()) return false;

		char *buf = Net::GetData();

		if(Net::GetId() != Net::ID_CAM) return false;

		Net::Mat output_left;
		Net::Vec3 output_pos;
		Net::Vec4 output_rot;
		Net::DecodeCameraData(buf, output_left, output_pos, output_rot);

		// 1. create cv::Mat from received data first
		_rgb = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, output_left.data);
		// 2. output_left.data will be freed out of this scope
		// 3. cv::Mat::clone() lets cv::Mat own its buffer so that _rgb will maintain its value
		_rgb = _rgb.clone();

		_camPos = wVector3( output_pos.x, output_pos.y, output_pos.z );
		_camRot = wQuaternion( output_rot.x, output_rot.y, output_rot.z, output_rot.w );


		delete[] buf;
		return true;
    }

    bool ReceivePlaneData()
    {
    	if (not Net::IsConnected()) return false;
		Net::Send(Net::ID_PLANE, nullptr, 0);
		while(not Net::Receive());
		//if (not Net::Receive()) return false;

		char *buf = Net::GetData();

		if(Net::GetId() != Net::ID_PLANE) return false;

		Net::Vec4 eq;
		Net::Vec3 pos;
		Net::DecodePlaneData(buf, eq, pos);

		_planeEq = Eigen::Vector4f(eq.x, eq.y, eq.z, eq.w);
		_planePos = Eigen::Vector3f(pos.x, pos.y, pos.z);

		delete[] buf;
		return true;
    }

    void OnKeyEvent( const KeyEvent& event )
    {
        if( event.state == KeyEvent::Down )
        {
            // cout << event.keyCode << endl;
            if ( IsKey( event, Dali::DALI_KEY_ESCAPE ) || IsKey( event, Dali::DALI_KEY_BACK ) )
            {
                mApplication.Quit();
            }
        }
        mScene->KeyEvent(event);
    }

    bool OnTouch(Dali::Actor actor, const Dali::TouchData &touch)
    {
        mScene->Touch(actor, touch);
        return true;
    }

    void InitBullet()
    {
        btDefaultCollisionConfiguration *cfg = new btDefaultCollisionConfiguration();
        btCollisionDispatcher *dispatcher = new btCollisionDispatcher(cfg);
        btBroadphaseInterface *overlappingPairCache = new btDbvtBroadphase();
        btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver();
        mDynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, cfg);
        mDynamicsWorld->setGravity(btVector3(0, -9.81, 0));
    }

    void InitUILayer()
    {
        cout << "init ui" << endl; 
        cout << "current camera pos is : " << mCamera.GetCurrentWorldPosition() << endl;

        float cameraZ = SCREEN_HEIGHT / (2.0f * std::tan(0.5f * CAMERA_FOV));

        mUILayer = Dali::Layer::New();
        cout << "mUILayer init pos is : " << mUILayer.GetCurrentWorldPosition() << endl; 
        cout << "mUILayer init rot is : " << mUILayer.GetCurrentWorldOrientation() << endl;
        mUILayer.SetParentOrigin( Dali::ParentOrigin::CENTER );
        mUILayer.SetAnchorPoint( Dali::AnchorPoint::CENTER );
        mUILayer.SetSize( Dali::Vector2( mStage.GetSize().y * CAMERA_ASPECT, mStage.GetSize().y ) );
        mCamera.Add( mUILayer );
        mUILayer.RaiseToTop();
        mUILayer.SetOrientation(Dali::Quaternion( Dali::Radian(Dali::Degree(180)) , Dali::Vector3(0, 1, 0) ));
        mUILayer.SetPosition(Dali::Vector3(0, 0, cameraZ));
    }



private:
    // Application
    Dali::Application &mApplication;

    Scene *mScene;
    btDiscreteDynamicsWorld *mDynamicsWorld;
    Dali::Stage mStage;
    Dali::CameraActor mCamera;
    Dali::Timer mTimer;
    bool mSceneStart = false;

    // UI
    Dali::Toolkit::Control mUIControl;
    Dali::Layer mUILayer;

    bool mUpdatePlane = true;

    // Time 
    std::chrono::time_point<std::chrono::high_resolution_clock> mCurrentTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> mOldTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> mInitTime;

    // Data from server
    wVector3 _camPos;
    wQuaternion _camRot;
    Eigen::Vector4f _planeEq = Eigen::Vector4f();
    Eigen::Vector3f _planePos = Eigen::Vector3f();
    cv::Mat _rgb;
};

// main function
int DALI_EXPORT_API main(int argc, char **argv)
{
    Dali::Application application = Dali::Application::New(&argc, &argv);
    TizenRendererSystem sys(application);
    application.MainLoop();
    sys.Dispose();
}
