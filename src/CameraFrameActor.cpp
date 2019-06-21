#include "CameraFrameActor.h"
#include <bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <cmath>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <iostream>
#include <sstream>
#include <dlog.h>

CameraFrameActor::CameraFrameActor(Dali::Stage &stage, Dali::CameraActor &actor)
    : FrameActor(stage, actor),
      mCameraActor(actor)
{
	fov = mCameraActor.GetFieldOfView();
	aspect = mCameraActor.GetAspectRatio();
	near = mCameraActor.GetNearClippingPlane();
	far = mCameraActor.GetFarClippingPlane();
}

Dali::Matrix
CameraFrameActor::GetProjMatrix()
{
	// Make Projection Matrix
	float e00 = 1.0f / tan(fov/2.0f) / aspect;
	float e11 = 1.0f / tan(fov/2.0f);
	float e22 = (far + near) / (far - near);
	float e23 = (2 * near * far) / (far - near);
	float e32 = -1;

	float arr[] = {
			-e00, 0, 0, 0,
			0, -e11, 0, 0,
			0, 0, -e22, -e32,
			0, 0, e23, 0
	};
	Dali::Matrix projMat(arr);

	Dali::Matrix projDaliMat;
	projMat = mCameraActor.GetCurrentProperty<Dali::Matrix>(Dali::CameraActor::Property::PROJECTION_MATRIX);
    std::stringstream ss;
	ss << "dali proj : "  << projDaliMat;
	dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
	ss.str("");
	ss << "eigen proj : " << projMat;
	dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
	ss.str("");

	return projMat;
}

Dali::Matrix
CameraFrameActor::GetViewMatrix()
{
	wVector3 p = GetLocalPosition();
	wQuaternion r = GetLocalRotation().Inverse();
	Eigen::Quaternionf q(r.w, r.x, r.y, r.z);
	Eigen::Matrix3f R = q.toRotationMatrix();
	Eigen::Vector3f T(p.x, p.y, p.z);
	T = R * T;

	float arr[] = {
			R(0, 0), R(1, 0), R(2, 0), 0,
			R(0, 1), R(1, 1), R(2, 1), 0,
			R(0, 2), R(1, 2), R(2, 2), 0,
			T(0), T(1), T(2), 1
	};
	Dali::Matrix viewMat(arr);

    std::stringstream ss;
    Dali::Matrix viewDali;
    viewDali = mCameraActor.GetCurrentProperty<Dali::Matrix>(Dali::CameraActor::Property::VIEW_MATRIX);

    ss << "dali view : "  << viewDali;
    dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
    ss.str("");
    ss << "eigen view : " << viewMat;
    dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
    ss.str("");
    return viewMat;
}

