#include "EnginePch.h"

#include "ISearchable.h"

bool BoxAbovePlane(
   const Vector &plane,
   const Matrix &rotateWorldToLocal,
   const Vector &worldCenter,
   const Vector &half
)
{
   Vector localPlane;
   Math::Rotate( &localPlane, plane, rotateWorldToLocal );

   //keep the distance
   localPlane.w = plane.w;
   
   Vector closestAxisToPlane = localPlane * half;
   Math::Abs( &closestAxisToPlane, closestAxisToPlane );

   //r == farthest distance (based on the extents) to the plane (extents projected on the plane normal)
   //b == distance from the box center to the plane
   float r = closestAxisToPlane.x + closestAxisToPlane.y + closestAxisToPlane.z;
   float b = Math::DotProduct4( plane, worldCenter );

   bool result = ( b >= -r );

   return result;
}

bool SphereAbovePlane(
   const Vector &plane,
   const Vector &worldCenter,
   const float radius
)
{
   float b = Math::DotProduct4( plane, worldCenter );

   bool result = b >= - radius;
   
   return result;
}

bool BoxInFrustum(
   const Vector &localCenter,
   const Vector &half,
   const Transform &worldTransform,
   const Frustum &frustum
)
{
   Vector center;
   Math::TransformPosition( &center, localCenter, worldTransform );

   Matrix worldToLocal;

   //To transform a plane into world space you multiply it by the
   //inverse transpose of the world matrix.  However we want to go into local space
   //which means we'd invert it first and then do invert/transpose
   //the two inverts cancel each other out
   Math::Transpose( &worldToLocal, worldTransform.ToMatrix(true) );

   bool result;   
   
   result = BoxAbovePlane(frustum.leftPlane, worldToLocal, center, half );
   if ( true == result ) {} else return false;

   result = BoxAbovePlane(frustum.rightPlane, worldToLocal, center, half );
   if ( true == result ) {} else return false;

   result = BoxAbovePlane(frustum.topPlane, worldToLocal, center, half );
   if ( true == result ) {} else return false;

   result = BoxAbovePlane(frustum.bottomPlane, worldToLocal, center, half );
   if ( true == result ) {} else return false;

   result = BoxAbovePlane(frustum.nearPlane, worldToLocal, center, half );
   if ( true == result ) {} else return false;

   result = BoxAbovePlane(frustum.farPlane, worldToLocal, center, half );
   if ( true == result ) {} else return false;

   return true;
}

bool SphereInFrustum(
   const Vector &center,
   const float radius,
   const Frustum &frustum
)
{
   bool result;   
   
   result = SphereAbovePlane( frustum.leftPlane, center, radius );
   if ( true == result ) {} else return false;

   result = SphereAbovePlane( frustum.rightPlane, center, radius );
   if ( true == result ) {} else return false;

   result = SphereAbovePlane( frustum.topPlane, center, radius );
   if ( true == result ) {} else return false;

   result = SphereAbovePlane( frustum.bottomPlane, center, radius );
   if ( true == result ) {} else return false;

   result = SphereAbovePlane( frustum.nearPlane, center, radius );
   if ( true == result ) {} else return false;

   result = SphereAbovePlane( frustum.farPlane, center, radius );
   if ( true == result ) {} else return false;

   return true;
}

bool SphereInBox(
   const Vector &boxCenter,
   const Vector &boxExtents,
   const Transform &invBoxTransform,
   const Vector &sphereCenter,
   float radius
)
{
   Vector sphereLocal;
   Math::TransformPosition( &sphereLocal, sphereCenter, invBoxTransform );

   Vector plane;
   bool result;

   plane = Vector( 0, -1, 0, boxExtents.y );
   result = SphereAbovePlane( plane, sphereCenter, radius );
   if ( true == result ) {} else return false;

   plane = Vector( 0,  1, 0, boxExtents.y );
   result = SphereAbovePlane( plane, sphereCenter, radius );
   if ( true == result ) {} else return false;

   plane = Vector( -1, 0, 0, boxExtents.x );
   result = SphereAbovePlane( plane, sphereCenter, radius );
   if ( true == result ) {} else return false;

   plane = Vector( 1, 0, 0, boxExtents.x );
   result = SphereAbovePlane( plane, sphereCenter, radius );
   if ( true == result ) {} else return false;

   plane = Vector( 0, 0, 1, boxExtents.z );
   result = SphereAbovePlane( plane, sphereCenter, radius );
   if ( true == result ) {} else return false;

   plane = Vector( 0, 0, -1, boxExtents.z );
   result = SphereAbovePlane( plane, sphereCenter, radius );
   if ( true == result ) {} else return false;

   return true;

}
