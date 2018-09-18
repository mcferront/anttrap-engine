#include "EnginePch.h"

#include "CollisionMesh.h"
#include "PhysicsWorld.h"
#include "DebugGraphics.h"
#include "CollisionSphere.h"
#include "ColliderAsset.h"
#include "MeshAsset.h"

void CollisionSphereVsCollisionMesh(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void CollisionMeshVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    );

void CollisionMeshStub(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    ) { }

void CollisionMesh::Create(
    Component *pComponent,
    float restitution,
    float friction,
    ResourceHandle mesh
    )
{
    static bool registered;

    if ( false == registered )
    {
        registered = true;

        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeMesh, CollisionTypeSphere, CollisionMeshVsCollisionSphere );
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeSphere, CollisionTypeMesh, CollisionSphereVsCollisionMesh );
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypePlane, CollisionTypeMesh, CollisionMeshStub );
        PhysicsWorld::Instance( ).RegisterCollisionFunction( CollisionTypeMesh, CollisionTypePlane, CollisionMeshStub );
    }

    CollisionObject::Create( pComponent );

    m_Mesh = mesh;

    memset( &m_SurfaceDesc, 0, sizeof(m_SurfaceDesc) );
    
    m_SurfaceDesc.spring = restitution;
    m_SurfaceDesc.friction = friction;

    SetTransform( Math::IdentityTransform() );
}

void CollisionMesh::Destroy( void )
{
    m_Mesh = NullHandle;
}

void CollisionMesh::Render( 
    const Vector &color 
    )
{
    if ( false == IsResourceLoaded(m_Mesh) ) return;

    //const Collider *pCollider = GetResource( m_Mesh, Collider);

    //const Vector blue(0, 0, 1, 1);

    //////render mesh
    ////{
    ////    int count = pCollider->GetNumTriangles( );
    ////    const Collider::Triangle *pTriangles = pCollider->GetTriangles( );

    ////    Vector start;

    ////    float normalSize = Math::Magnitude( pTriangles[ 0 ].positions[ 0 ] - pTriangles[ 0 ].positions[ 1 ] );

    ////    for ( int i = 0; i < count; i++ )
    ////    {
    ////        DebugGraphics::Instance( ).RenderLine( m_Transform, pTriangles[ i ].positions[ 0 ], pTriangles[ i ].positions[ 1 ], color );
    ////        DebugGraphics::Instance( ).RenderLine( m_Transform, pTriangles[ i ].positions[ 1 ], pTriangles[ i ].positions[ 2 ], color );
    ////        DebugGraphics::Instance( ).RenderLine( m_Transform, pTriangles[ i ].positions[ 2 ], pTriangles[ i ].positions[ 0 ], color );

    ////        Math::TransformPosition( &start, pTriangles[ i ].positions[ 0 ], m_Transform );
    ////        DebugGraphics::Instance( ).RenderArrow( start, pTriangles[ i ].normal, color, normalSize );

    ////        Math::TransformPosition( &start, pTriangles[ i ].positions[ 0 ], m_Transform );
    ////        DebugGraphics::Instance( ).RenderArrow( start, pTriangles[ i ].normals[ 0 ], blue, normalSize / 8 );

    ////        Math::TransformPosition( &start, pTriangles[ i ].positions[ 1 ], m_Transform );
    ////        DebugGraphics::Instance( ).RenderArrow( start, pTriangles[ i ].normals[ 1 ], blue, normalSize / 8 );

    ////        Math::TransformPosition( &start, pTriangles[ i ].positions[ 2 ], m_Transform );
    ////        DebugGraphics::Instance( ).RenderArrow( start, pTriangles[ i ].normals[ 2 ], blue, normalSize / 8 );
    ////    }
    ////}

    ////render bounding box
    //{
    //    Vector center  = *pCollider->GetCenter( );
    //    Vector extents = *pCollider->GetExtents( );

    //    Vector s = center - extents;
    //    Vector e = s;

    //    e.x += extents.x * 2;
    //    DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, blue );

    //    s = e;
    //    e.z += extents.z * 2;
    //    DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, blue );

    //    s = e;
    //    e.x = center.x - extents.x;
    //    DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, blue );

    //    s = e;
    //    e = center - extents;
    //    DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, blue );
    //}

    ////render grid
    //{
    //    int cellsX = pCollider->GetGridCellsX( );
    //    int cellsZ = pCollider->GetGridCellsZ( );

    //    Vector cell  = *pCollider->GetGridCellDimensions( );
    //    Vector start = *pCollider->GetGridStart( );

    //    for ( int z = 0; z < cellsZ; z++ )
    //    {
    //        Vector origin = start;
    //        origin.z += z * cell.z;

    //        for ( int x = 0; x < cellsX; x++ )
    //        {
    //            Vector s = origin;
    //            Vector e = origin;

    //            e.x += cell.x;
    //            DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //            s = e;
    //            e.z += cell.z;
    //            DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //            s = e;
    //            e.x = origin.x;
    //            DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //            s = e;
    //            e = origin;
    //            DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //            origin.x += cell.x;
    //        }
    //    }
    //}
}

