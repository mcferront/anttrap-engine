#include "EnginePch.h"

#include "Raycast.h"
#include "PhysicsWorld.h"
#include "DebugGraphics.h"
#include "CollisionSphere.h"
#include "CollisionPlane.h"
#include "CollisionMesh.h"
#include "Node.h"

void RaycastVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void RaycastVsCollisionPlane(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void RaycastVsCollisionMesh(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

bool Raycast::Cast( 
    RaycastResult *pResult 
    )
{
    static bool registered;

    if ( false == registered )
    {
        registered = true;
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeRaycast, CollisionTypeSphere, RaycastVsCollisionSphere );
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeRaycast, CollisionTypePlane,  RaycastVsCollisionPlane );
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeRaycast, CollisionTypeMesh,  RaycastVsCollisionMesh );
    }

    RaycastCollisionHandler handler( pResult );

    PhysicsWorld::Instance( ).RunCollisionHandler( this, &handler );

    return pResult->isValid;
}


void RaycastVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    Raycast *pRay = (Raycast *) pObjectA;
    CollisionSphere *pSphere = (CollisionSphere *) pObjectB;

    Vector center, projectedCenter;

    float radius   = pSphere->GetRadius( );
    float radiusSq = radius * radius;
    float length   = pRay->GetLength( );

    Vector direction = *pRay->GetDirection( );
    Vector start     = *pRay->GetStart( );

    //project sphere center on to ray
    pSphere->GetPosition( &center );

    float distanceOnRay = Math::DotProduct( center - start, direction );

    //if we're behind the ray, ignore it
    if ( distanceOnRay + radius < 0 ) return;

    //projectedCenter = direction * distanceOnRay;

    //if we're too far from the end of the ray
    //float distToStartSq = Math::MagnitudeSq(projectedCenter);
    if (  distanceOnRay - radius > length ) return;

    //distToEdge is the distance from projected center to the edge of the circle (along the normal)
    Vector worldPositionOnRay = start + direction * distanceOnRay;

    float distanceSq = Math::MagnitudeSq( worldPositionOnRay - center );

    //if we're too far from the ray's line
    if ( distanceSq > radiusSq ) return;

    //distToEdge is the distance from projected center to the edge of the circle (along the normal)
    float distance = Math::Sqrt( distanceSq );

    float adjustedRadius = distance / radius * Math::DegreesToRadians(90.0f);
    adjustedRadius = Math::Cos( adjustedRadius );

    Vector intersection = worldPositionOnRay - direction * (adjustedRadius * pSphere->GetRadius());

    Vector normal = intersection - center;
    Math::Normalize( &normal, normal );

    pDesc->Add( normal, intersection, 0.0f );
    pDesc->collision = true;
}

void RaycastVsCollisionPlane(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    Raycast *pRay = (Raycast *) pObjectA;
    CollisionPlane *pPlane = (CollisionPlane *) pObjectB;

    Vector plane;
    pPlane->GetPlane( &plane );

    Vector end = *pRay->GetEnd( );
    Vector start = *pRay->GetStart( );
    Vector direction = *pRay->GetDirection( );

    //which side of the plane is the ray startbon
    float distance = Math::DotProduct( plane, start );
    float startPenetration = (distance + plane.w);

    //which side of the plane is the ray end on
    distance = Math::DotProduct( plane, end );   
    float endPenetration = (distance + plane.w);

    //if start and end are on the same side of the plane
    //then there is no collision
    if ( endPenetration > 0 && startPenetration > 0 ) return;
    if ( endPenetration < 0 && startPenetration < 0 ) return;

    //Treat as triangle a, b, c
    //a == ray start to plane along plane normal
    //b == ray start at plane to ray/plane intersect
    //c == ray start to plane intersect
    float a = startPenetration;

    //angle of a
    //cosine of angle of plane normal and ray direction
    float angle = Math::DotProduct( -plane, direction );
    angle = Math::ACos( angle );
    
    //Tan of angle * ray start to plane (a) gives the length of b
    float b = a * Math::Tan( angle );

    //c
    float c = Math::Sqrt( a * a + b * b );

    //now that we know the length of c we can figure out intersection along ray
    Vector intersection = start + direction * c;

    Vector normal = Vector( plane.x, plane.y, plane.z );

    pDesc->Add( normal, intersection, 0.0f );
    pDesc->collision = true;
}

