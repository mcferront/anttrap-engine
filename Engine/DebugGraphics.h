#pragma once

#include "EngineGlobal.h"
#include "RenderObject.h"
#include "TextArea.h"
#include "MaterialAsset.h"

class Line;

class DebugGraphics : public RenderObject
{
protected:
   struct Sphere
   {
      Transform transform;
      Vector color;
      GraphicsMaterialObject *pMaterial;
      float radius;
   };

   struct Plane
   {
      Vector plane;
      Vector color;
   };

public:
   static DebugGraphics &Instance( void );

protected:
   //TextArea       m_TextArea;
   List<Line>     m_Lines;
   List<Triangle> m_Triangles;
   List<Sphere>   m_Spheres;
   List<Plane>    m_Planes;

   GraphicsMaterialObject *m_pLinesMaterial;
   GraphicsMaterialObject *m_pTrianglesMaterial;

   List<GraphicsMaterialObject *> m_MaterialObjectPool;
   List<GraphicsMaterialObject *> m_UsedMaterialObjects;

public:
   DebugGraphics( void );

   void Create( void );

   void Destroy( void );

   void RenderArrow(
      const Vector &origin,
      const Vector &direction,
      const Vector &color,
      float size
   );

   void RenderTransform(
      const Transform &transform,
      float size
   );

   void RenderLine(
      const Transform &transform,
      const Vector &start,
      const Vector &end,
      const Vector &color
   );

   void RenderLine(
      const Vector &start,
      const Vector &end,
      const Vector &color
   );

   void RenderTriangle(
      const Vector &v1,
      const Vector &v2,
      const Vector &v3,
      const Vector &color
   );

   void RenderTriangle(
      const Transform &transform,
      const Vector &v1,
      const Vector &v2,
      const Vector &v3,
      const Vector &color
   );

   void RenderCircle(
      const Vector2 &center,
      float radius,
      const Vector &color
   );

   void RenderSphere(
      const Transform &transform,
      float radius,
      const Vector &color
   );

   void RenderPlane(
      const Vector &plane,
      const Vector &color
   );

   void RenderBox(
      const Transform &transform,
      const Vector2 &center,
      const Vector2 &half,
      const Vector &color
   );

   void RenderCube(
      const Transform &transform,
      const Vector &center,
      const Vector &half,
      const Vector &color
   );

   void RenderFrustum(
      const Frustum &frustum,
      const Vector &color
   );

   void Print(
      const char *pText,
      ...
   );

   void Clear( void );

   virtual bool NeedsFlush( void ) const { return true; }

   virtual void Flush( void ) { FreeMaterialObjects( ); }

   virtual void AddToScene( void );

   virtual void RemoveFromScene( void );

   bool IsVisible(
      const Frustum &frustum
   ) const
   {
      return true;
   }

   virtual void GetRenderData(
      RendererDesc *pDesc
   ) const;

   virtual void GetRenderGroups(
      IdList *pGroups
   ) const
   {
      static const char *pRef = StringRef( "DebugGraphics.material" );
      ResourceHandle material = ResourceHandle::FromAlias( pRef );

      if ( true == IsResourceLoaded( material ) )
         pGroups->Add( Id( "DebugGraphics" ) );
   }

   virtual void GetWorldTransform(
      Transform *pWorldTransform
   ) const
   {
      *pWorldTransform = Math::IdentityTransform( );
   }

   virtual int GetRenderType( void ) const { return RenderObject::Type::Mesh; }

private:
   GraphicsMaterialObject *GetMaterialObject( void );
   void FreeMaterialObjects( void );

private:

   static void RenderLines(
      const RenderDesc &desc,
      RenderStats *pStats
   );

   static void RenderTriangles(
      const RenderDesc &desc,
      RenderStats *pStats
   );

   static void RenderSphere(
      const RenderDesc &desc,
      RenderStats *pStats
   );

   static void RenderPlanes(
      const RenderDesc &desc,
      RenderStats *pStats
   );
};