void CollisionMesh::GetOverlappingTriangles(
    int *pIds,
    int size,
    int *pReturned,
    const Vector &position,
    float radius
    ) const
{
    //if ( false == IsResourceLoaded(m_Mesh) )
    //{
    //    *pReturned = 0;
    //    return;
    //}

    //const Collider *pCollider = GetResource( m_Mesh, Mesh )->GetCollider( );

    //Vector localPosition;
    //Math::TransformPosition( &localPosition, position, m_InvTransform );

    //Vector cell  = *pCollider->GetGridCellDimensions( );
    //Vector start = *pCollider->GetGridStart( );

    //localPosition = localPosition - start;

    ////TODO: (future)
    //// use 3d grid instead of 2d

    //int cellsX = pCollider->GetGridCellsX( );
    //int cellsZ = pCollider->GetGridCellsZ( );

    //int cellCoverageX = (int) ceilf(radius / cell.x);
    //int cellCoverageZ = (int) ceilf(radius / cell.z);

    //int cx = (int) floorf(localPosition.x / cell.x);
    //int cz = (int) floorf(localPosition.z / cell.z);

    //int trianglesReturned = 0;

    //int sz = cz - cellCoverageZ;
    //if ( sz < 0 ) sz = 0;

    //int ez = cz + cellCoverageZ;
    //if ( ez >= cellsZ - 1 ) ez = cellsZ - 1;

    //int sx = cx - cellCoverageX;
    //if ( sx < 0 ) sx = 0;

    //int ex = cx + cellCoverageX;
    //if ( ex >= cellsX - 1 ) ex = cellsX - 1;

    //for ( int z = sz; z <= ez; z++ )
    //{
    //    //Vector color(0,0,1,1);

    //    //Vector origin = start;
    //    //origin.z += z * cell.z;

    //    for ( int x = sx; x <= ex; x++ )
    //    {
    //        //origin.x = start.x + x * cell.x;

    //        //Vector s = origin;
    //        //Vector e = origin;

    //        //e.x += cell.x;
    //        //DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //        //s = e;
    //        //e.z += cell.z;
    //        //DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //        //s = e;
    //        //e.x = origin.x;
    //        //DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //        //s = e;
    //        //e = origin;
    //        //DebugGraphics::Instance( ).RenderLine( m_Transform, s, e, color );

    //        int numForThisCell;
    //        pCollider->GetCellTriangles( x, z, pIds + trianglesReturned, size - trianglesReturned, &numForThisCell );

    //        trianglesReturned += numForThisCell;
    //    }
    //}

    ////look up in grid
    ////check cell and 8 surrounding cells
    //*pReturned = trianglesReturned;
}

