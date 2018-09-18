#pragma once

#include "EngineGlobal.h"
#include "ResourceWorld.h"
#include "Serializer.h"
#include "Asset.h"

class Lua : public Asset
{
    friend class LuaSerializer;

private:
    char   *m_pData;
    uint32  m_Size;

public:
    virtual void Destroy( void );

    virtual void AddToScene( void );
    virtual void RemoveFromScene( void );

    DeclareResourceType( Lua );
};

class LuaSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new Lua; }

    virtual const SerializableType &GetSerializableType( void ) const { return Lua::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
