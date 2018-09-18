#pragma once

#include "EngineGlobal.h"
#include "Component.h"

class PhysicsObject
{
private:
    Component *m_pComponent;

public:
    PhysicsObject( void )
    {
        m_pComponent = NULL;
    }

    virtual ~PhysicsObject( void ) {}

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

public:
    virtual void Create(
        Component *pComponent
        )
    {
        m_pComponent = pComponent;
    }

    virtual void BeginSimulation( 
        uint32 intervals
        ) {};

    virtual void EndSimulation  ( void ) {};
    virtual void Update         ( void ) {};

    virtual void Resolve(
        float deltaTime
        ) {};

    Component *GetComponent( void ) const { Debug::Assert( Condition(m_pComponent != NULL), "Component is null" ); return m_pComponent; }
};