void CollisionMesh::GetOverlappingTriangles(
    int *pIds,
    int size,
    int *pReturned,
    const Vector &position,
    const Vector &direction,
    float length
    ) const
{
    //if ( false == IsResourceLoaded(m_Mesh) )
    //{
    //    *pReturned = 0;
    //    return;
    //}

    //const Collider *pCollider = GetResource( m_Mesh, Mesh )->GetCollider( );

    //Vector localStart, localDirection;

    //Math::TransformPosition( &localStart, position, m_InvTransform );

    //Math::Rotate( &localDirection, direction, m_InvTransform );

    //Vector cell  = *pCollider->GetGridCellDimensions( );
    //Vector start = *pCollider->GetGridStart( );

    //Vector gridLocalStart = localStart - start;

    ////starting cell
    //float sx = floorf(gridLocalStart.x / cell.x);
    //float sz = floorf(gridLocalStart.z / cell.z);

    ////ending cell
    //float ex = floorf((gridLocalStart.x + localDirection.x * length) / cell.x);
    //float ez = floorf((gridLocalStart.z + localDirection.z * length) / cell.z);

    //int startX, endX, startZ, endZ;

    ////Simply search the grid in a square pattern from
    ////Ray Start X, Z to ray end X, Z
    ////This is the only way I can find to guarantee we check
    ////every cell the ray crosses
    ////TODO: Research grid walking algorithms for a faster implementation

    //if ( sx < ex )
    //{
    //    startX = (int) sx;
    //    endX = (int) ex;
    //}
    //else
    //{
    //    startX = (int) ex;
    //    endX = (int) sx;
    //}

    //if ( sz < ez )
    //{
    //    startZ = (int) sz;
    //    endZ = (int) ez;
    //}
    //else
    //{
    //    startZ = (int) ez;
    //    endZ = (int) sz;
    //}

    //startX -= 1;
    //startZ -= 1;

    //endX += 1;
    //endZ += 1;

    ////clamp start/end values to grid here
    //startX = Math::Max( startX, 0 );
    //startZ = Math::Max( startZ, 0 );

    //endX = Math::Min( endX, pCollider->GetGridCellsX( ) - 1 );
    //endZ = Math::Min( endZ, pCollider->GetGridCellsZ( ) - 1 );

    //int trianglesReturned = 0;

    //for ( int z = startZ; z <= endZ; z++ )
    //{
    //    for ( int x = startX; x <= endX; x++ )
    //    {
    //        int numForThisCell;
    //        pCollider->GetCellTriangles( x, z, pIds + trianglesReturned, size - trianglesReturned, &numForThisCell );

    //        trianglesReturned += numForThisCell;
    //    }
    //}
    //
    ////for ( int z = 0; z < pCollider->GetGridCellsZ(); z++ )
    ////{
    ////    for ( int x = 0; x < pCollider->GetGridCellsX(); x++ )
    ////    {
    ////        int numForThisCell;
    ////        pCollider->GetCellTriangles( x, z, pIds + trianglesReturned, size - trianglesReturned, &numForThisCell );

    ////        trianglesReturned += numForThisCell;
    ////    }
    ////}

    //*pReturned = trianglesReturned;
}

const Collider::Triangle *CollisionMesh::GetTriangles( void ) const
{
    return NULL;//GetResource( m_Mesh, Mesh)->GetCollider( )->GetTriangles( );
}

void CollisionSphereVsCollisionMesh(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    CollisionMeshVsCollisionSphere( pObjectB, pObjectA, pDesc );
    pDesc->Swap( );
}

int QCompare( const void *pA, const void *pB )
{
    int a = *(int *) pA;
    int b = *(int *) pB;

    return a - b;
}

