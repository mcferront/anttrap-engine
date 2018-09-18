#include "EnginePch.h"

#include "FrameMapAsset.h"
#include "IOStreams.h"
#include "TextureAsset.h"

DefineResourceType(FrameMap, Asset, new FrameMapSerializer);

void FrameMap::Destroy( void )
{
   int i;

   for ( i = 0; i < m_Count; i++ )
   {
      m_pTextures[ i ].Destroy( );
   }

   delete [] m_pTextures;

   Asset::Destroy( );
}

ISerializable *FrameMapSerializer::Deserialize(
   Serializer *pSerializer,
   ISerializable *pSerializable
)
{
   struct Header
   {
      int width;
      int height;

      float movementX;
      float movementY;
      float movementZ;
      
      int frameCount;
      
      float framerate;

      char id[ 42 ];
   };

   Header header;

   if ( NULL == pSerializable ) pSerializable = new FrameMap; 

   pSerializer->GetInputStream( )->Read( &header, sizeof(header), NULL );
   
   FrameMap *pFrameMap = (FrameMap *) pSerializable;
   
   pFrameMap->m_Width    = header.width;
   pFrameMap->m_Height   = header.height;
   pFrameMap->m_Count    = header.frameCount;
   pFrameMap->m_Framerate= header.framerate;
   pFrameMap->m_Movement = Vector( header.movementX, header.movementY, header.movementZ );

   pFrameMap->m_pTextures = new Texture[ header.frameCount ];

   int i;

   ISerializer *pTextureSerializer = SerializerWorld::Instance( ).GetISerializer( Texture::StaticSerializableType( ).ToString() );

   for ( i = 0; i < header.frameCount; i++ )
   {
      pTextureSerializer->Deserialize( pSerializer, &pFrameMap->m_pTextures[ i ] );
   }
   
   return pSerializable;
}
