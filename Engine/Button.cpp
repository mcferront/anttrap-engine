#include "EnginePch.h"

#include "Button.h"
#include "ResourceWorld.h"

void Button::Create(
    Component *pComponent,
    ResourceHandle upMaterial,
    ResourceHandle downMaterial,
    ResourceHandle hoverMaterial,
    const Vector2 &size,
    const IdList &renderGroups,
    Id touchStageId
    )
{
    m_Sprite.Create( pComponent, upMaterial, renderGroups, size );
    
    Vector2 adjustedSize = *m_Sprite.GetSize( );

    m_TouchEvent.Create( pComponent, adjustedSize, touchStageId );

    m_RequestedSize = size;
    m_Color = Vector( 1, 1, 1, 1 );
    m_UpMaterial   = upMaterial;
    m_DownMaterial = downMaterial;
    m_HoverMaterial= hoverMaterial;
}

void Button::Destroy( void )
{
    m_Sprite.Destroy( );
    m_TouchEvent.Destroy( );

    m_UpMaterial   = NullHandle;
    m_DownMaterial = NullHandle;
    m_HoverMaterial= NullHandle;
}

void Button::Bind( void )
{
    SetSize( m_RequestedSize );
}

void Button::AddToScene( void )
{
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->AddEventListener( EventMap<Button>("BeginTouch", this, &Button::Down) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->AddEventListener( EventMap<Button>("Touch",      this, &Button::Down) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->AddEventListener( EventMap<Button>("EndTouch",   this, &Button::Click) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->AddEventListener( EventMap<Button>("CancelTouch",this, &Button::Up) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->AddEventListener( EventMap<Button>("BeginHover", this, &Button::BeginHover) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->AddEventListener( EventMap<Button>("EndHover",   this, &Button::EndHover) );

    if ( m_UpMaterial != NullHandle )
        m_Sprite.AddToScene( );

    m_TouchEvent.AddToScene( );
}

void Button::RemoveFromScene( void )
{
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->RemoveEventListener( EventMap<Button>("BeginTouch", this, &Button::Down) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->RemoveEventListener( EventMap<Button>("Touch",      this, &Button::Down) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->RemoveEventListener( EventMap<Button>("EndTouch",   this, &Button::Click) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->RemoveEventListener( EventMap<Button>("CancelTouch",this, &Button::Up) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->RemoveEventListener( EventMap<Button>("BeginHover", this, &Button::BeginHover) );
    m_TouchEvent.GetComponent( )->GetParent( )->GetChannel( )->RemoveEventListener( EventMap<Button>("EndHover",   this, &Button::EndHover) );

    if ( m_UpMaterial != NullHandle )
        m_Sprite.RemoveFromScene( );

    m_TouchEvent.RemoveFromScene( );
}

void Button::SetSize( const Vector2 &size )
{
    m_RequestedSize = size;
    m_Sprite.SetSize( size );

    Vector2 adjustedSize = *m_Sprite.GetSize( );

    m_TouchEvent.SetRect( adjustedSize );
}

void Button::SetColor(
    const Vector &color
    )
{
    m_Color = color;
    m_Sprite.SetColor( color );
}

void Button::SetAlpha(
    float alpha
    )
{
    m_Color.w = alpha;
    m_Sprite.SetAlpha( alpha );
}

void Button::Down( 
    const Channel *pChannel,
    const char *, 
    const ArgList &
    )
{
    if ( false == IsResourceLoaded(m_DownMaterial) )
    {
        Vector half = m_Color * 0.5f;
        half.w = m_Color.w;
        m_Sprite.SetColor( half );
    }
    else
    {
        m_Sprite.SetMaterial( m_DownMaterial );
    }
}

void Button::Up( 
    const Channel *pChannel,
    const char *, 
    const ArgList &
    )
{
    if ( false == IsResourceLoaded(m_UpMaterial) )
    {
        m_Sprite.SetColor( m_Color );
    }
    else
    {
        m_Sprite.SetMaterial( m_UpMaterial );
    }
}

void Button::Click(
    const Channel *pChannel,
    const char *,
    const ArgList &
    )
{
    if ( false == IsResourceLoaded(m_DownMaterial) )
    {
        m_Sprite.SetColor( m_Color );
    }
    else
    {
        m_Sprite.SetMaterial( m_UpMaterial );
    }
}

void Button::BeginHover( 
    const Channel *pChannel,
    const char *, 
    const ArgList &
    )
{
    if ( true == IsResourceLoaded(m_HoverMaterial) )
    {
        m_Sprite.SetMaterial( m_HoverMaterial );
    }
}

void Button::EndHover( 
    const Channel *pChannel,
    const char *, 
    const ArgList &
    )
{
    if ( false == IsResourceLoaded(m_UpMaterial) )
    {
        m_Sprite.SetColor( m_Color );
    }
    else
    {
        m_Sprite.SetMaterial( m_UpMaterial );
    }
}
