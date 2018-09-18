#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Serializer.h"
#include "StringPool.h"

class FontMap : public Asset
{
    friend class FontMapSerializer;

public:
    struct CharacterDesc
    {
        float x;
        float y;
        float width;
        float height;
    };

private:
    CharacterDesc *m_pCharacters;
    size_t m_Count;

    int m_SpacingWidth;
    int m_SpacingHeight;

public:
    virtual void Create( )
    {
        m_pCharacters = NULL;

        m_Count = 0;
        m_SpacingWidth = 0;
        m_SpacingHeight = 0;
    }

    inline CharacterDesc *GetCharacter( uint32 index )
    {
        if ( index >= m_Count )
        {
            index = '*' - 32;
        }
        return &m_pCharacters[ index ];
    }

    int GetSpacingWidth( void ) const { return m_SpacingWidth; }
    int GetSpacingHeight( void ) const { return m_SpacingHeight; }

    virtual void Destroy( void )
    {
        free( m_pCharacters );

        Asset::Destroy( );
    }

    DeclareResourceType( FontMap );
};

class FontMapSerializer : public ISerializer
{
public:
    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        )
    {
        return false;
    }

    virtual ISerializable *Instantiate( ) const { return new FontMap; }

    virtual const SerializableType &GetSerializableType( void ) const { return FontMap::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};

