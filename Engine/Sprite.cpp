#include "EnginePch.h"

#include "Sprite.h"
#include "RenderWorld.h"
#include "ResourceWorld.h"
#include "MaterialObject.h"
#include "Viewport.h"
#include "Node.h"

void Sprite::Create(
   Component *pComponent,
   ResourceHandle material,
   const IdList &renderGroups,
   const Vector2 &size
)
{
   SetColor( Vector( 1, 1, 1, 1 ) );

   m_pComponent = pComponent;
   m_pMaterial = new GraphicsMaterialObject( material );
   m_Size = size;
   m_RenderGroups = renderGroups;

   MakeFromMaterial( );
}

void Sprite::Destroy( void )
{
   delete m_pMaterial;
}

void Sprite::GetRenderData(
   RendererDesc *pDesc
) const
{
   int selected = m_pComponent->GetParent( )->IsSelected( );

   if ( m_pMaterial->Prepare( ) )
   {
      RenderObjectDesc *pRODesc = RenderWorld::Instance( ).AllocRenderObjectDesc( );

      pRODesc->renderContext = m_pMaterial->GetRenderContext( pDesc->pPass, pDesc->viewport.GetContext(), Quad::GetVertexContext( ) );
      pRODesc->pMaterial = m_pMaterial;
      pRODesc->argList = ArgList( m_Color, m_pComponent->GetParent( )->GetWorldTransform( ), selected );
      pRODesc->renderFunc = Sprite::Render;
      pRODesc->pObject = this;

      pRODesc->buffer.Write( &m_Quad, sizeof( Quad ) );

      pDesc->renderObjectDescs.Add( pRODesc );
   }
}

void Sprite::SetSize(
   const Vector2 &size
)
{
   m_Size = size;
   MakeFromMaterial( );
}

bool Sprite::IsVisible(
   const Frustum &frustum
) const
{
   return BoxInFrustum( Math::ZeroVector( ), m_Size * .5f, m_pComponent->GetParent( )->GetWorldTransform( ), frustum );
}

void Sprite::SetMaterial(
   ResourceHandle material
)
{
   delete m_pMaterial;

   m_pMaterial = new GraphicsMaterialObject( material );
   MakeFromMaterial( );
}

void Sprite::GetRenderGroups(
   IdList *pGroups
) const
{
   pGroups->CopyFrom( m_RenderGroups );
}

void Sprite::MakeFromMaterial( void )
{
   Vector2 matSize = GetMaterialSize( m_pMaterial );

   if ( 0 == m_Size.x )
      m_Size.x = matSize.x;
   if ( 0 == m_Size.y )
      m_Size.y = matSize.y;

   Vector uv = GetMaterialUV( m_pMaterial );
   m_Quad.Create( m_Size.x, m_Size.y, uv.x, uv.y );
}

void Sprite::Render(
   const RenderDesc &desc,
   RenderStats *pStats
)
{
   Transform worldTransform;
   Vector color;
   int selected;

   desc.pDesc->argList.GetArg( 0, &color );
   desc.pDesc->argList.GetArg( 1, &worldTransform );
   desc.pDesc->argList.GetArg( 2, &selected );

   Quad *pQuad = (Quad *) desc.pDesc->buffer.Read( 0 );

   pQuad->Render( selected != 0, color, worldTransform, desc, pStats );
}
