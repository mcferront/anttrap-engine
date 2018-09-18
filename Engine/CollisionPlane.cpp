#include "EnginePch.h"

#include "CollisionPlane.h"
#include "CollisionSphere.h"
#include "PhysicsWorld.h"
#include "DebugGraphics.h"

void CollisionPlaneVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void CollisionSphereVsCollisionPlane(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void CollisionPlane::Create(
    Component *pComponent,
    float restitution,
    float friction
    )
{
    static bool registered;

    if ( false == registered )
    {
        registered = true;
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypePlane, CollisionTypeSphere, CollisionPlaneVsCollisionSphere );
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeSphere, CollisionTypePlane, CollisionSphereVsCollisionPlane );
    }

    CollisionObject::Create( pComponent );

    memset( &m_SurfaceDesc, 0, sizeof(m_SurfaceDesc) );

    m_SurfaceDesc.spring = restitution;
    m_SurfaceDesc.friction = friction;
}

void CollisionPlane::Destroy( void )
{
}

void CollisionSphereVsCollisionPlane(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    CollisionPlaneVsCollisionSphere( pObjectB, pObjectA, pDesc );
    pDesc->Swap( );
}

void CollisionPlaneVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    CollisionPlane  *pPlane  = (CollisionPlane *) pObjectA;
    CollisionSphere *pSphere = (CollisionSphere *) pObjectB;

    Vector center, plane;
    float radius;

    radius = pSphere->GetRadius( );

    pSphere->GetPosition( &center );
    pPlane ->GetPlane( &plane );

    float distance = Math::DotProduct( center, plane );

    float penetration = - (distance + plane.w - radius);
    if ( penetration <= 0 ) return;


    Vector normal = Vector( plane.x, plane.y, plane.z );

    Vector intersection = center + -normal * radius;

    pDesc->Add( - normal, intersection, penetration );

    if ( pDesc->count > 0 )
    {
        pDesc->surfaceA.position = intersection;
        pSphere->GetPosition( &pDesc->surfaceB.position );

        pDesc->pObjectA = pObjectA;
        pDesc->pObjectB = pObjectB;

        pDesc->surfaceA.linear  = *pPlane->GetLinearVelocity( );
        pDesc->surfaceA.angular = *pPlane->GetAngularVelocity( );
        pDesc->surfaceA.spring  =  pPlane->GetSpring( );
        pDesc->surfaceA.friction=  pPlane->GetFriction( );

        pDesc->surfaceB.linear  = *pSphere->GetLinearVelocity( );
        pDesc->surfaceB.angular = *pSphere->GetAngularVelocity( );
        pDesc->surfaceB.spring  =  pSphere->GetSpring( );
        pDesc->surfaceB.friction=  pSphere->GetFriction( );

        pDesc->collision = true;
    }
}
