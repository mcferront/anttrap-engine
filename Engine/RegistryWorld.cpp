#include "EnginePch.h"

#include "RegistryWorld.h"
#include "RegistryAsset.h"

RegistryValue::RegistryValue(
   const char *pValue,
   const char *pPath
)
{
   m_pString = StringRef( pValue );
   m_pPath = StringRef( pPath );
   m_Type = String;
   m_Callback = NULL;
}

RegistryWorld &RegistryWorld::Instance( void )
{
   static RegistryWorld s_instance;
   return s_instance;
}

void RegistryWorld::Destroy( void )
{
   List<RegistryValue *> values;
   values.Create( );

   {
      Enumerator<const char *, RegistryValue *> e = m_Hashtable.GetEnumerator( );

      while ( e.EnumNext( ) )
      {
         values.Add( e.Data( ) );
      }
   }

   m_Hashtable.Clear( );

   int i, size = values.GetSize( );

   for ( i = 0; i < size; i++ )
   {
      delete values.GetAt( i );
   }

   values.Destroy( );
   m_Hashtable.Destroy( );
};

void RegistryWorld::ImportRegistry(
   Registry *pRegistry
)
{
   Enumerator<const char *, RegistryValue> e = pRegistry->GetEnumerator( );

   while ( e.EnumNext( ) )
   {
      RegistryValue *pValue = GetValue( e.Key( ), true );
      *pValue = e.Data( );
   }
}

void RegistryWorld::ExportRegistry(
   const char *pRootPath,
   Registry *pRegistry
)
{
   Enumerator<const char *, RegistryValue *> e = m_Hashtable.GetEnumerator( );

   HashTable<const char *, RegistryValue> hashtable;
   hashtable.Create( );

   size_t length = strlen( pRootPath );

   while ( e.EnumNext( ) )
   {
      if ( 0 == strncmp( e.Key( ), pRootPath, length ) )
      {
         hashtable.Add( e.Key( ), *e.Data( ) );
      }
   }

   pRegistry->Create( hashtable );

   hashtable.Destroy( );
}

void RegistryWorld::RemoveEntries(
   const char *pRootPath
)
{
   List<const char *> keys;
   keys.Create( );

   pRootPath = StringRef( pRootPath );

   //get all matching keys
   {
      Enumerator<const char *, RegistryValue *> e = m_Hashtable.GetEnumerator( );

      size_t length = strlen( pRootPath );

      while ( e.EnumNext( ) )
      {
         if ( StringRefEqual( e.Key( ), pRootPath ) )
            keys.Add( e.Key( ) );
      }
   }

   //remove all matching keys
   {
      List<const char *>::Enumerator k = keys.GetEnumerator( );

      while ( k.EnumNext( ) )
      {
         m_Hashtable.Remove( k.Data( ) );
      }
   }

   keys.Destroy( );
}

RegistryValue *RegistryWorld::GetValue(
   const char *pPath,
   bool create // = false
)
{
   RegistryValue *pValue = NULL;

   pPath = StringRef( pPath );

   if ( false == m_Hashtable.Get( pPath, &pValue ) )
   {
      if ( true == create )
      {
         pValue = new RegistryValue;
         m_Hashtable.Add( StringRef( pPath ), pValue );
      }
   }
   
   StringRel( pPath );

   return pValue;
}

