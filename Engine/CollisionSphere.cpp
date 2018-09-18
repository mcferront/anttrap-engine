#include "EnginePch.h"

#include "CollisionSphere.h"
#include "PhysicsWorld.h"
#include "DebugGraphics.h"

void CollisionSphereVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void CollisionSphere::Create(
    Component *pComponent,
    float restitution,
    float friction
    )
{
    static bool registered;

    if ( false == registered )
    {
        registered = true;
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeSphere, CollisionTypeSphere, CollisionSphereVsCollisionSphere );
    }

    CollisionObject::Create( pComponent );

    m_Radius   = 1.0f;
    m_Position = Math::ZeroVector();

    memset( &m_SurfaceDesc, 0, sizeof(m_SurfaceDesc) );

    m_SurfaceDesc.spring = restitution;
    m_SurfaceDesc.friction = friction;
}

void CollisionSphere::Destroy( void )
{
}

void CollisionSphereVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    CollisionSphere *pSphereA = (CollisionSphere *) pObjectA;
    CollisionSphere *pSphereB = (CollisionSphere *) pObjectB;

    Vector centerA, centerB;
    float radiusA, radiusB;

    radiusA = pSphereA->GetRadius( );
    radiusB = pSphereB->GetRadius( );

    pSphereA->GetPosition( &centerA );
    pSphereB->GetPosition( &centerB );

    float distanceSq = Math::MagnitudeSq( centerA - centerB );

    if ( distanceSq >= (radiusA + radiusB) * (radiusA + radiusB) ) 
        return;

    Vector normal;

    float distance = Math::Normalize( &normal, centerB - centerA );
    float penetration = (radiusA + radiusB) - distance;

    Vector intersection = centerA + normal * (radiusA - penetration);

    pDesc->Add( - normal, intersection, penetration );

    if ( pDesc->count > 0 )
    {
        pSphereA->GetPosition( &pDesc->surfaceA.position );
        pSphereB->GetPosition( &pDesc->surfaceB.position );

        pDesc->pObjectA = pObjectA;
        pDesc->pObjectB = pObjectB;

        pDesc->surfaceA.linear  = *pSphereA->GetLinearVelocity( );
        pDesc->surfaceA.angular = *pSphereA->GetAngularVelocity( );
        pDesc->surfaceA.spring  =  pSphereA->GetSpring( );
        pDesc->surfaceA.friction=  pSphereA->GetFriction( );

        pDesc->surfaceB.linear  = *pSphereB->GetLinearVelocity( );
        pDesc->surfaceB.angular = *pSphereB->GetAngularVelocity( );
        pDesc->surfaceB.spring  =  pSphereB->GetSpring( );
        pDesc->surfaceB.friction=  pSphereB->GetFriction( );

        pDesc->collision = true;
    }
}
