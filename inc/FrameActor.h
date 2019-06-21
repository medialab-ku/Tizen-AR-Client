#ifndef __FRAMEACTOR_H__
#define __FRAMEACTOR_H__

#include <dali/dali.h>
#include <dali-toolkit/dali-toolkit.h>
#include <string>
#include <list>
#include "TypeWrappers.h"

/*
 * Basic object that managed by the system and can receive events.
 */
class FrameActor
{
public:
    FrameActor(Dali::Stage &stage);
    virtual ~FrameActor();
    Dali::Actor& GetActor() { return mActor; }
    std::string GetName() const { return mActor.GetName(); }
    virtual wVector3 GetLocalPosition() { return mLocalPosition; }
    virtual wQuaternion GetLocalRotation() { return mLocalRotation; }
    virtual wVector3 GetPlanePosition() { return (_parent!=nullptr ? mLocalPosition + _parent->GetPlanePosition() : mLocalPosition); }
    virtual wQuaternion GetPlaneRotation() { return (_parent!=nullptr ? _parent->GetPlaneRotation() * mLocalRotation : mLocalRotation); }
    wVector3 GetSize() { return mSize; }
    void SetName (const std::string name) { mActor.SetName(name); }
    virtual void SetPosition(float x, float y, float z, bool setWorld=false);
    virtual void SetPosition(wVector3 position, bool setWorld=false);
    virtual void SetRotation(float x, float y, float z, float w, bool setWorld=false);
    virtual void SetRotation(wQuaternion rotation, bool setWorld=false);
    virtual void SetSize(float x, float y, float z);
    virtual void SetSize(wVector3 size);
    virtual void RotateBy(wQuaternion rot);

    // Hierarchy (It's not completed. Don't use this.)
    void AddChild(FrameActor *child);
    void RemoveChild(FrameActor *child);
    void Unparent();

    virtual void OnSpaceUpdated(FrameActor *plane, wVector3 basisX, wVector3 basisY, wVector3 basisZ, wVector3 origin);

    /*
     * Called at first frame right after the Dali::Actor is initialized.
     * Precisely, it is second frame since the system begun.
     */
    virtual void OnStart();
    /*
     * Being called every timer event
     */
    virtual void OnUpdate(double deltaTime);

    void RemoveActor();

protected:
    FrameActor(Dali::Stage &stage, Dali::Actor &actor);
    
protected:
    Dali::Actor mActor;
    Dali::Stage mStage;
    wVector3 mLocalPosition;
    wQuaternion mLocalRotation;
    wVector3 mSize;
    FrameActor *_parent;
    FrameActor *_plane;
    wVector3 _spaceBasisX, _spaceBasisY, _spaceBasisZ;
    wVector3 _spaceOrigin;
};

#endif
