#include "EnginePch.h"

#include "DataAsset.h"
#include "IOStreams.h"

DefineResourceType(Data, Asset, new DataSerializer);

void Data::Destroy( void )
{
   free( m_pData );

   Asset::Destroy( );
}

ISerializable *DataSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   struct Header
   {
      unsigned int version;
      uint32 size;
   };

   if ( NULL == pSerializable ) pSerializable = new Data;

   Header header;

   const uint32 CurrentVersion = 1;

   pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
   Debug::Assert( Condition(header.version == CurrentVersion), "Header version was %d but I was expecting %d", header.version, CurrentVersion );

   void *pBlock = malloc( header.size );
   pSerializer->GetInputStream( )->Read( pBlock, header.size );

   Data *pData = (Data *) pSerializable;

   pData->m_pData = pBlock;
   pData->m_Size  = header.size;

   return pSerializable;
}
