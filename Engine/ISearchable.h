#pragma once

#include "EngineGlobal.h"

struct Frustum
{
   Vector leftPlane, rightPlane;
   Vector bottomPlane, topPlane;
   Vector nearPlane, farPlane;
};

class ISearchable
{
public:
   virtual bool IsVisible( 
      const Frustum &frustum
   ) const = 0;

   virtual void GetWorldTransform(
      Transform *pWorldTransform
   ) const = 0;
};

bool BoxInFrustum(
   const Vector &localCenter,
   const Vector &half,
   const Transform &worldTransform,
   const Frustum &frustum
);

bool SphereInFrustum(
   const Vector &localCenter,
   const float radius,
   const Frustum &frustum
);

bool SphereInBox(
   const Vector &boxCenter,
   const Vector &boxExtents,
   const Transform &invBoxTransform,
   const Vector &sphereCenter,
   float radius
);
