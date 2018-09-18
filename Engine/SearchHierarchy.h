#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "IRenderModifier.h"
#include "SystemId.h"
#include "Renderer.h"

class FrustumCullRenderModifier : public IRenderModifier
{
private:
   RenderObjectDescList m_Renderables;
   LightDescList        m_Lights;

   uint32 m_TotalRenderables;
   uint32 m_TotalLights;
   uint32 m_RenderablesCulled;
   uint32 m_LightsCulled;

public:
   FrustumCullRenderModifier( void )
   {
      m_Renderables.Create( );
      m_Lights.Create( );
   }

   ~FrustumCullRenderModifier( void )
   {
      m_Renderables.Destroy( );
      m_Lights.Destroy( );
   }

   virtual void Begin( void );

   virtual void Process(
      RendererDesc *pRendererDesc
      );

   uint32 TotalRenderables( void ) const  { return m_TotalRenderables; }
   uint32 TotalLights( void ) const  { return m_TotalLights; }
   uint32 RenderablesCulled( void ) const { return m_RenderablesCulled; }
   uint32 LightsCulled( void ) const { return m_LightsCulled; }
};

class TransparentSortRenderModifier : public IRenderModifier
{
private:
   struct ZOrder
   {
      RenderObjectDesc *pObject;
      float z;
   };

private:
   RenderObjectDescList m_Renderables;
   ZOrder *m_pZOrders;
   uint32  m_ZOldCount;

public:
   TransparentSortRenderModifier( void )
   {
      m_Renderables.Create( );

      m_pZOrders = NULL;
      m_ZOldCount = 0;
   }

   ~TransparentSortRenderModifier( void )
   {
      m_Renderables.Destroy( );

      free( m_pZOrders );
   }

   virtual void Begin( void );

   virtual void Process(
      RendererDesc *pRendererDesc
      );

private:
   static int ZCompare( const void *pA, const void *pB );
};
