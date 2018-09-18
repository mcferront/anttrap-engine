#pragma once

#include "EngineGlobal.h"

class Component;

class TouchObject
{
private:
    Component *m_pComponent;

public:
    virtual ~TouchObject( void ) {}

    virtual void Create(
        Component *pComponent
    )
    {
        m_pComponent = pComponent;
    }

    Component *GetComponent( void ) const { Debug::Assert( Condition(m_pComponent != NULL), "Component is null" ); return m_pComponent; }

    virtual void BeginTouch(
        const Vector2 &position
        ) = 0;

    virtual void Touch(
        const Vector2 &position
        ) = 0;

    virtual void EndTouch(
        const Vector2 &position
        ) = 0;

    virtual void CancelTouch(
        const Vector2 &position
        ) = 0;

    virtual void BeginHover(
        const Vector2 &position
        ) = 0;

    virtual void Hover(
        const Vector2 &position
        ) = 0;

    virtual void EndHover(
        const Vector2 &position
        ) = 0;

    virtual void Swipe(
        const Vector2 &swipe
        ) = 0;

    virtual bool InRange(
        float x,
        float y,
        float scale = 1.0f
        ) = 0;

    virtual void AddToScene(
        Id touchStageId
        );

    virtual bool ClampsTouch( void ) const = 0;

    virtual void RemoveFromScene( void );
};
