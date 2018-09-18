#include "EnginePch.h"

#include "RenderableShape.h"
#include "LightComponent.h"
#include "MaterialObject.h"
#include "Line.h"
#include "Viewport.h"
#include "Node.h"
#include "RenderWorld.h"

void RenderableShape::Create(
   Component *pComponent,
   ResourceHandle material,
   const IdList &renderGroups,
   const Triangle *pTriangles,
   int numTriangles,
   const Line *pLines,
   int numLines
)
{
   m_pComponent = pComponent;

   m_pMaterial = new GraphicsMaterialObject( material );

   m_NumTriangles = numTriangles;

   m_pTriangles = (Triangle *) malloc( sizeof( Triangle ) * m_NumTriangles );
   memcpy( m_pTriangles, pTriangles, sizeof( Triangle ) * m_NumTriangles );


   m_NumLines = numLines;

   m_pLines = (Line *) malloc( sizeof( Line ) * m_NumLines );
   memcpy( m_pLines, pLines, sizeof( Line ) * m_NumLines );

   m_IsFixedSize = false;
   m_FixedSize = 0.0f;

   m_RenderGroups.CopyFrom( renderGroups );
}

void RenderableShape::Destroy( void )
{
   delete m_pMaterial;

   free( m_pTriangles );
   free( m_pLines );
}

void RenderableShape::GetRenderData(
   RendererDesc *pDesc
) const
{
   if ( m_pMaterial->Prepare( ) )
   {
      int selected = m_pComponent->GetParent( )->IsSelected( );

      Transform transform = m_pComponent->GetParent( )->GetWorldTransform( );

      if ( m_NumTriangles > 0 )
      {
         RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

         pRODesc->pMaterial = m_pMaterial;
         pRODesc->pObject = this;
         pRODesc->argList = ArgList( transform, selected, m_NumTriangles, (int) ( m_IsFixedSize ? 1 : 0 ), m_FixedSize );
         pRODesc->renderFunc = RenderableShape::RenderTriangles;

         pRODesc->buffer.Write( m_pTriangles, sizeof( Triangle ) * m_NumTriangles );

         pDesc->renderObjectDescs.Add( pRODesc );
      }

      if ( m_NumLines > 0 )
      {
         RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

         pRODesc->pMaterial = m_pMaterial;
         pRODesc->pObject = this;
         pRODesc->argList = ArgList( transform, selected, m_NumLines, (int) ( m_IsFixedSize ? 1 : 0 ), m_FixedSize );
         pRODesc->renderFunc = RenderableShape::RenderLines;

         pRODesc->buffer.Write( m_pLines, sizeof( Line ) * m_NumLines );

         pDesc->renderObjectDescs.Add( pRODesc );
      }
   }
}

bool RenderableShape::IsVisible(
   const Frustum &frustum
) const
{
   return true;
}

void RenderableShape::GetRenderGroups(
   IdList *pList
) const
{
   pList->CopyFrom( m_RenderGroups );
}

void RenderableShape::RenderTriangles(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   Transform transform;
   int count, selected;
   int isFixedSize;
   float fixedSize;

   desc.pDesc->argList.GetArg( 0, &transform );
   desc.pDesc->argList.GetArg( 1, &selected );
   desc.pDesc->argList.GetArg( 2, &count );
   desc.pDesc->argList.GetArg( 3, &isFixedSize );
   desc.pDesc->argList.GetArg( 4, &fixedSize );

   if ( 0 != isFixedSize )
   {
      Transform world;
      Vector look, pos;

      const Camera *pCamera = desc.pViewport->GetCamera( );

      float d;

      if ( true == pCamera->IsPerspective( ) )
      {
         pCamera->GetWorldTransform( &world );

         world.GetTranslation( &pos );
         world.GetLook( &look );

         Vector shapePos;
         transform.GetTranslation( &shapePos );

         shapePos = shapePos - pos;

         d = Math::DotProduct( shapePos, look ) * fixedSize;

         Matrix invProj;
         pCamera->GetInvProjection( &invProj );

         //one pixel reverse projected
         float x = 1.0f / ( desc.pViewport->GetWidth( ) / 2 );

         Vector one( x, 0, 0, 1 );
         Math::TransformPosition( &one, one, invProj );

         d = d * one.x;
      }
      else
         d = fixedSize;

      transform.SetScale( Vector( d, d, d, 1.0f ) );
   }

   Triangle *pTriangles = (Triangle *) desc.pDesc->buffer.Read( 0 );
   Triangle::Render( pTriangles, count, 0 != selected, desc.pDesc->pMaterial, transform, desc, pStats );
}

void RenderableShape::RenderLines(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   Transform transform;
   int count, selected;

   int isFixedSize;
   float fixedSize;

   desc.pDesc->argList.GetArg( 0, &transform );
   desc.pDesc->argList.GetArg( 1, &selected );
   desc.pDesc->argList.GetArg( 2, &count );
   desc.pDesc->argList.GetArg( 3, &isFixedSize );
   desc.pDesc->argList.GetArg( 4, &fixedSize );

   if ( 0 != isFixedSize )
   {
      const Camera *pCamera = desc.pViewport->GetCamera( );

      float d;

      if ( true == pCamera->IsPerspective( ) )
      {
         Transform world;
         Vector look, pos;

         pCamera->GetWorldTransform( &world );

         world.GetTranslation( &pos );
         world.GetLook( &look );

         Vector shapePos;
         transform.GetTranslation( &shapePos );

         shapePos = shapePos - pos;

         d = Math::DotProduct( shapePos, look ) * fixedSize;

         Matrix invProj;
         pCamera->GetInvProjection( &invProj );

         //one pixel reverse projected
         float x = 1.0f / ( desc.pViewport->GetWidth( ) / 2 );

         Vector one( x, 0, 0, 1 );
         Math::TransformPosition( &one, one, invProj );

         d = d * one.x;
      }
      else
         d = fixedSize;

      transform.SetScale( Vector( d, d, d, 1.0f ) );
   }

   Line *pLines = (Line *) desc.pDesc->buffer.Read( 0 );

   Line::Render( pLines, count, 0 != selected, desc.pDesc->pMaterial, transform, desc, pStats );
}
