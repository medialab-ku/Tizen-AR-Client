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

#include <dlog.h>
#include <sstream>

const unsigned int UPDATE_INTERVAL = 64;
const float Focal_X = 517.306408f;  // TUM
const float Focal_Y = 516.469215f;  // TUM
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 720;
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
        InitSLAM();
        dlog_print(DLOG_DEBUG, "TIZENAR", "slam init");
        mApplication.InitSignal().Connect(this, &TizenRendererSystem::Create);
    }

    void Dispose()
    {
        mScene->Dispose();
        delete mDynamicsWorld;

        SLAM->SaveTrajectoryTUM(FileSystem::GetDataPath("CameraTrajectory.txt"));
        SLAM->SaveMapPoints(FileSystem::GetDataPath("MapPoints.txt"));
        SLAM->SaveDetectedPlane(FileSystem::GetDataPath("Plane.txt"));
        SLAM->Shutdown();
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

        CreatePlane();

        mScene = new DebugScene(mStage, mCamera, mUILayer, mDynamicsWorld, mPlane, SLAM);
        dlog_print(DLOG_DEBUG, "TIZENAR", "create done");
        // mScene->Init();
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
            ++_updateCount;
            mInitTime = std::chrono::high_resolution_clock::now();
            mOldTime = mInitTime;
            mCurrentTime = mInitTime;
        }
        else
        {
            // Get elapsed time measured in seconds
            mCurrentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = mCurrentTime - mOldTime;
            deltaTime = elapsed.count();
            if(mSeq) sensor.GetImage(left, depth);
            std::chrono::duration<double> time = mCurrentTime - mInitTime;

            // Camera update
            UpdateCamera(time);
            
            // Plane detection
            Eigen::Vector4f planeEq;
            Eigen::Vector3f planePos;
            int planeInlier;
            SLAM->GetPlaneDetector()->GetPlane(planeEq, planePos, planeInlier);
            if(planeInlier >= mPlaneInliers && mUpdatePlane)
            {
                cout << "calc plane" << endl;
                dlog_print(DLOG_DEBUG, "TIZENAR", "plane update");
                wVector3 normal( Eigen::Vector3f(planeEq(0), planeEq(1), planeEq(2)) );
                normal.Normalize();
                if (normal.y < 0) normal = wVector3(-normal.x, -normal.y, -normal.z);

                Dali::Vector3 n = normal.ToDali();
				Dali::Vector3 z = Dali::Vector3(-1, 0, 0).Cross(n);
				Dali::Vector3 x = n.Cross(z);
				wQuaternion rotation( Dali::Quaternion(x, n, z) );
                //wQuaternion rotation( Dali::Quaternion(Dali::Vector3(0, 1, 0), normal.ToDali()) );
                mPlane->SetPosition(wVector3(planePos));
                mPlanePos = wVector3(planePos).ToDali();
                mPlane->SetRotation(rotation);
                mPlaneRot = rotation.ToDali();
                mPlaneInliers = planeInlier;

                std::stringstream ss, ss2;
				ss << "Plane pos : " << mPlanePos;
				ss2 << "Plane rot : " << mPlaneRot.mVector;
				dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
				dlog_print(DLOG_DEBUG, "TIZENAR", ss2.str().c_str());

                float gravity = normal.y > 0 ? -9.81f : 9.81f;
                mDynamicsWorld->setGravity(normal.ToBullet() * gravity);

                OnPlaneUpdated();
            }

            UpdateBackgroundMat(left);

            if(mSceneStart) mScene->Update(deltaTime);
        }

        mOldTime = mCurrentTime;
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

            // q pressed
            if(event.keyCode == 24)
            {
                mUpdatePlane = !mUpdatePlane;
                cout << "plane detection mode changed to" << mUpdatePlane << endl;
            }

            // s pressed. start the demo scene
            if(event.keyCode == 39)
            {
                mSceneStart = !mSceneStart;
            }

            // p pressed. stop the image seqence
            if (event.keyCode == 33)
            {
                mSeq = !mSeq;
            }

            // space pressed. shoot the apple from camera
            if(event.keyCode == 65)
            {

            }

            // left
            if(event.keyCode == 113)
            {
                Dali::Vector3 lp = mUILayer.GetCurrentPosition();
                mUILayer.SetPosition(lp - Dali::Vector3(0, 0, 1));
                cout << "changed to : " << mUILayer.GetCurrentPosition() << endl;
            }
            // right
            else if(event.keyCode == 114)
            {
                Dali::Vector3 lp = mUILayer.GetCurrentPosition();
                mUILayer.SetPosition(lp + Dali::Vector3(0, 0, 1));
                cout << "changed to : " << mUILayer.GetCurrentPosition() << endl;
            }
        }

        mScene->OnKeyEvent(event);
    }

    bool OnTouch(Dali::Actor actor, const Dali::TouchData &touch)
    {
        mScene->OnTouch(actor, touch);
        return true;
    }

    void OnPlaneUpdated()
    {
        static bool _first = true;
        if (_first)
        {
            // Scene should be started after the main plane is detected.
            mScene->Init();
            dlog_print(DLOG_DEBUG, "TIZENAR", "scene init");
            mScene->OnStart();
            dlog_print(DLOG_DEBUG, "TIZENAR", "scene start");
            mSceneStart = true;
            _first = false;
        }
    }

    void UpdateCamera(std::chrono::duration<double> time)
    {
        try
        {
            cv::Mat Tcw = SLAM->TrackRGBD(left, depth, time.count());
            cv::Mat Rwc = Tcw.rowRange(0,3).colRange(0,3).t();
            cv::Mat twc = -Rwc*Tcw.rowRange(0,3).col(3);
            vector<float> q = ORB_SLAM2::Converter::toQuaternion(Rwc);
            wVector3 CameraPos( Eigen::Vector3f(twc.at<float>(0), twc.at<float>(1), twc.at<float>(2)) );
            wQuaternion CameraRot( Eigen::Vector4f(q[0], q[1], q[2], q[3]) );
            CameraRot = CameraRot.Inverse(); // ORB-SLAM2 returns C->W. So we need inverse of that quaternion.
//            std::stringstream ss, ss2;
//			  ss << "camera pos: " << CameraPos.ToDali();
//			  ss2 << "camera rot:  " << CameraRot.ToDali().mVector;
//			  dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
//			  dlog_print(DLOG_DEBUG, "TIZENAR", ss2.str().c_str());
            mCamera.SetPosition( CameraPos.ToDali() );
            mCamera.SetOrientation( CameraRot.ToDali() );
        }
        catch(const std::exception& e)
        {
            cout << "tracking failed!" << endl;
        }
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

    void InitSLAM()
    {
        //const string orb = "../res/SLAM/ORB.bin";
    	const string orb = FileSystem::GetResourcePath("SLAM/ORB.bin");
        //const string settings = "../res/SLAM/TUM1.yaml";
        const string settings = FileSystem::GetResourcePath("SLAM/TUM1.yaml");
        SLAM = new ORB_SLAM2::System(orb, settings, ORB_SLAM2::System::RGBD);
        dlog_print(DLOG_DEBUG, "TIZENAR", "slam system init");

        //sensor.LoadImages("../res/SLAM/fr1_xyz.txt", "rgbd_dataset_freiburg1_xyz");
        sensor.LoadImages(FileSystem::GetResourcePath("SLAM/fr1_xyz.txt"), "rgbd_dataset_freiburg1_xyz");
        dlog_print(DLOG_DEBUG, "TIZENAR", "slam img init");
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

    void CreatePlane()
    {
        mPlaneShader = LoadShaders("vertexColor.glsl", "fragmentColor.glsl");
        mPlaneShader.RegisterProperty("uAlpha", 0.3f);


        PrimitiveCube floorModel("wood.png", mPlaneShader);
        
        mPlane = new PhysicsActor(mStage, floorModel, mDynamicsWorld);
        mPlane->SetName("Plane");
        mPlane->SetPosition(0, 0, 0);
        mPlane->SetSize( 1, 0.05, 1 );

        // plane position for debugging
		// mPlane->SetPosition(wVector3(Dali::Vector3(0.0504673, 0.0932271, 1.283)));
		// mPlane->SetRotation(wQuaternion(Dali::Quaternion(-0.244961, 0.740481, 0.193315, -0.595241)));
    }

private:
    // Application
    Dali::Application &mApplication;

    Scene *mScene;
    btDiscreteDynamicsWorld *mDynamicsWorld;
    Dali::Stage mStage;
    Dali::CameraActor mCamera;
    Dali::Timer mTimer;
    bool mSeq = true;
    bool mSceneStart = false;

    // UI
    Dali::Toolkit::Control mUIControl;
    Dali::Layer mUILayer;

    // Plane
    Dali::Shader mPlaneShader;
    PhysicsActor* mPlane;
    Dali::Vector3 mPlanePos;
    Dali::Quaternion mPlaneRot;
    int mPlaneInliers = 0;
    bool mUpdatePlane = true;

    // Time 
    std::chrono::time_point<std::chrono::high_resolution_clock> mCurrentTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> mOldTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> mInitTime;

    // slam & sensor
    ORB_SLAM2::System* SLAM;
    TUM sensor;
    cv::Mat left, depth;
    //RealSense realSense;
};

// main function
int DALI_EXPORT_API main(int argc, char **argv)
{
    Dali::Application application = Dali::Application::New(&argc, &argv);
    TizenRendererSystem sys(application);
    application.MainLoop();
    sys.Dispose();
}
