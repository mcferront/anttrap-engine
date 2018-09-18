#pragma once

#include "EngineGlobal.h"
#include "RenderObject.h"

class Line;

class RenderableShape : public RenderObject
{
private:
   IdList      m_RenderGroups;
   GraphicsMaterialObject *m_pMaterial;
   Component *m_pComponent;
   Triangle  *m_pTriangles;
   Line      *m_pLines;

   int m_NumTriangles;
   int m_NumLines;
   bool  m_IsFixedSize;
   float m_FixedSize;

public:
   void Create(
      Component *pComponent,
      ResourceHandle material,
      const IdList &renderGroups,
      const Triangle *pTriangles,
      int numTriangles,
      const Line *pLines,
      int numLines
   );

   void Destroy( void );

   virtual void GetRenderData(
      RendererDesc *pDesc
   ) const;

   bool IsVisible(
      const Frustum &frustum
   ) const;

   virtual void GetRenderGroups( IdList *pGroups ) const;

   void SetFixedSize(
      bool isFixedSize,
      float fixedSize
   )
   {
      m_IsFixedSize = isFixedSize;
      m_FixedSize = fixedSize;
   }

   virtual void GetWorldTransform(
      Transform *pTransform
   ) const
   {
      *pTransform = m_pComponent->GetParent( )->GetWorldTransform( );
   }

   virtual int GetRenderType( void ) const { return RenderObject::Type::Mesh; }

private:

   static void RenderTriangles(
      const RenderDesc &desc,
      RenderStats *pStats
   );

   static void RenderLines(
      const RenderDesc &desc,
      RenderStats *pStats
   );
};