void TriangleVsSphere(
    const Collider::Triangle *pTriangle,
    const Vector &sphere,
    float radius,
    const Transform *pWorldTransform,
    CollisionDesc *pDesc
    )
{
   float distance;

   distance = Math::DotProduct( sphere - pTriangle->positions[0], pTriangle->normal );
   if ( Math::Abs(distance) > radius ) return;

   float d[3];

   //close enough, now is it within the edges?
   d[0] = Math::DotProduct( sphere - pTriangle->positions[0], pTriangle->normals[0] );
   if ( d[0] > radius ) return;

   d[1] = Math::DotProduct( sphere - pTriangle->positions[1], pTriangle->normals[1] );
   if ( d[1] > radius ) return;

   d[2] = Math::DotProduct( sphere - pTriangle->positions[2], pTriangle->normals[2] );
   if ( d[2] > radius ) return;

   //we have an interection, calculate where
   Vector intersection;
   float penetration; 
   
   //outside of an edge
   int i;

   penetration = radius - Math::Abs(distance); 

   for ( i = 0; i < 3; i++ )
   {
      if ( d[i] <= 0 ) continue;

      float distOnEdge;

      //within range of edge i

      //where does the point lie on the edge
      distOnEdge = Math::DotProduct( sphere - pTriangle->positions[i], pTriangle->edges[i] );

      //if it's within the two edge corners
      if ( distOnEdge >= - radius || distOnEdge < pTriangle->edges[i].w + radius ) 
      {
         //now that we know we collide, clamp the point to
         //within the edge points
         distOnEdge  = Math::Clamp( distOnEdge, 0.0f, pTriangle->edges[i].w );
         
         //exact collision point of the sphere
         intersection = pTriangle->positions[i] + pTriangle->edges[i] * distOnEdge;

         if ( penetration > 0 )
         {
            //decrease penetration by the curvature of the sphere
            //along the triangle's edge
            Vector direction;
            Math::Normalize( &direction, intersection - sphere );

            penetration *= Math::DotProduct( direction, - pTriangle->normal );
         }

         break;
      }
   }
  
   //we were within all sides
   if ( 3 == i )
   {
      intersection = sphere + - pTriangle->normal * (radius - penetration);
   }

   Vector normal = - pTriangle->normal;

   if ( pWorldTransform )
   {
      Math::Rotate( &normal, normal, *pWorldTransform ); 
      Math::TransformPosition( &intersection, intersection, *pWorldTransform ); 
   }
   
   pDesc->Add( normal, intersection, penetration );
}

void CollisionMeshVsCollisionSphere(
    const CollisionObject *pObjectA,
    const CollisionObject *pObjectB,
    CollisionDesc *pDesc
    )
{
    //TODO: (future)
    //Decide what type of data structure the CollisionMesh holds
    //and relay to the appropriate call, example:
    //if (pMesh->GetCollisionGrid()) CollisionGridVsCollisionSphere
    //if (pMesh->ConvexHull()) ConvexHullVsCollisionSphere

    CollisionMesh *pMesh = (CollisionMesh *) pObjectA;
    CollisionSphere *pSphere = (CollisionSphere *) pObjectB;

    const Transform *pWorldTransform = pMesh->GetTransform( );
    if ( *pWorldTransform == Math::IdentityTransform() ) pWorldTransform = NULL;

    Vector sphere = *pSphere->GetPosition( );
    float radius  = pSphere->GetRadius();

    static int *pIds = (int *) malloc( 2048 * 4 );
    int count = 0;

    pMesh->GetOverlappingTriangles( pIds, 2048, &count, sphere, radius );

    if ( count > 0 )
    {
        if ( pWorldTransform )
            Math::TransformPosition( &sphere, sphere, *pMesh->GetInvTransform( ) );

        //sort so we can quickly rule out duplicates
        qsort( pIds, count, sizeof(int), QCompare );

        const Collider::Triangle *pTriangles = pMesh->GetTriangles( );

        const Collider::Triangle *pTriangle = &pTriangles[ pIds[0] ];

        //do first test
        TriangleVsSphere( pTriangle, sphere, radius, pWorldTransform, pDesc );

        for ( int i = 1; i < count; i++ )
        {
            //
            if ( pIds[i] != pIds[i - 1] ) 
            {
                const Collider::Triangle *pTriangle = &pTriangles[ pIds[i] ];
                TriangleVsSphere( pTriangle, sphere, radius, pWorldTransform, pDesc );
            }
        }

        if ( pDesc->count > 0 )
        {
            pMesh->GetTransform( )->GetTranslation( &pDesc->surfaceA.position );
            pSphere->GetPosition( &pDesc->surfaceB.position );

            pDesc->pObjectA = pMesh;
            pDesc->pObjectB = pSphere;

            pDesc->surfaceA.linear  = *pMesh->GetLinearVelocity( );
            pDesc->surfaceA.angular = *pMesh->GetAngularVelocity( );
            pDesc->surfaceA.spring  =  pMesh->GetSpring( );
            pDesc->surfaceA.friction=  pMesh->GetFriction( );

            pDesc->surfaceB.linear  = *pSphere->GetLinearVelocity( );
            pDesc->surfaceB.angular = *pSphere->GetAngularVelocity( );
            pDesc->surfaceB.spring  =  pSphere->GetSpring( );
            pDesc->surfaceB.friction=  pSphere->GetFriction( );

            pDesc->collision = true;
        }
    }
}

