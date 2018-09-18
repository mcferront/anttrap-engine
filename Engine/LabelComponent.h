#pragma once

#include "EngineGlobal.h"
#include "Component.h"
#include "TextArea.h"

class LabelComponent : public Component
{
public:
    DeclareComponentType( LabelComponent );

private:
    TextArea m_TextArea;

public:
    void Create(
        const Vector2 &size,
        const Vector &color,
        ResourceHandle back_material,
        ResourceHandle front_material,
        ResourceHandle font_map,
        ResourceHandle font_texture,
        const IdList &renderGroups
        );

    void Destroy( void );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    void SetBackground(
        ResourceHandle material
        )
    {
        m_TextArea.SetBackground( material );
    }

    void SetAlign( TextArea::Align horiz_alignment, TextArea::Align vert_alignment )
    {
        m_TextArea.SetAlign( horiz_alignment, vert_alignment );
    }

    void Clear( void )
    {
        m_TextArea.Clear( );
    }

    void PrintArgs(
        const char *pString,
        ...
        )
    {
        va_list args;

        va_start( args, pString );

        m_TextArea.PrintV( pString, args );

        va_end( args );
    }

    void Print(
        const char *pString
        )
    {
        m_TextArea.Print( pString );
    }

    void Pad(
        float vertical,
        float horizontal
        )
    {
        m_TextArea.Pad( vertical, horizontal );
    }

    const Vector2 &GetSize( void ) const
    {
        return m_TextArea.GetSize( );
    }

    void SetSize(
        const Vector2 &size
        )
    {
        m_TextArea.SetSize( size );
    }

    float GetAlpha( void ) const
    {
        return m_TextArea.GetAlpha( );
    }

    void SetAlpha(
        float alpha
        )
    {
        m_TextArea.SetAlpha( alpha );
    }

    const Vector &GetColor( void ) const { return m_TextArea.GetColor( ); }

    void SetColor( const Vector &color )
    {
        m_TextArea.SetColor( color );
    }
};

class LabelComponentSerializer : public ISerializer
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

    virtual ISerializable *Instantiate() const { return new LabelComponent; }

    virtual const SerializableType &GetSerializableType( void ) const { return LabelComponent::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
