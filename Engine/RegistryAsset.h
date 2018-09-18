#pragma once

#include "EngineGlobal.h"
#include "Asset.h"
#include "Serializer.h"
#include "RegistryWorld.h"

class Registry : public Asset
{
    friend class RegistrySerializer;

public:
    DeclareResourceType( Registry );

private:
    HashTable<const char *, RegistryValue> m_Hashtable;

public:
    virtual void Create( )
    {
        m_Hashtable.Create( );
    }

    virtual void Destroy( void )
    {
        m_Hashtable.Destroy( );

        Asset::Destroy( );
    }

    void Create(
        const HashTable<const char *, RegistryValue> &hashtable
        );

    Enumerator<const char *, RegistryValue> GetEnumerator( void ) const { return m_Hashtable.GetEnumerator( ); }
};

class RegistrySerializer : public ISerializer
{
public:
    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        );

    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        );

    virtual ISerializable *Instantiate( ) const { return new Registry; }

    virtual const SerializableType &GetSerializableType( void ) const { return Registry::StaticSerializableType( ); }

    virtual uint32 GetVersion( void ) const { return 1; }
};

