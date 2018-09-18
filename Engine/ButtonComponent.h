#pragma once

#include "EngineGlobal.h"

#include "Component.h"
#include "Button.h"
#include "TextArea.h"

class ButtonComponent : public Component
{
public:
    DeclareComponentType(ButtonComponent);

private:    
    Button         m_Button;
    ResourceHandle m_MainMaterial;
    TextArea      *m_pTextArea;
    IdList         m_RenderGroups;

public:
    void Create(
        Id id,
        ResourceHandle upMaterial,
        ResourceHandle downMaterial,
        ResourceHandle hoverMaterial,
        const Vector2 &size,
        const IdList &renderGroups,
        Id touchStageId
        );

    virtual void Destroy( void );

    virtual void AddToScene( void );

    virtual void Bind( void );

    virtual void RemoveFromScene( void );

    void SetupText( 
        const char *pString, 
        const Vector &textColor, 
        int hPad, 
        int vPad, 
        TextArea::Align horiz_align, 
        TextArea::Align vert_align, 
        ResourceHandle material,
        ResourceHandle fontMap,
        ResourceHandle fontTexture
        );
   
    void SetColor(
        const Vector &color
        )
    {
        m_Button.SetColor( color );
    }

    void SetSize( 
        const Vector2 &size 
        )
    {
        m_Button.SetSize( size );
    }

    void SetAlpha( float alpha ) { m_Button.SetAlpha(alpha); }   
    float GetAlpha( void ) const { return m_Button.GetAlpha(); }

    Id GetTouchStageId( void ) const { return m_Button.GetTouchStageId( ); }
    void SetTouchStageId( Id touchStageId ) { m_Button.SetTouchStageId( touchStageId ); }

private:
    void SetWorldTransforms( void );
};

class ButtonComponentSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate() const { return new ButtonComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return ButtonComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
