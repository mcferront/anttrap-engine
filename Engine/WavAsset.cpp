#include "EnginePch.h"

#include "WavAsset.h"
#include "IOStreams.h"
#include "AudioWorld.h"

DefineResourceType(Wav, Asset, new WavSerializer);

ISerializable *WavSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   Wav::Header header;

   if ( NULL == pSerializable ) pSerializable = Instantiate(); 

   Wav *pWav = (Wav *) pSerializable;

   pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );

   pWav->m_AudioSize = header.audioSize;
   pWav->m_FileSize = header.fileSize;
   pWav->m_DataOffset = header.dataOffset;
   pWav->m_NumChannels = header.numChannels;
   pWav->m_Frequency = header.frequency;
   pWav->m_BitsPerSample = header.bitsPerSample;
   pWav->m_CompressedOgg = header.compressedOgg;
   pWav->m_Streaming = header.streaming;
   pWav->m_pData = NULL;

   char *pData = (char *) malloc( header.fileSize );
   memset( pData, 0, header.fileSize );
   pSerializer->GetInputStream( )->Read( pData, header.fileSize, NULL );

   AudioWorld::Instance().LoadBuffer(pWav, pData);

   if (pWav->m_Streaming)
   {
      // if this sound is meant for streaming, keep its data around
      pWav->m_pData = pData;
   }
   else
   {
      free( pData );
   }

   return pSerializable;
}

void Wav::Destroy( void )
{
   AudioWorld::Instance().UnloadBuffer( GetHandle() );

   if (m_Streaming)
   {
      // if this sound was meant for streaming, free its data
      free( m_pData );
   }

   Resource::Destroy( );
}
