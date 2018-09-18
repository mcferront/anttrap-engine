#include "EnginePch.h"

#include "RegistryAsset.h"
#include "IOStreams.h"

DefineResourceType(Registry, Asset, new RegistrySerializer)

void Registry::Create(
   const HashTable<const char *, RegistryValue> &hashtable
)
{
   m_Hashtable.Create( );
   m_Hashtable.Copy( hashtable );
}

ISerializable *RegistrySerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   Registry *pRegistry = (Registry *) pSerializable;

   struct Header
   {
      int version;
      int count;
   };

   Header header;

   if ( NULL == pSerializable ) pSerializable = new Registry; 

   pSerializer->GetInputStream( )->Read( &header, sizeof(header) );
   Debug::Assert( Condition(header.version <= 2), "Old registry data (%d but need %d), please repackage the scene", header.version, 2 );

   pRegistry->Create( );

   int i;

   char key[ 256 ];

   for ( i = 0; i < header.count; i++ )
   {
      int length;

      pSerializer->GetInputStream( )->Read( &length, sizeof(length) );
      Debug::Assert( Condition(length <= sizeof(key) - 1), "Registry key is larger than max size: %d", sizeof(key) - 1 );

      pSerializer->GetInputStream( )->Read( key, length );
      key[ length ] = NULL;

      int type;
      pSerializer->GetInputStream( )->Read( &type, sizeof(type) );

      if ( RegistryValue::Int == type )
      {
         int value;
         pSerializer->GetInputStream( )->Read( &value, sizeof(value) );
         
         const char *pKey = StringRef( key );
         pRegistry->m_Hashtable.Add( pKey, RegistryValue(value, pKey) );
      }
      else if ( RegistryValue::Float == type )
      {
         float value;
         pSerializer->GetInputStream( )->Read( &value, sizeof(value) );

         const char *pKey = StringRef( key );
         pRegistry->m_Hashtable.Add( pKey, RegistryValue(value, pKey) );
      }
      else if ( RegistryValue::String == type )
      {
         int length;
         char value[ 256 ];

         pSerializer->GetInputStream( )->Read( &length, sizeof(length) );
         Debug::Assert( Condition(length <= sizeof(value) - 1), "Registry value string is larger than max size: %d", sizeof(value) - 1 );

         pSerializer->GetInputStream( )->Read( value, length );
         value[ length ] = NULL;
         
         const char *pValue = StringRef(value);

         const char *pKey = StringRef( key );
         pRegistry->m_Hashtable.Add( pKey, RegistryValue(pValue, pKey) );

         StringRel(pValue);
      }
      else if ( RegistryValue::Bool == type )
      {
         int value;
         pSerializer->GetInputStream( )->Read( &value, sizeof(value) );
         
         const char *pKey = StringRef( key );
         pRegistry->m_Hashtable.Add( pKey, RegistryValue(value != 0, pKey) );
      }
      else if ( RegistryValue::Method == type )
      {
         // do nothing
      }
      else
      {
         Debug::Assert( Condition(false), "Unrecognized registry type: %d", type );
         return false;
      }
   }

   return pSerializable;
}

bool RegistrySerializer::Serialize(
   Serializer *pSerializer,
   const ISerializable *pSerializable
)
{
   const Registry *pRegistry = (const Registry *) pSerializable;

   struct Header
   {
      int version;
      int count;
   };

   int count = 0;

   Enumerator<const char *, RegistryValue> e = pRegistry->GetEnumerator( );
   while ( e.EnumNext( ) ) { ++count; }


   Header header;

   header.version  = 2;
   header.count    = count;

   pSerializer->GetOutputStream( )->Write( &header, sizeof(header) );

   e = pRegistry->GetEnumerator( );
   while ( e.EnumNext( ) )
   {
      int size = strlen(e.Key( ));

      pSerializer->GetOutputStream( )->Write( &size, sizeof(size) );   
      pSerializer->GetOutputStream( )->Write( e.Key( ), size );
      pSerializer->GetOutputStream( )->Write( &e.Data( ).m_Type, sizeof(int) );

      if ( RegistryValue::Int == e.Data( ).m_Type )
      {
         pSerializer->GetOutputStream( )->Write( &e.Data( ).m_Int, sizeof(int) );
      }
      else if ( RegistryValue::Float == e.Data( ).m_Type )
      {
         pSerializer->GetOutputStream( )->Write( &e.Data( ).m_Float, sizeof(float) );
      }
      else if ( RegistryValue::String == e.Data( ).m_Type )
      {
         size = strlen( e.Data( ).m_pString );

         pSerializer->GetOutputStream( )->Write( &size, sizeof(size) );
         pSerializer->GetOutputStream( )->Write( e.Data( ).m_pString, size );
      }
      else if ( RegistryValue::Bool == e.Data( ).m_Type )
      {
         int value = e.Data( ).m_Bool ? 1 : 0;

         pSerializer->GetOutputStream( )->Write( &value, sizeof(int) );
      }
      else if ( RegistryValue::Method == e.Data( ).m_Type )
      {
         // do nothing
      }
      else
      {
         Debug::Assert( Condition(false), "Unrecognized registry type: %d", e.Data( ).m_Type );
         return false;
      }
   }

   return true;
}
