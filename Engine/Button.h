#pragma once

#include "EngineGlobal.h"
#include "Sprite.h"
#include "TouchEvent.h"

class Button
{
private:
    ResourceHandle m_UpMaterial;
    ResourceHandle m_DownMaterial;
    ResourceHandle m_HoverMaterial;
    Vector         m_Color;
    Sprite         m_Sprite;
    TouchEvent     m_TouchEvent;
    Vector2        m_RequestedSize;

public:
    void Create(
        Component *pComponent,
        ResourceHandle upMaterial,
        ResourceHandle downMaterial,
        ResourceHandle hoverMaterial,
        const Vector2 &size,
        const IdList &renderGroups,
        Id touchStageId
        );

    void Destroy( void );

    void AddToScene( void ); 

    void Bind( void );

    void RemoveFromScene( void );

    void SetSize( const Vector2 &size );

    void SetColor(
        const Vector &color
        );

    const Vector &GetColor( void ) const { return m_Color; }
    const Vector2 *GetSize( void ) const { return m_Sprite.GetSize( ); }

    void SetAlpha( float alpha );
    float GetAlpha( void ) const { return m_Color.w; }

    Id GetTouchStageId( void ) const { return m_TouchEvent.GetTouchStageId( ); }
    void SetTouchStageId( Id touchStageId ) { m_TouchEvent.SetTouchStageId( touchStageId ); }

private:
    void Down( 
        const Channel *pChannel,
        const char *, 
        const ArgList &
        );

    void Up( 
        const Channel *pChannel,
        const char *, 
        const ArgList &
        );

    void Click( 
        const Channel *pChannel,
        const char *, 
        const ArgList &
        );

    void BeginHover( 
        const Channel *pChannel,
        const char *, 
        const ArgList &
        );

    void EndHover( 
        const Channel *pChannel,
        const char *, 
        const ArgList &
        );
};
