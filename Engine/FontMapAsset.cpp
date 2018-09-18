#include "EnginePch.h"

#include "FontMapAsset.h"
#include "IOStreams.h"

DefineResourceType(FontMap, Asset, new FontMapSerializer)

ISerializable *FontMapSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   struct Header
   {
      int count;
      int spacingWidth;
      int spacingHeight;
   };

   Header header;

   if ( NULL == pSerializable ) pSerializable = new FontMap; 

   FontMap *pFontMap = (FontMap *) pSerializable;

   pSerializer->GetInputStream( )->Read( &header, sizeof(header) );
   
   pFontMap->Create( );

   size_t memorySize = sizeof(FontMap::CharacterDesc) * header.count;

   FontMap::CharacterDesc *pDesc = (FontMap::CharacterDesc *) malloc( memorySize );

   pSerializer->GetInputStream( )->Read( pDesc, (uint32) memorySize );
   
   pFontMap->m_pCharacters   = pDesc;
   pFontMap->m_Count         = header.count;
   pFontMap->m_SpacingWidth  = header.spacingWidth;
   pFontMap->m_SpacingHeight = header.spacingHeight;

   return pSerializable;
}
