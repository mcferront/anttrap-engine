#include "EnginePch.h"

#include "SpriteComponent.h"

DefineComponentType(SpriteComponent, new SpriteComponentSerializer);

void SpriteComponent::Create(
    Id id,
    ResourceHandle material,
    const IdList &renderGroups,
    const Vector2 &size
    )
{
    SetId( id );

    m_Renderable.Create( this, material, renderGroups, size );
}

void SpriteComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    Component::AddToScene( );
    m_Renderable.AddToScene( );
}

void SpriteComponent::RemoveFromScene( void )
{
    m_Renderable.RemoveFromScene( );

    Component::RemoveFromScene( );
}

void SpriteComponent::Destroy( void )
{
    m_Renderable.Destroy( );

    Component::Destroy( );
}

ISerializable *SpriteComponentSerializer::Deserialize(
    Serializer *pSerializer,
    ISerializable *pSerializable
    )
{
    if ( NULL == pSerializable ) pSerializable = new SpriteComponent; 

    SpriteComponent *pSpriteComponent = (SpriteComponent *) pSerializable;

    Id id = Id::Deserialize( pSerializer->GetInputStream() );
    Id material = Id::Deserialize( pSerializer->GetInputStream() );

    IdList renderGroups;
    Id::DeserializeList( pSerializer->GetInputStream(), &renderGroups );

    int width, height;

    pSerializer->GetInputStream()->Read( &width, sizeof(width) );
    pSerializer->GetInputStream()->Read( &height, sizeof(height) );

    pSpriteComponent->Create( id, material, renderGroups, Vector2((float)width, (float)height) );

    return pSerializable;
}

