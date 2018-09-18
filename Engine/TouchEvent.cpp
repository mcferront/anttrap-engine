#include "EnginePch.h"

#include "TouchEvent.h"
#include "TouchWorld.h"
#include "Channel.h"
#include "Component.h"
#include "Node.h"

void TouchEvent::Create(
    Component *pComponent,
    const Vector2 &size,
    Id touchStageId
    )
{
    TouchObject::Create( pComponent );

    SetRect( size );

    m_TouchStageId = touchStageId;

    m_ClampsTouch = false;
}

void TouchEvent::Destroy( void )
{
}

void TouchEvent::SetRect(
    const Vector2 &size
    )
{
    m_HalfSize  = size * 0.5f;
}

void TouchEvent::AddToScene( void )
{
    TouchObject::AddToScene( m_TouchStageId );
}

void TouchEvent::BeginTouch(
    const Vector2 &position
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "BeginTouch", ArgList(local.x, local.y) );
}

void TouchEvent::Touch(
    const Vector2 &position
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "Touch", ArgList(local.x, local.y) );
}

void TouchEvent::EndTouch(
    const Vector2 &position
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "EndTouch", ArgList(local.x, local.y) );
}

void TouchEvent::BeginHover(
    const Vector2 &position
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "BeginHover", ArgList(local.x, local.y) );
}

void TouchEvent::Hover(
    const Vector2 &position
    )
{
    UpdateTransform( );
    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "Hover", ArgList(local.x, local.y) );
}

void TouchEvent::EndHover(
    const Vector2 &position
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "EndHover", ArgList(local.x, local.y) );
}

void TouchEvent::CancelTouch(
    const Vector2 &position
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::TransformPosition( &local, position, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "CancelTouch", ArgList(local.x, local.y) );
}

void TouchEvent::Swipe( 
    const Vector2 &swipe
    )
{
    UpdateTransform( );

    Vector2 local;
    Math::Rotate( &local, swipe, m_InvTransform );

    GetComponent( )->GetParent( )->GetChannel( )->SendEvent( "Swipe", ArgList(local.x, local.y) );   
}

void TouchEvent::SetTouchStageId( Id touchStageId )
{
    TouchObject::RemoveFromScene( );

    m_TouchStageId = touchStageId;
    TouchObject::AddToScene( m_TouchStageId );
}

bool TouchEvent::InRange(
    float x,
    float y,
    float scale //= 1.0f
    )
{
    Vector pos( x, y, 0.0f );

    UpdateTransform( );

    Math::TransformPosition( &pos, pos, m_InvTransform );

    Vector2 halfScaled = m_HalfSize * scale;

    if ( pos.x > - halfScaled.x && pos.x < halfScaled.x )
    {
        if ( pos.y > - halfScaled.y && pos.y < halfScaled.y )
        {
            return true;
        }
    }

    return false;
}

void TouchEvent::UpdateTransform( void )
{
    if ( m_PrevTransform != GetComponent()->GetParent()->GetWorldTransform() )
    {
        m_PrevTransform = GetComponent()->GetParent()->GetWorldTransform();
        Math::Invert( &m_InvTransform, m_PrevTransform );
    }
}