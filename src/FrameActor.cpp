#include "FrameActor.h"
#include <iostream>

FrameActor::FrameActor(Dali::Stage &stage)
    : mStage(stage),
	  _spaceBasisX(wVector3::right),
	  _spaceBasisY(wVector3::up),
	  _spaceBasisZ(wVector3::forward),
	  _spaceOrigin(wVector3::zero),
      _parent(nullptr),
	  _plane(nullptr)
{
    mActor = Dali::Actor::New();
    mActor.SetAnchorPoint(Dali::AnchorPoint::CENTER);
    mActor.SetParentOrigin(Dali::ParentOrigin::CENTER);
    // Initial size of Dali Actor is... (0, 0, 0)... really!
    mActor.SetSize(Dali::Vector3(1, 1, 1));
    mStage.Add(mActor);
    FrameActor::SetPosition(0, 0, 0);
    FrameActor::SetRotation(0, 0, 0, 1);
}

FrameActor::FrameActor(Dali::Stage &stage, Dali::Actor &actor)
    : mStage(stage),
      mActor(actor)
{
    
}

FrameActor::~FrameActor()
{
    // Dali::Actor's destructor will be called
    mStage.Remove(mActor);
}

void
FrameActor::SetPosition(float x, float y, float z, bool setWorld)
{
	bool hasParent = (_parent != nullptr);
    mLocalPosition = (setWorld && hasParent) ? (wVector3(x, y, z) - _parent->GetPlanePosition()) : (wVector3(x, y, z));
    if(setWorld && hasParent) mActor.SetInheritPosition(false);
    mActor.SetPosition(mLocalPosition.ToDali());
    if(setWorld && hasParent) mActor.SetInheritPosition(true);
}

void
FrameActor::SetPosition(wVector3 position, bool setWorld)
{
	SetPosition(position.x, position.y, position.z, setWorld);
}

void
FrameActor::SetRotation(float x, float y, float z, float w, bool setWorld)
{
	bool hasParent = (_parent != nullptr);
    mLocalRotation = (setWorld && hasParent) ? (wQuaternion(x, y, z, w) * _parent->GetLocalRotation().Inverse()) : wQuaternion(x, y, z, w);
    if(setWorld && hasParent) mActor.SetInheritPosition(false);
    mActor.SetOrientation(mLocalRotation.ToDali());
    if(setWorld && hasParent) mActor.SetInheritPosition(true);
}

void
FrameActor::SetRotation(wQuaternion rotation, bool setWorld)
{
	SetRotation(rotation.x, rotation.y, rotation.z, rotation.w, setWorld);
}

void
FrameActor::SetSize(float x, float y, float z)
{
    mSize = wVector3(x, y, z);
    mActor.SetSize(mSize.ToDali());
}

void
FrameActor::SetSize(wVector3 size)
{
    mSize = size;
    mActor.SetSize(mSize.ToDali());
}

void
FrameActor::RotateBy(wQuaternion rot)
{
	auto curRot = mLocalRotation.ToDali();
	auto paramRot = rot.ToDali();
	auto newRot = paramRot * curRot;
	SetRotation(wQuaternion(newRot));
}

void
FrameActor::OnSpaceUpdated(FrameActor *plane, wVector3 basisX, wVector3 basisY, wVector3 basisZ, wVector3 origin)
{
    _plane = plane;
    _spaceBasisX = basisX;
    _spaceBasisY = basisY;
    _spaceBasisZ = basisZ;
    _spaceOrigin = origin;
}

void
FrameActor::AddChild(FrameActor *child)
{
	mActor.Add(child->GetActor());
	child->_parent = this;
}

void
FrameActor::RemoveChild(FrameActor *child)
{
	mActor.Remove(child->GetActor());
	if (_plane) _plane->AddChild(child);
	else child->_parent = nullptr;
}

void
FrameActor::Unparent()
{
    if (_parent)
    {
        _parent->RemoveChild(this);
    }
}

void
FrameActor::OnStart()
{
}

void
FrameActor::OnUpdate(double deltaTime)
{
}
