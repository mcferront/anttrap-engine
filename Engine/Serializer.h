#pragma once

#include "EngineGlobal.h"
#include "HashTable.h"
#include "ResourceType.h"
#include "ResourceMaps.h"
#include "ISerializable.h"

class IInputStream;
class IOutputStream;
class Serializer;
class Resource;
class Channel;

class SerializerWorld
{
public:
   static SerializerWorld &Instance( void );
   
private:
   HashTable<const char *, ISerializer *> m_SerializerHash;

public:
   SerializerWorld( void ) {}

   void Create ( void ) 
   {
      m_SerializerHash.Create( 16, 16 );
   };

   void Destroy( void )
   {
      m_SerializerHash.Destroy( );
   }

   void DeleteSerializers( void );

   void AddISerializer(
      const TypeRegistry::TypeInfo *pType,
      ISerializer *pISerializer
   );

   ISerializer *GetISerializer(
      const char *pString
   )
   {
      ISerializer *pISerializer = NULL;

      m_SerializerHash.Get( pString, &pISerializer );

      return pISerializer;
   }
};

class Serializer
{
private:
   IInputStream  *m_pInputStream;
   IOutputStream *m_pOutputStream;
   uint32         m_CurrentVersion;

public:
   Serializer(
      IInputStream *pInputStream
   );

   Serializer(
      IOutputStream *pOutputStream
   );

   Serializer(
      IInputStream *pInputStream,
      IOutputStream *pOutputStream
   );

   bool Serialize(
      const ISerializable *pSerializable
   );

   ISerializable *Deserialize(   
      ISerializable *pObject
   );

   uint32 GetCurrentVersion( void ) const { return m_CurrentVersion; }

   IInputStream *GetInputStream  ( void ) { Debug::Assert( Condition(NULL != m_pInputStream),  "InputStream is NULL" ); return m_pInputStream; }
   IOutputStream *GetOutputStream( void ) { Debug::Assert( Condition(NULL != m_pOutputStream), "OutputStream is NULL" ); return m_pOutputStream; }
};
