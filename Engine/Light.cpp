#include "EnginePch.h"

#include "Light.h"
#include "DebugGraphics.h"
#include "CameraComponent.h"
#include "RegistryWorld.h"

void Light::Create(Component *pComponent,
   const IdList &renderGroups,
   const LightDesc &desc
   )
{
   m_RenderGroups.Create( renderGroups.GetSize( ), 0 );
   m_RenderGroups.CopyFrom( renderGroups );

   m_pComponent = pComponent;
   m_Desc = desc;
   m_Desc.pLight = this;

   m_Extents = Math::ZeroVector();

   if ( m_Desc.cast == LightDesc::Cast::CastSpot )
   {
      m_CosOuter = Math::Cos( m_Desc.outer / 2.0f );
      m_CosInner = Math::Cos( m_Desc.inner / 2.0f );

      float opp = Math::Abs(m_Desc.range * Math::Tan(m_Desc.outer / 2.0f));

      m_Extents.x = opp;
      m_Extents.y = opp;
      m_Extents.z = m_Desc.range * .5f;

      // TODO: bug - near shouldn't have to be 0.30f 
      // but it does for a valid frustum?
      m_Camera.CreateAsPerspective( m_Desc.outer, 1.0f, 0.3f, m_Desc.range );
   }
}

void Light::GetRenderData(
   RendererDesc *pDesc
   ) const
{
 
   LightDesc desc = m_Desc;

   if ( LightDesc::CastAmbient == m_Desc.cast )
   {
      pDesc->ambient_light += m_Desc.color * m_Desc.nits;

      {
         static RegistryFloat ambient( "Light/debug_nits", 0.0f );
         pDesc->ambient_light.x += ambient.GetValue();
         pDesc->ambient_light.y += ambient.GetValue();
         pDesc->ambient_light.z += ambient.GetValue();
      }
   }
   else if ( LightDesc::CastDirectional == m_Desc.cast )
   {
      desc.position = Math::ZeroVector();
      desc.direction = m_pComponent->GetParent( )->GetWorldTransform( ).GetLook( );

      CameraComponent *pCamera = (CameraComponent *) m_pComponent->GetParent( )->GetComponent( CameraComponent::StaticType() );
      
      if ( NULL != pCamera )
      {
         Transform view;
         Matrix projection;
         
         pCamera->GetCamera( )->GetProjection( &projection );
         pCamera->GetCamera( )->GetViewTransform( &view );

         pDesc->shadowViewMatrix = view.ToMatrix(true);
         pDesc->shadowProjMatrix = projection;

         static RegistryBool render( "Light/render_shadow_light", false );
         static const char *pForward = StringRef("Forward");

         if ( StringRefEqual(pDesc->pPass, pForward) && render.GetValue() == true )
         {
            Transform worldTransform;
            m_pComponent->GetParent( )->GetWorldTransform( &worldTransform );

            DebugGraphics::Instance( ).RenderTransform( worldTransform, pCamera->GetCamera()->GetFarClip() - pCamera->GetCamera()->GetNearClip() );
         }
      }
      if ( desc.color.w != 0 )
         pDesc->lightDescs.Add( desc );
   }
   else if ( LightDesc::CastOmni == m_Desc.cast )
   {
      Transform worldTransform;
      m_pComponent->GetParent( )->GetWorldTransform( &worldTransform );

      desc.position = worldTransform.GetTranslation( );
      desc.direction = worldTransform.GetLook( );

      static RegistryBool render( "Light/render_omnis", false );
      static const char *pForward = StringRef("Forward");

      pDesc->lightDescs.Add( desc );

      if ( StringRefEqual(pDesc->pPass, pForward) && render.GetValue() == true )
      {
         DebugGraphics::Instance( ).RenderTransform( worldTransform, m_Desc.range * .25f );
         DebugGraphics::Instance( ).RenderSphere( worldTransform, m_Desc.range, Vector(1, 1, 1, .25) );
      }
   }
   else if ( LightDesc::CastSpot == m_Desc.cast )
   {
      Transform worldTransform;
      m_pComponent->GetParent( )->GetWorldTransform( &worldTransform );

      desc.position = worldTransform.GetTranslation( );
      desc.direction = worldTransform.GetLook( );
      desc.outer = m_CosOuter;
      desc.inner = m_CosInner;

      static RegistryBool render( "Light/render_spots", false );
      static const char *pForward = StringRef("Forward");

      if ( desc.color.w != 0 )
         pDesc->lightDescs.Add( desc );

      if ( StringRefEqual(pDesc->pPass, pForward) && render.GetValue() == true )
      {
         Vector center = Vector(0, 0, m_Desc.range * .5f, 1.0f);

         // TODO should render the frustum, not a cube
         DebugGraphics::Instance( ).RenderCube( worldTransform, center, m_Extents, Vector(1, 1, 1, 1) );
         DebugGraphics::Instance( ).RenderArrow( m_pComponent->GetParent( )->GetWorldTransform( ).GetTranslation( ), desc.direction, Vector(1, 1, 1, 1), desc.range );
      }
   }
   else
      Debug::Assert( Condition(false), "Unrecognized Light Type" );
}

void Light::Destroy( void )
{
   m_RenderGroups.Destroy( );
}

void Light::Flush( void )
{
   Transform worldTransform;
   m_pComponent->GetParent( )->GetWorldTransform( &worldTransform );

   m_Camera.SetWorldTransform( worldTransform );
   m_Camera.GetFrustum( &m_Frustum );
}

bool Light::IsVisible(
   const Frustum &frustum
) const
{

   if ( LightDesc::CastOmni == m_Desc.cast )
   {
      Transform worldTransform;
      m_pComponent->GetParent( )->GetWorldTransform( &worldTransform );

      Vector position = worldTransform.GetTranslation( );

      return SphereInFrustum( position, m_Desc.range, frustum );
   }
   else if ( LightDesc::CastSpot == m_Desc.cast )
   {
		//Transform worldTransform;
      //m_pComponent->GetParent( )->GetWorldTransform( &worldTransform );

      //// TODO: this should be frustum vs frustum or cone vs frustum
      //Vector localCenter( 0, 0, m_Extents.z, 1.0f );
      //return BoxInFrustum( localCenter, m_Extents, worldTransform, frustum );
   }

   return true;
}

bool Light::IsInRange(
   const Transform &boxTransform,
   const Transform &invBoxTransform,
   const Vector &boxCenter,
   const Vector &boxExtents
) const
{
   if ( LightDesc::CastOmni == m_Desc.cast )
   {
      Vector lightPosition;
      m_pComponent->GetParent( )->GetWorldPosition( &lightPosition );

      return SphereInBox( boxCenter, boxExtents, invBoxTransform, lightPosition, m_Desc.range );
   }
   else if ( LightDesc::CastSpot == m_Desc.cast )
   {
      return BoxInFrustum( boxCenter, boxExtents, boxTransform, m_Frustum );   
   }

   return true;
}
