#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "TextureAsset.h"

class FrameMap : public Asset
{
    friend class FrameMapSerializer;

private:
    int m_Width;
    int m_Height;
    int m_Count;

    float    m_Framerate;
    Vector   m_Movement;
    Texture *m_pTextures;

public:
    virtual void Destroy( void );

    int GetCount( void ) const { return m_Count; }

    const Vector *GetMovementVector( void ) const { return &m_Movement; }

    Texture *GetTexture( int index ) const { return &m_pTextures[ index ]; }

    float GetFramerate( void ) const { return m_Framerate; }

    DeclareResourceType( FrameMap );
};

class FrameMapSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new FrameMap; }

    virtual const SerializableType &GetSerializableType( void ) const { return FrameMap::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
