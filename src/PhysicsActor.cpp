#include "PhysicsActor.h"

PhysicsActor::PhysicsActor(Dali::Stage &stage, Model &model, btDynamicsWorld *dynamicsWorld)
    : GraphicsActor(stage, model),
      mDynamicsWorld(dynamicsWorld)
{
    // Collision shape data is provided by Model
    auto shape = model.CreateCollisionShape();
    float mass = model.GetMass();
    btVector3 localInertia(0, 0, 0);
    shape->calculateLocalInertia(mass, localInertia);

    btTransform transform;
    transform.setIdentity();

    btDefaultMotionState *ms = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, ms, shape, localInertia);
    rbInfo.m_friction = model.GetFriction();
    rbInfo.m_rollingFriction = model.GetRollingFriction();
    rbInfo.m_spinningFriction = model.GetSpinningFriction();
    rbInfo.m_restitution = model.GetRestitution();

    mRigidBody = new btRigidBody(rbInfo);
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
    mRigidBody->getMotionState()->setWorldTransform(transform);
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
    mRigidBody->getMotionState()->setWorldTransform(transform);
}

void
PhysicsActor::SetRotation(wQuaternion rotation, bool setWorld)
{
    SetRotation(rotation.x, rotation.y, rotation.z, rotation.w, setWorld);
}

void
PhysicsActor::SetSize(float x, float y, float z)
{
    FrameActor::SetSize(x, y, z);
    auto colShape = mRigidBody->getCollisionShape();
    colShape->setLocalScaling(btVector3(x, y, z));
}

void
PhysicsActor::SetSize(wVector3 size)
{
    SetSize(size.x, size.y, size.z);
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
    mRigidBody->getMotionState()->getWorldTransform(trans);
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
