#pragma once

#include "EngineGlobal.h"
#include "TouchObject.h"
#include "SystemId.h"

class TouchEvent : public TouchObject
{
private:
    Transform m_PrevTransform;
    Transform m_InvTransform;
    Vector2   m_HalfSize;
    Id      m_TouchStageId;
    bool      m_ClampsTouch;

public:
    void Create(
        Component *pComponent,
        const Vector2 &size,
        Id touchStageId
        );

    void Destroy( void );

    void SetRect(
        const Vector2 &size
        );

    virtual void AddToScene( void );

    virtual void BeginTouch(
        const Vector2 &position
        );

    virtual void Touch(
        const Vector2 &position
        );

    virtual void EndTouch(
        const Vector2 &position
        );

    virtual void CancelTouch(
        const Vector2 &position
        );

    virtual void BeginHover(
        const Vector2 &position
        );

    virtual void Hover(
        const Vector2 &position
        );

    virtual void EndHover(
        const Vector2 &position
        );

    virtual void Swipe(
        const Vector2 &swipe
        );

    virtual bool InRange(
        float x,
        float y,
        float scale = 1.0f
        );

    virtual bool ClampsTouch( void ) const { return m_ClampsTouch; }

    void SetClampsTouch(
        bool clamps
        ) 
    { m_ClampsTouch = clamps;}

    Id GetTouchStageId( void ) const { return m_TouchStageId; }
    void SetTouchStageId( Id touchStageId );

private:
    void UpdateTransform( void );
};
