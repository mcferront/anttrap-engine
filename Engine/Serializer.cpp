#include "EnginePch.h"

#include "Serializer.h"
#include "IOStreams.h"
#include "Resource.h"
#include "ResourceWorld.h"

SerializerWorld &SerializerWorld::Instance( void )
{
   static SerializerWorld s_instance;
   return s_instance;
}

void SerializerWorld::DeleteSerializers( void )
{
   {
      List<ISerializer*> list;
      list.Create( );

      {
         Enumerator<const char *, ISerializer *> e = m_SerializerHash.GetEnumerator( );
         while ( e.EnumNext( ) )
         {
            list.Add(e.Data( ));
         }
      }

      m_SerializerHash.Clear( );

      int i, size = list.GetSize( );

      for ( i = 0; i < size; i++ )
      {
         delete list.GetAt( i );
      }

      list.Destroy( );
   }
}

void SerializerWorld::AddISerializer(
   const TypeRegistry::TypeInfo *pType,
   ISerializer *pISerializer
)
{
   m_SerializerHash.Add( pISerializer->GetSerializableType( ).ToString( ), pISerializer );
}

Serializer::Serializer(
   IInputStream *pInputStream
)
{
   m_pInputStream   = pInputStream;
   m_pOutputStream  = NULL;
   m_CurrentVersion = 0;
}

Serializer::Serializer(
   IOutputStream *pOutputStream
)
{
   m_pOutputStream  = pOutputStream;
   m_pInputStream   = NULL;
   m_CurrentVersion = 0;
}

Serializer::Serializer(
   IInputStream *pInputStream,
   IOutputStream *pOutputStream
)
{
   m_pInputStream   = pInputStream;
   m_pOutputStream  = pOutputStream;
   m_CurrentVersion = 0;
}

bool Serializer::Serialize(
   const ISerializable *pSerializable
)
{
   Debug::Assert( Condition(NULL != m_pOutputStream), "Serializer needs a valid output stream" );
   
   ISerializer *pISerializer = SerializerWorld::Instance( ).GetISerializer( pSerializable->GetSerializableType().ToString() );
   Debug::Assert( Condition(NULL != pISerializer), "Serializer not found for type: %s", pSerializable->GetSerializableType().ToString() );

   struct Header
   {
      uint32 version;
   };

   Header header;
   uint32 bytesWritten;

   header.version = pISerializer->GetVersion( );
   m_pOutputStream->Write( &header, sizeof(header), &bytesWritten );

   Id::Serialize( m_pOutputStream, Id(pISerializer->GetSerializableType( ).ToString( )) );
   
   return pISerializer->Serialize( this, pSerializable );
}

ISerializable *Serializer::Deserialize(
   ISerializable *pSerializable
)
{
   Debug::Assert( Condition(NULL != m_pInputStream), "Serializer needs a valid input stream" );

   struct Header
   {
      uint32 version;
   };

   Header header;
   uint32 bytesRead;

   m_pInputStream->Read( &header, sizeof(header), &bytesRead );
   Id typeString = Id::Deserialize( m_pInputStream );

   ISerializer *pISerializer = SerializerWorld::Instance( ).GetISerializer( typeString.ToString() );
   Debug::Assert( Condition(NULL != pISerializer), "Serializer not found for %s", typeString.ToString() );

   if ( pISerializer )
   {
      uint32 previousVersion = m_CurrentVersion;

      m_CurrentVersion = header.version;

      pSerializable = pISerializer->Deserialize(this, pSerializable);
      
      m_CurrentVersion = previousVersion;

      Debug::Assert( Condition(NULL != pSerializable), "Serializer failed to deserialize type: %s", typeString.ToString() );

      return pSerializable;      
   }

   return NULL;
}
