#include "EnginePch.h"

#include "SearchHierarchy.h"
#include "Viewport.h"
#include "Renderer.h"
#include "RenderObject.h"
#include "Node.h"
#include "Light.h"
#include "MaterialObject.h"

void FrustumCullRenderModifier::Begin( void )
{
   m_TotalRenderables = 0;
   m_TotalLights = 0;
   m_RenderablesCulled = 0;
   m_LightsCulled = 0;
}

void FrustumCullRenderModifier::Process(
   RendererDesc *pRendererDesc
   )
{
   Frustum frustum;
   pRendererDesc->viewport.GetCamera( )->GetFrustum( &frustum );

   m_Renderables.Clear( );
   m_Renderables.CopyFrom( pRendererDesc->renderObjectDescs );

   m_Lights.Clear( );
   m_Lights.CopyFrom( pRendererDesc->lightDescs );

   m_TotalRenderables += m_Renderables.GetSize( );
   m_TotalLights += m_Lights.GetSize( );

   pRendererDesc->renderObjectDescs.Clear( );
   pRendererDesc->lightDescs.Clear( );

   for ( uint32 i = 0; i < m_Renderables.GetSize( ); i++ )
   {
      RenderObjectDesc *pDesc = m_Renderables.GetAt( i );

      if ( true == m_Renderables.GetAt( i )->pObject->IsVisible(frustum) )
         pRendererDesc->renderObjectDescs.Add( pDesc );
      else
         ++m_RenderablesCulled;
   }

   for ( uint32 i = 0; i < m_Lights.GetSize( ); i++ )
   {
      LightDesc *pDesc = m_Lights.GetPointer( i );

      if ( true == pDesc->pLight->IsVisible(frustum) )
         pRendererDesc->lightDescs.Add( *pDesc );
      else
         ++m_LightsCulled;
   }
}

void TransparentSortRenderModifier::Begin( void )
{
}

void TransparentSortRenderModifier::Process(
   RendererDesc *pRendererDesc
   )
{
   m_Renderables.Clear( );
   m_Renderables.CopyFrom( pRendererDesc->renderObjectDescs );

   pRendererDesc->renderObjectDescs.Clear( );

   uint32 i;

   for ( i = 0; i < m_Renderables.GetSize( ); i++ )
   {
      RenderObjectDesc *pDesc = m_Renderables.GetAt( i );

      if ( false == pDesc->pMaterial->HasAlpha(pRendererDesc->pPass) )
         pRendererDesc->renderObjectDescs.Add( pDesc );
   }

   uint32 start = pRendererDesc->renderObjectDescs.GetSize( );

   uint32 zNewCount = m_Renderables.GetSize( ) - start;

   if ( zNewCount > m_ZOldCount )
   {
      m_ZOldCount = zNewCount;
      m_pZOrders = (ZOrder *) realloc( m_pZOrders, zNewCount * sizeof( ZOrder ) );
   }

   Vector look = pRendererDesc->viewport.GetCamera( )->GetWorldTransform( )->GetLook( );
   Vector cam = pRendererDesc->viewport.GetCamera( )->GetWorldTransform( )->GetTranslation( );

   int index = 0;
   Transform worldTransform;

   //Gather up pivot distance from camera
   for ( i = 0; i < m_Renderables.GetSize( ); i++ )
   {
      RenderObjectDesc *pDesc = pRendererDesc->renderObjectDescs.GetAt( i );

      if ( true == pDesc->pMaterial->HasAlpha(pRendererDesc->pPass) )
      {
         pDesc->pObject->GetWorldTransform( &worldTransform );

         Vector p = worldTransform.GetTranslation( );
         float z = Math::DotProduct( look, p - cam );

         ZOrder zOrder = { m_Renderables.GetAt( i ), z };
         m_pZOrders[ index++ ] = zOrder;
      }
   }

   //Sort by pivot distance from camera
   qsort( m_pZOrders, zNewCount, sizeof( ZOrder ), ZCompare );

   //Add sorted back to the end of the render list
   for ( i = 0; i < zNewCount; i++ )
      pRendererDesc->renderObjectDescs.Add( m_pZOrders[ i ].pObject );
}

int TransparentSortRenderModifier::ZCompare( const void *pA, const void *pB )
{
   float d = ( (ZOrder *) pA )->z - ( (ZOrder *) pB )->z;

   if ( d < 0 ) return 1;
   if ( d > 0 ) return -1;

   return 0;
}
