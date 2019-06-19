#ifndef __PHYSICSACTOR_H__
#define __PHYSICSACTOR_H__

#include <bullet/btBulletDynamicsCommon.h>
#include "GraphicsActor.h"

class PhysicsActor : public GraphicsActor
{
public:
    PhysicsActor(Dali::Stage& stage, Model &model, btDynamicsWorld *dynamicsWorld);
    ~PhysicsActor() override;
    btRigidBody* GetRigidBody() const { return mRigidBody; }
    void SetPosition(float x, float y, float z, bool setWorld=false) override;
    void SetPosition(wVector3 position, bool setWorld=false) override;
    void SetRotation(float x, float y, float z, float w, bool setWorld=false) override;
    void SetRotation(wQuaternion rotation, bool setWorld=false) override;
    void SetSize(float x, float y, float z) override;
    void SetSize(wVector3 size) override;
    void RotateBy(wQuaternion rot) override;
    void OnUpdate(double deltaTime) override;
    void ApplyForce(wVector3 force);

private:
    btRigidBody *mRigidBody;
    btDynamicsWorld *mDynamicsWorld;
};

#endif
