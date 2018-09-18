#pragma once

#include "EngineGlobal.h"
#include "ResourceType.h"

struct SerializableType
{
private:
    const char *m_pType;

public:
    SerializableType( void )
    {
        m_pType = NULL;
    }

    SerializableType(
        const char *pType
        )
    {
        m_pType = StringRef(pType);
    }

    bool operator == ( const SerializableType &rhs ) const
    {
        return 0 == memcmp( this, &rhs, sizeof( rhs ) );
    }

    const char *ToString( ) const { return m_pType; }
};

class ISerializable
{
public:
    virtual ~ISerializable( void ) {}

    virtual const SerializableType &GetSerializableType( void ) const = 0;
};

class Serializer;
class Serializable;

class ISerializer
{
public:
    virtual ~ISerializer( void ) {}

    virtual bool Serialize(
        Serializer *pSerializer,
        const ISerializable *pSerializable
        ) = 0;

    virtual ISerializable *Deserialize(
        Serializer *pSerializer,
        ISerializable *pSerializable
        ) = 0;

    virtual ISerializable *Instantiate( ) const = 0;

    virtual const SerializableType &GetSerializableType( void ) const = 0;

    virtual uint32 GetVersion( void ) const = 0;
};


#define DeclareSerializableType(className)\
virtual const SerializableType &GetSerializableType( void ) const { return className::StaticSerializableType( ); }\
static const SerializableType &LoadSerializableType( void );\
static void Register( void );\
static const SerializableType &StaticSerializableType( void )\
{\
   static const SerializableType &type = LoadSerializableType( );\
   return type;\
}

#define DefineSerializableType(className, serializerAddr)\
void className::Register( void )\
{\
   TypeRegistry_RegisterTypeInfo(className);\
   ISerializer *pSerializer = serializerAddr;\
   SerializerWorld::Instance( ).AddISerializer( TypeRegistry::GetTypeInfo(typeid(className*)), pSerializer );\
}\
const SerializableType &className::LoadSerializableType( void )\
{\
   static SerializableType s_Type( #className );\
   return s_Type;\
}
