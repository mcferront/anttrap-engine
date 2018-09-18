#pragma once

#include "EngineGlobal.h"
#include "Asset.h"

class Data : public Asset
{
    friend class DataSerializer;

private:
    void  *m_pData;
    uint32 m_Size;

public:
    virtual void Destroy( void );

    const void *GetData( void ) const { return m_pData; }
    uint32 GetSize( void ) const { return m_Size; }

    DeclareResourceType( Data );
};

class DataSerializer : public ISerializer
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

    virtual ISerializable *Instantiate( ) const { return new Data; }

    virtual const SerializableType &GetSerializableType( void ) const { return Data::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};
