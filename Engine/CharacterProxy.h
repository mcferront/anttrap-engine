#pragma once

#include "EngineGlobal.h"
#include "PhysicsObject.h"
#include "CollisionObject.h"
#include "CollisionHandler.h"

class CharacterProxy : public PhysicsObject
{
private:
    CollisionObject *m_pCollision;

    Transform m_Transform;
    Vector    m_MovementRequest;
    Vector    m_Gravity;
    Vector    m_CollisionCorrection;
    Vector    m_LinearVelocity;
    Vector    m_CollisionNormal;

    bool  m_UseGravity;

public:
    void Create(
        Component *pComponent    
    );

    virtual void Destroy( void );

    void Bind(
        CollisionObject *pCollision
        );

    void ApplyMovement( 
        const Vector &delta 
        );

    CollisionObject *GetCollisionObject( void ) const { return m_pCollision; }

    void UseGravity( 
        bool useGravity
        )
    {
        m_UseGravity = useGravity;
    }

    void ApplyCollisionCorrection( 
        const Vector &delta 
        )
    {
        m_CollisionCorrection += delta;
        m_CollisionNormal += m_CollisionCorrection;
    }

public:
    virtual void BeginSimulation( 
        uint32 intervals
        );

    virtual void EndSimulation  ( void );
    virtual void Update         ( void );

    virtual void Resolve(
        float deltaTime   
        );

public:
    void SetTransform(
        const Transform &transform
        )
    { 
        m_Transform = transform;
        m_pCollision->SetTransform( transform );
    }

    void GetCollisionNormal(
        Vector *pNormal
        ) const
    {
        *pNormal = m_CollisionNormal;
    }
};

class CollisionDesc;

class CharacterProxyCollisionHandler : public ICollisionHandler
{
private:
    CharacterProxy *m_pCharacterProxy;

public:
    CharacterProxyCollisionHandler(
        CharacterProxy *pCharacterProxy
        );

    virtual void Run(
        const CollisionObject *pObjectA,
        const CollisionObject *pObjectB
        );

private:
    void Respond(
        const CollisionDesc &desc
        );
};