bool 
CameraFrameActor::Raycast(Dali::Vector2 touch, btDynamicsWorld *dynamicsWorld, RaycastHit &hit)
{
    // btDiscreteDynamicsWorld has rayTest function
    // if we can get screen to world ray,
    // than everything is ok

    Dali::Vector3 pos = mCameraActor.GetCurrentPosition();
    Dali::Vector4 rot = mCameraActor.GetCurrentOrientation().AsVector();
    float fov = mCameraActor.GetFieldOfView();
    float aspect = mCameraActor.GetAspectRatio();
    float far = mCameraActor.GetFarClippingPlane();
    float near = mCameraActor.GetNearClippingPlane();
    
    // Make Projection Matrix
    float e00 = 1.0f / tan(fov/2.0f) / aspect;
    float e11 = 1.0f / tan(fov/2.0f);
    float e22 = far / (far - near);
    float e23 = (near * far) / (far - near);
    float e32 = -1;
    Eigen::Matrix4f projMat;
    projMat << e00, 0, 0, 0,
            0, e11, 0, 0,
            0, 0, e22, e23,
            0, 0, e32, 0;

    Eigen::Matrix4f projMatInverse = projMat.inverse();

    // Make View Matrix
    Eigen::Quaternionf quat(rot.w, rot.x, rot.y, rot.z);
    Eigen::Matrix3f rotMat = quat.matrix();
    Eigen::Vector3f u = -rotMat.row(0);
    Eigen::Vector3f v = rotMat.row(1);
    Eigen::Vector3f n = -rotMat.row(2);
    Eigen::Vector3f eye = Eigen::Vector3f(pos.x, pos.y, pos.z);
    float eye_u = eye.dot(u);
    float eye_v = eye.dot(v);
    float eye_n = eye.dot(n);
    Eigen::Matrix4f viewMat;
    viewMat << u.x(), u.y(), u.z(), -eye_u,
            v.x(), v.y(), v.z(), -eye_v,
            n.x(), n.y(), n.z(), -eye_n,
            0, 0, 0, 1;

    Eigen::Matrix4f viewMatInverse = viewMat.inverse();

    // Get screen position (We are in Projection Space currently)
    Dali::Vector2 screenSize = mStage.GetSize();
    float x = 2.0 * touch.x / screenSize.x - 1;
    float y = 2.0 * touch.y / screenSize.y - 1;
    std::cout << "touch [" << x << ", " << y << "]" << std::endl;

    Eigen::Vector4f start_proj = Eigen::Vector4f(x, -y, 0, 1);
    // (end - start) will be ray direction
    Eigen::Vector4f end_proj = Eigen::Vector4f(x, -y, -1, 1);
    start_proj *= near;
    end_proj *= far;

    // Go Back to View Frustum
    Eigen::Vector4f start_view = projMatInverse * start_proj;
    Eigen::Vector4f end_view = projMatInverse * end_proj;

    // Go Back to World
    Eigen::Vector4f start_world = viewMatInverse * start_view;
    Eigen::Vector4f end_world = viewMatInverse * end_view;

    // Note that we are currently in the Dali coordinates.
    Dali::Vector3 from_dali(start_world.x(), start_world.y(), start_world.z());
    Dali::Vector3 to_dali(end_world.x(), end_world.y(), end_world.z());

    // Now we try raycast
    btVector3 from = wVector3(from_dali).ToBullet();
    btVector3 to = wVector3(to_dali).ToBullet();
    btCollisionWorld::AllHitsRayResultCallback result(from, to);
    result.m_flags |= btTriangleRaycastCallback::kF_KeepUnflippedNormal;
    result.m_flags |= btTriangleRaycastCallback::kF_UseSubSimplexConvexCastRaytest;

    std::cout << "Raycast from [" << from.getX() << ", " 
            << from.getY() << ", " << from.getZ() << "]"
            << " to [" << to.getX() << ", " 
            << to.getY() << ", " << to.getZ() << "]" 
            << std::endl;
    dynamicsWorld->rayTest(from, to, result);

    if (result.m_collisionObjects.size() > 0)
    {
        hit.rigidbody = (btRigidBody*)result.m_collisionObjects[0]->getUserPointer();
        hit.point = wVector3(result.m_hitPointWorld[0]);
        return true;
    }
    else
    {
        return false;
    }
}

ScreenToWorldResult 
CameraFrameActor::ScreenToWorld(Dali::Vector2 screen)
{
	std::stringstream ss;
	dlog_print(DLOG_DEBUG, "TIZENAR", "start debug");
	// screen to clip
	Dali::Vector2 screenSize = mStage.GetSize();
	// need to get screen points in landscape mode
	float y = (2.0 * screen.x / screenSize.x - 1);
	float x = (2.0 * screen.y / screenSize.y - 1);
	dlog_print(DLOG_DEBUG, "TIZENAR", "screen point : %f, %f", x, y);

	Dali::Vector4 clip_Start(x, y, -1, 1);
	Dali::Vector4 clip_End(x, y, 1, 1);

	// clip to camera
	Dali::Matrix projMat;
	//projMat = mCameraActor.GetCurrentProperty<Dali::Matrix>(Dali::CameraActor::Property::PROJECTION_MATRIX);
	projMat = GetProjMatrix();
	projMat.Invert();
	Dali::Matrix projMatInv = projMat;
	Dali::Vector4 camera_Start = projMatInv * clip_Start;
	Dali::Vector4 camera_End = projMatInv * clip_End;
	camera_Start /= camera_Start.w;
	camera_End /= camera_End.w;
	ss << "touch camera pos : " << camera_Start;
	dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
	ss.str("");

	// camera to world
	Dali::Matrix viewMat;
	//viewMat = mCameraActor.GetCurrentProperty<Dali::Matrix>(Dali::CameraActor::Property::VIEW_MATRIX);
	viewMat = GetViewMatrix();
	Dali::Matrix viewMatInv;
	viewMat.InvertTransform(viewMatInv);
	Dali::Vector4 world_Start = viewMatInv * camera_Start;
	Dali::Vector4 world_End = viewMatInv * camera_End;
	Dali::Vector4 dir = world_End - world_Start;
	ss << "touch world pos : " << world_Start << ", dir : " << dir;
	dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
	ss.str("");

	ScreenToWorldResult result;
	result.worldPos = wVector3(Dali::Vector3(world_Start.x, world_Start.y, world_Start.z));
	result.direction = wVector3(Dali::Vector3(dir.x, dir.y, dir.z)).Normalize();

	ss << "touch final pos : " << result.worldPos << ", camera pos : " << GetLocalPosition();
	dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str());
	ss.str("");
	return result;
}
