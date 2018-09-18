#include "EnginePch.h"

#include "LightComponent.h"
#include "Node.h"
#include "RenderWorld.h"

DefineComponentType( LightComponent, NULL );
DefineComponentType( DirectionalLightComponent, new DirectionalLightComponentSerializer );
DefineComponentType( AmbientLightComponent, new AmbientLightComponentSerializer );
DefineComponentType( SpotLightComponent, new SpotLightComponentSerializer );
DefineComponentType( PointLightComponent, new PointLightComponentSerializer );

void LightComponent::Destroy( void )
{
   Component::Destroy( );
}

void LightComponent::AddToScene( void )
{
   if ( false == GetParent( )->IsInScene( ) ) return;

   Component::AddToScene( );
   m_Light.AddToScene( );
}

void LightComponent::RemoveFromScene( void )
{
   m_Light.RemoveFromScene( );

   Component::RemoveFromScene( );
}

void DirectionalLightComponent::Create(
   const IdList &renderGroups,
   Vector color,
   float nits
   )
{
   LightDesc desc;
   desc.color = color;
   desc.nits = nits;
   desc.inner = FLT_MAX;
   desc.outer = FLT_MAX;
   desc.range = FLT_MAX;
   desc.cast = LightDesc::CastDirectional;

   LightComponent::Create( renderGroups, desc );
}

void AmbientLightComponent::Create(
   const IdList &renderGroups,
   Vector color,
   float nits
   )
{
   LightDesc desc;
   desc.color = color;
   desc.nits = nits;
   desc.inner = FLT_MAX;
   desc.outer = FLT_MAX;
   desc.range = FLT_MAX;
   desc.cast = LightDesc::CastAmbient;

   LightComponent::Create( renderGroups, desc );
}

void SpotLightComponent::Create(
   const IdList &renderGroups,
   Vector color,
   float nits,
   float inner,
   float outer,
   float range
   )
{
   LightDesc desc;

   desc.color = color;
   desc.nits = nits;
   desc.inner = inner;
   desc.outer = outer;
   desc.range = range;
   desc.cast = LightDesc::CastSpot;

   LightComponent::Create( renderGroups, desc );
}

void PointLightComponent::Create(
   const IdList &renderGroups,
   Vector color,
   float nits,
   float range
   )
{
   LightDesc desc;

   desc.color = color;
   desc.nits = nits;
   desc.inner = FLT_MAX;
   desc.outer = FLT_MAX;
   desc.range = range;
   desc.cast = LightDesc::CastOmni;

   LightComponent::Create( renderGroups, desc );
}

void PointLightComponent::SetColor(
   const Vector &color
)
{
   m_Light.SetColor( color );
}

ISerializable *DirectionalLightComponentSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
   )
{
   return NULL;
   //if ( NULL == pSerializable ) pSerializable = new DirectionalLightComponent;

   //DirectionalLightComponent *pDirectionalLightComponent = (DirectionalLightComponent *) pSerializable;

   //Id id;
   //IdList renderGroups;
   //Color color;
   //float nits;

   //id = Id::Deserialize( pSerializer->GetInputStream( ) );
   //Id::DeserializeList( pSerializer->GetInputStream( ), &renderGroups );
   //pSerializer->GetInputStream( )->Read( &color, sizeof( color ) );
   //pSerializer->GetInputStream( )->Read( &nits, sizeof( nits ) );

   //pDirectionalLightComponent->Create( renderGroups, Vector( color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f ), nits );

   //return pSerializable;
}

ISerializable *AmbientLightComponentSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
   )
{
   return NULL;
   //if ( NULL == pSerializable ) pSerializable = new AmbientLightComponent;

   //AmbientLightComponent *pAmbientLightComponent = (AmbientLightComponent *) pSerializable;

   //Id id;
   //Color color;
   //IdList renderGroups;

   //id = Id::Deserialize( pSerializer->GetInputStream( ) );
   //Id::DeserializeList( pSerializer->GetInputStream( ), &renderGroups );
   //pSerializer->GetInputStream( )->Read( &color, sizeof( color ) );

   //pAmbientLightComponent->Create( renderGroups, Vector( color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f ) );

   //return pSerializable;
}

ISerializable *SpotLightComponentSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
   )
{
   return NULL;
//   if ( NULL == pSerializable ) pSerializable = new SpotLightComponent;
//
//   SpotLightComponent *pSpotLightComponent = (SpotLightComponent *) pSerializable;
//
//   Id id;
//   IdList renderGroups;
//   Color color;
//   float nits, inner, outer, range;
//
//   id = Id::Deserialize( pSerializer->GetInputStream( ) );
//   Id::DeserializeList( pSerializer->GetInputStream( ), &renderGroups );
//   pSerializer->GetInputStream( )->Read( &color, sizeof( color ) );
//   pSerializer->GetInputStream( )->Read( &nits, sizeof( nits ) );
//   pSerializer->GetInputStream( )->Read( &inner, sizeof( inner ) );
//   pSerializer->GetInputStream( )->Read( &outer, sizeof( outer ) );
//   pSerializer->GetInputStream( )->Read( &range, sizeof( range ) );
//
//   pSpotLightComponent->Create( renderGroups, Vector( color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f ), nits, inner, outer, range );
//
//   return pSerializable;
}

ISerializable *PointLightComponentSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
   )
{
   return NULL;
   //if ( NULL == pSerializable ) pSerializable = new PointLightComponent;

   //PointLightComponent *pPointLightComponent = (PointLightComponent *) pSerializable;

   //Id id;
   //IdList renderGroups;
   //Color color;
   //float nits, range;

   //id = Id::Deserialize( pSerializer->GetInputStream( ) );
   //Id::DeserializeList( pSerializer->GetInputStream( ), &renderGroups );
   //pSerializer->GetInputStream( )->Read( &color, sizeof( color ) );
   //pSerializer->GetInputStream( )->Read( &nits, sizeof( nits ) );
   //pSerializer->GetInputStream( )->Read( &range, sizeof( range ) );

   //pPointLightComponent->Create( renderGroups, Vector( color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f ), nits, range );

   //return pSerializable;
}
