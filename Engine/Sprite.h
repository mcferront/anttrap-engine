#pragma once

#include "EngineGlobal.h"
#include "RenderObject.h"
#include "ResourceMaps.h"
#include "ResourceWorld.h"
#include "Quad.h"

class Viewport;

class Sprite : public RenderObject
{
private:
   Vector    m_Color;
   Vector2   m_Size;
   Quad      m_Quad;
   IdList    m_RenderGroups;

   GraphicsMaterialObject *m_pMaterial;
   Component      *m_pComponent;

public:
   void Create(
      Component *pComponent,
      ResourceHandle material,
      const IdList &renderGroups,
      const Vector2 &size
   );

   void Destroy( void );

   virtual void GetRenderData(
      RendererDesc *pDesc
   ) const;

   void SetSize(
      const Vector2 &size
   );

   bool IsVisible(
      const Frustum &frustum
   ) const;

   void SetMaterial(
      ResourceHandle material
   );

   void SetColor( const Vector &color ) { m_Color = color; }
   const Vector &GetColor( void ) const { return m_Color; }

   void SetAlpha( float alpha ) { m_Color.w = alpha; }
   float GetAlpha( void ) const { return m_Color.w; }

   //ResourceHandle GetMaterial( void ) const { return m_Material; }   

   const Vector2 *GetSize( void ) const { return &m_Size; }

   virtual int GetRenderType( void ) const { return RenderObject::Type::Sprite; }

   virtual void GetRenderGroups(
      IdList *pGroups
   ) const;

   virtual void GetWorldTransform(
      Transform *pWorldTransform
   ) const
   {
      m_pComponent->GetParent( )->GetWorldTransform( pWorldTransform );
   }

private:
   void MakeFromMaterial( void );

protected:
   static void Render(
      const RenderDesc &desc,
      RenderStats *pStats
   );
};
