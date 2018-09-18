#include "EnginePch.h"

#include "TouchComponent.h"

DefineComponentType(TouchComponent, NULL);

void TouchComponent::Create(
    Id id,
    const Vector2 &size,
    Id touchStageId
    )
{
    SetId( id );

    m_TouchEvent.Create( this, size, touchStageId ); 
}

void TouchComponent::Destroy( void )
{
    m_TouchEvent.Destroy( );

    Component::Destroy( );
}

void TouchComponent::AddToScene( void )
{
    if ( false == GetParent()->IsInScene() ) return;

    Component::AddToScene( );
    m_TouchEvent.AddToScene( );
}

void TouchComponent::RemoveFromScene( void )
{
    m_TouchEvent.RemoveFromScene( );

    Component::RemoveFromScene( );
}
