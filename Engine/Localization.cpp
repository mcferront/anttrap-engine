#include "EnginePch.h"

#include "Localization.h"
#include "RegistryAsset.h"

Localization &Localization::Instance( void )
{
   static Localization s_Localization;
   return s_Localization;
}

void Localization::Create ( void )
{
}

void Localization::Destroy( void )
{
}

size_t Localization::GetString(
   char *pLocalized,
   const char *pKeyword,
   size_t localizedSize
)
{
   char key[ 256 ];
   const char *pString;

   String::Format( key, sizeof(key), "Localization/%s", pKeyword );

   RegistryValue *pValue = RegistryWorld::Instance( ).GetValue( key, false );

   if ( NULL != pValue && pValue->GetType() == RegistryValue::String )
   {
      RegistryString string( pValue );

      const char *pString = string.GetValue( );
      if ( 0 == *pString ) pString = pKeyword;
   }
   else
      pString = pKeyword;

   return InsertRegistryKeys( pLocalized, pString, localizedSize, 0 );
}

size_t Localization::InsertRegistryKeys(
   char *pLocalized,
   const char *pString,
   size_t localizedSize,
   size_t currentLength 
)
{
   char key[ 256 ];

   const char *pStart = strchr( pString, '<' );
   const char *pEnd   = strchr( pString, '>' );

   //if no registry values to insert then
   //just copy over the string and be done
   if ( NULL == pStart || NULL == pEnd || pEnd < pStart )
   {
      if ( pLocalized ) String::Copy( pLocalized, pString, localizedSize );
      return currentLength + strlen(pString);
   }

   //make sure we have enough room to copy the key path
   int length = pEnd - pStart;
   if ( length > sizeof(key) )
   {
      Debug::Assert( Condition(false), "Localized registry key path is too long: %s", pStart );
      if ( pLocalized) String::Copy( pLocalized, pString, localizedSize );
      return currentLength + strlen(pString);
   }

   String::Copy( key, pStart + 1, sizeof(key) );
   key[ length - 1 ] = NULL;


   RegistryString string( key );

   //copy up to the insertion point
   length = pStart - pString;
   if ( pLocalized ) length = Math::Min( length, localizedSize );

   if ( length > 0 )
   {
      //use unsafe copy so null isn't forceably added to the end
      if ( pLocalized) 
      {
         String::UnsafeCopy( pLocalized, pString, length );
         pLocalized  += length;
      }

      localizedSize -= length;
      currentLength += length;
   }

   //copy insert the new string
   length = strlen(string.GetValue());
   if ( pLocalized) length = Math::Min( length, localizedSize );

   if ( length > 0 )
   {
      //use unsafe copy so null isn't forceably added to the end
      if ( pLocalized) 
      {
         String::UnsafeCopy( pLocalized, string.GetValue( ), length );
         pLocalized    += length;
      }

      localizedSize -= length;
      currentLength += length;
   }
   
   pString = pEnd + 1;
   return InsertRegistryKeys( pLocalized, pString, localizedSize, currentLength );
}

void Localization::SetString( 
   const char *pKeyword, 
   const char *pValue
)
{
   char key[ 256 ];
   String::Format( key, sizeof(key), "Localization/%s", pKeyword );

   RegistryString string( key );

   string.SetValue( pValue );
}