void RayVsTriangle(
    const Collider::Triangle *pTriangle,
    const Vector &localStart,
    const Vector &localDirection,
    const Vector &worldStart,
    const Vector &worldDirection,
    float length,
    const Transform *pWorldTransform,
    CollisionDesc *pDesc
    )
{
    Vector start = localStart;
    Vector direction = localDirection;

    Vector faceNormal = pTriangle->normal;

    Vector end = start + direction * length;

    //which side of the triangle face is the ray startbon
    float startPenetration = Math::DotProduct( faceNormal, start - pTriangle->positions[0] );
    float endPenetration = Math::DotProduct( faceNormal, end - pTriangle->positions[0] );
    
    //if start and end are on the same side of the triangle face
    //then there is no collision
    if ( endPenetration > 0 && startPenetration > 0 ) return;
    if ( endPenetration < 0 && startPenetration < 0 ) return;

    Vector e1 = pTriangle->positions[1] - pTriangle->positions[0];
    Vector e2 = pTriangle->positions[2] - pTriangle->positions[0];

    Vector p;
    Math::CrossProduct( &p, e2, direction );

    float a = Math::DotProduct( e1, p );

    if ( a > -0.00001f && a < .00001f ) return;

    float f = 1.0f / a;
    Vector s = start - pTriangle->positions[0];

    float u = f * Math::DotProduct(s, p);

    if ( u < 0 || u > 1 ) return;

    Vector q;
    Math::CrossProduct( &q, e1, s );
    
    float v = f * Math::DotProduct( direction, q );

    if ( v < 0 || u + v > 1 ) return;

    float t = f * Math::DotProduct( e2, q );

    Vector intersection;
    
    intersection = pTriangle->positions[ 0 ] * (1 - u - v) +
                   pTriangle->positions[ 1 ] * u +
                   pTriangle->positions[ 2 ] * v;

    if ( NULL != pWorldTransform )
    {
        Math::TransformPosition( &intersection, intersection, *pWorldTransform );
        Math::Rotate( &faceNormal, faceNormal, *pWorldTransform );
    }

    if ( pDesc->count > 0 )
    {
        float existingDepth = Math::DotProduct( worldDirection, pDesc->desc[0].intersection - worldStart );
        float depth = Math::DotProduct( worldDirection, intersection - worldStart );
    
        if ( Math::Abs(depth) < Math::Abs(existingDepth) )
            pDesc->count = 0;
    }

    if ( 0 == pDesc->count )
    {
        pDesc->collision = true;
        pDesc->Add( faceNormal, intersection, 0.0f );
    }
}

void RaycastVsCollisionMesh(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    Raycast *pRay = (Raycast *) pObjectA;
    CollisionMesh *pMesh = (CollisionMesh *) pObjectB;

    const Transform *pWorldTransform = pMesh->GetTransform( );
    if ( *pWorldTransform == Math::IdentityTransform( ) ) pWorldTransform = NULL;

    Vector end = *pRay->GetEnd( );
    Vector start = *pRay->GetStart( );
    Vector direction = *pRay->GetDirection( );
    float length = pRay->GetLength( );

    static int *pIds = (int *) malloc( 4096 * 4 );
    int count = 0;

    pMesh->GetOverlappingTriangles( pIds, 4096, &count, start, direction, length );

    if ( count > 0 )
    {
        Vector localStart, localDirection;

        if ( pWorldTransform )
        {
            Math::TransformPosition( &localStart, start, *pMesh->GetInvTransform( ) );
            Math::Rotate( &localDirection, direction, *pMesh->GetInvTransform( ) );
        }
        else
        {
            localStart = start;
            localDirection = direction;
        }

        //sort so we can quickly rule out duplicates
        extern int QCompare( const void *pA, const void *pB );
        qsort( pIds, count, sizeof(int), QCompare );

        const Collider::Triangle *pTriangles = pMesh->GetTriangles( );

        const Collider::Triangle *pTriangle = &pTriangles[ pIds[0] ];

        //do first test
        RayVsTriangle( pTriangle, localStart, localDirection, start, direction, length, pWorldTransform, pDesc );

        for ( int i = 1; i < count; i++ )
        {
            //
            if ( pIds[i] != pIds[i - 1] ) 
            {
                const Collider::Triangle *pTriangle = &pTriangles[ pIds[i] ];
                RayVsTriangle( pTriangle, localStart, localDirection, start, direction, length, pWorldTransform, pDesc );
            }
        }
    }
}

RaycastCollisionHandler::RaycastCollisionHandler(
    RaycastResult *pResult
    )
{
    m_pResult = pResult;
    m_pResult->isValid = false;
    m_pResult->length  = FLT_MAX;
}

void RaycastCollisionHandler::Run(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB
    )
{
    CollisionDesc desc;

    bool collision = PhysicsWorld::Instance( ).TestCollision( pObjectA, pObjectB, &desc, false );

    if ( true == collision )
    {
        Raycast *pRaycast = (Raycast *) pObjectA;

        float length = Math::DotProduct( *pRaycast->GetDirection( ), desc.desc[0].intersection - *pRaycast->GetStart( ) );
        length = Math::Abs( length );

        //take closest result
        if ( length < m_pResult->length )
        {
            m_pResult->objectHit = pObjectB->GetComponent( )->GetId( );
            m_pResult->point     = desc.desc[0].intersection;
            m_pResult->normal    = desc.desc[0].normal;
            m_pResult->length    = length;
            m_pResult->isValid   = true;
        }
    }
}
