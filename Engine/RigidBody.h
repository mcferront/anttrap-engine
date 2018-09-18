#pragma once

#include "EngineGlobal.h"
#include "PhysicsObject.h"
#include "CollisionObject.h"
#include "CollisionHandler.h"

class RigidBodyComponent;

class RigidBody : public PhysicsObject
{
private:
    Vector m_LinearForce;
    Vector m_AngularForce;

    Vector m_AngularVelocity;
    Vector m_LinearVelocity;

    Matrix m_Tensor;
    Matrix m_InvTensor;

    CollisionObject *m_pCollision;

    Transform m_SimulationTransform;

    float m_Mass;
    float m_InvMass;
    bool  m_UseGravity;

public:
    virtual void Create(
        Component *pComponent,
        const Vector &tensor,
        float mass,
        bool useGravity
        );

    void Destroy( void );

    void Bind(
        CollisionObject *pCollision
        );

    void ApplyLocalForce(
        const Vector &position,
        const Vector &force
        );

    void ApplyWorldForce(
        const Vector &position,
        const Vector &force
        );

    void ApplyLocalTorque(
        const Vector &torque
        );

    void ApplyWorldTorque(
        const Vector &torque
        );

    void ResetPhysics( void );

    void UseGravity( bool useGravity ) { m_UseGravity = useGravity; }

    float GetMass( void ) const { return m_Mass; }

    CollisionObject *GetCollisionObject( void ) const { return m_pCollision; }

    const Vector *GetLinearVelocity ( void ) const { return &m_LinearVelocity; }
    const Vector *GetAngularVelocity( void ) const { return &m_AngularVelocity; }

public:
    virtual void BeginSimulation( 
        uint32 intervals
        );

    virtual void EndSimulation  ( void );
    virtual void Update         ( void );

    virtual void Resolve(
        float deltaTime
        );
};

class CollisionDesc;

class RigidBodyCollisionHandler : public ICollisionHandler
{
private:
    RigidBody *m_pRigidBody;

public:
    RigidBodyCollisionHandler(
        RigidBody *pRigidBody
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
