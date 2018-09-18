#pragma once

#include "EngineGlobal.h"
#include "SystemId.h"
#include "PhysicsWorld.h"
#include "Component.h"
#include "Node.h"

class CollisionObject
{
private:
    CollisionType  m_Type;
    Component     *m_pComponent;
    
    mutable uint32 m_LayerBitFlag;

public:
    CollisionObject(
        CollisionType type
        )
    {
        m_Type  = type;
        m_LayerBitFlag = -1;
        m_pComponent = NULL;
    }

    virtual ~CollisionObject( void ) {}

    virtual void AddToScene( void );

    virtual void RemoveFromScene( void );

    virtual void Create(
        Component *pComponent
        ) 
    { 
        m_pComponent = pComponent;
    }

    virtual void Destroy( void ) {};

    virtual void SetTransform(
        const Transform &transform
        )
    {}

    virtual void SetSurfaceVelocity(
        const Vector &linearVelocity,
        const Vector &angularVelocity
        )
    {}

    virtual void Render(
        const Vector &color
    )
    {}

    Component *GetComponent( void ) const { Debug::Assert( Condition(m_pComponent != NULL), "Component is null" ); return m_pComponent; }

    CollisionLayer GetCollisionLayerBitFlag( void ) const 
    { 
        if (-1 == m_LayerBitFlag)
            m_LayerBitFlag = PhysicsWorld::Instance( ).LayerToBitFlag( m_pComponent->GetParent()->GetCollisionLayer() ); 

        return m_LayerBitFlag; 
    }
    
    CollisionType  GetCollisionType( void )  const { return m_Type; }

protected:
    void SetCollisionLayerBitFlag(
        const char *pLayer
        )
    {
        m_LayerBitFlag = PhysicsWorld::Instance( ).LayerToBitFlag( Id(pLayer) ); 
    }
};
