#pragma once

#include "EngineGlobal.h"
#include "Asset.h"

class Fbx : public Asset
{
    friend class FbxSerializer;

public:
    DeclareResourceType( Fbx );
};

class FbxSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new Fbx; }

    virtual const SerializableType &GetSerializableType( void ) const { return Fbx::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
