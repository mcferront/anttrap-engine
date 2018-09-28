#pragma once

#include "EngineGlobal.h"
#include "GpuBuffer.h"

class Texture : public GpuBuffer
{
    friend class TextureSerializer;

public:
    DeclareResourceType(Texture);
};


class TextureSerializer : public ISerializer
{
public:
    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
    ) {
        return false;
    }

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
    );

    virtual ISerializable *Instantiate() const { return new Texture; }

    virtual const SerializableType &GetSerializableType(void) const { return Texture::StaticSerializableType(); }

    virtual uint32 GetVersion(void) const { return 2; }
};
