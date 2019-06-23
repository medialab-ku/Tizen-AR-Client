#include "PhysicsActor.h"
#include <sstream>
#include <dlog.h>

PhysicsActor::PhysicsActor(Dali::Stage &stage, Model &model, btDynamicsWorld *dynamicsWorld, wVector3 size)
    : GraphicsActor(stage, model),
      mDynamicsWorld(dynamicsWorld)
{
    // Collision shape data is provided by Model
    auto shape = model.CreateCollisionShape(size);
    FrameActor::SetSize(size);
    float mass = model.GetMass();
    btVector3 localInertia(0, 0, 0);
    shape->calculateLocalInertia(mass, localInertia);
    std::stringstream ss;
    ss << "mass : " << mass << ", inertia : " << localInertia.x() << ", " << localInertia.y() << ", " << localInertia.z();
    dlog_print(DLOG_DEBUG, "TIZENAR", ss.str().c_str() );

    btTransform transform;
    transform.setIdentity();

    btDefaultMotionState *ms = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, ms, shape, localInertia);
    rbInfo.m_friction = model.GetFriction();
    rbInfo.m_rollingFriction = model.GetRollingFriction();
    rbInfo.m_spinningFriction = model.GetSpinningFriction();
    rbInfo.m_restitution = model.GetRestitution();

    mRigidBody = new btRigidBody(rbInfo);
    mRigidBody->setActivationState(DISABLE_DEACTIVATION);
    mDynamicsWorld->addRigidBody(mRigidBody);
}

PhysicsActor::~PhysicsActor()
{
    if (mRigidBody && mRigidBody->getMotionState())
    {
        delete mRigidBody->getMotionState();
    }

    mDynamicsWorld->removeCollisionObject(mRigidBody);
    delete mRigidBody;
}

void
PhysicsActor::SetPosition(float x, float y, float z, bool setWorld)
{
    FrameActor::SetPosition(x, y, z, setWorld);
    auto transform = mRigidBody->getWorldTransform();
    wVector3 planePos = GetPlanePosition();
    transform.setOrigin(planePos.ToBullet());
    mRigidBody->setWorldTransform(transform);
}

void
PhysicsActor::SetPosition(wVector3 pos, bool setWorld)
{
    SetPosition(pos.x, pos.y, pos.z, setWorld);
}

void
PhysicsActor::SetRotation(float x, float y, float z, float w, bool setWorld)
{
    FrameActor::SetRotation(x, y, z, w, setWorld);
    auto transform = mRigidBody->getWorldTransform();
    wQuaternion planeRot = GetPlaneRotation();
    transform.setRotation(planeRot.ToBullet());
    mRigidBody->setWorldTransform(transform);
}

void
PhysicsActor::SetRotation(wQuaternion rotation, bool setWorld)
{
    SetRotation(rotation.x, rotation.y, rotation.z, rotation.w, setWorld);
}

void
PhysicsActor::RotateBy(wQuaternion rot)
{
    FrameActor::RotateBy(rot);
    SetRotation(mLocalRotation);
}

void
PhysicsActor::OnUpdate(double deltaTime)
{
    // Update transform of Dali::Actor
    btTransform trans;
    trans = mRigidBody->getWorldTransform();
    btVector3 pos = trans.getOrigin();
    btQuaternion rot = trans.getRotation();

    wVector3 wpos(pos);
    wQuaternion wrot(rot);
    FrameActor::SetPosition(wpos);
    FrameActor::SetRotation(wrot);
}

void
PhysicsActor::ApplyForce(wVector3 force)
{
    mRigidBody->applyCentralForce(force.ToBullet());
}

void
PhysicsActor::ApplyTorque(wVector3 torque)
{
	mRigidBody->applyTorque(torque.ToBullet());
}

void
PhysicsActor::SetVelocity(wVector3 vel)
{
    mRigidBody->setLinearVelocity(vel.ToBullet());
}

void
PhysicsActor::RemoveRigidBody()
{
	mDynamicsWorld->removeRigidBody(mRigidBody);
}
